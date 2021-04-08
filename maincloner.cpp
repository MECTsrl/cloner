#include "maincloner.h"
#include "ui_maincloner.h"

#include "publics.h"

#include "chooseimage.h"
#include "managevpn.h"
#include "managessh.h"
#include "timeset.h"
#include "netcfg.h"
#include "info.h"

#include "alphanumpad.h"


#include <QDate>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDialog>
#include <QByteArray>



MainCloner::MainCloner(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainCloner)
{
    ui->setupUi(this);
    refresh_timer = new QTimer(this);
    connect(refresh_timer, SIGNAL(timeout()), this, SLOT(updateData()));
    runningAction = ACTION_NONE;
    nCommandCount = 0;
    refresh_timer->start(REFRESH_MS);
    if (loadInfo())  {
        ui->lblModel->setStyleSheet(COLOR_OK);
        ui->lblVersion->setStyleSheet(COLOR_OK);
    }
    else  {
        ui->lblModel->setStyleSheet(COLOR_FAIL);
        ui->lblVersion->setStyleSheet(COLOR_FAIL);
    }
    ui->lblTile->setText(QString("Cloner_%1") .arg(szClonerVersion));
    //-----------------------------------------
    // Abilitazione dei bottoni d'interfaccia
    //-----------------------------------------
    // Verifica che sulla chiavetta esista il file sysupdate di versione nella Root della Chiavetta
    QDir dirRootUSB(MOUNTED_USB);
    ui->cmdMectSuite->setEnabled(false);
    if (! sysUpdateModelFile.isEmpty() && dirRootUSB.exists(sysUpdateModelFile))  {
        ui->cmdMectSuite->setEnabled(true);
    }
    // Verifica che nell'immagine Cloner esista il Simple del Modello
    ui->cmdSimple->setEnabled(false);
    if (! mfgToolsModelDir.isEmpty())  {
        QDir dirSimple(mfgToolsModelDir);
        if (dirSimple.exists())  {
            ui->cmdSimple->setEnabled(dirSimple.exists(LOCAL_FS_TAR));
        }
    }
    // TODO: Verifica che sulla chiavetta esistano dei file OVPN
    ui->cmdVPN->setEnabled(false);
    // Abilitazione bottone SSH_KEYS
    ui->cmdSSH->setEnabled(QFile::exists(SSH_KEY_FILE));
    // Clear values
    szDestination.clear();
    szSource.clear();
    ui->lblAction->setText("");
    ui->lblAction->setMaximumWidth((screen_width * 2 / 3) -10);
    ui->progressBar->setMaximumWidth((screen_width / 3) -10);
    ui->progressBar->setVisible(false);
}

MainCloner::~MainCloner()
{
    delete ui;
}


void MainCloner::updateData()
{
    QString     szMessage;

    // Aggiornamento orologio
    QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->lblDateTime->setText(szDateTime);
    if (runningAction > ACTION_NONE)  {
//        if (runningAction == ACTION_BACKUP)  {
//            szMessage = QString("Backup Image: [%1] - Elapsed: [%2]") .arg(szDestination) .arg(startElapsed.elapsed() / 1000);
//        }
//        else if (runningAction == ACTION_RESTORE)  {
//            szMessage = QString("Restore Image: [%1] - Elapsed: [%2]") .arg(szSource) .arg(startElapsed.elapsed() / 1000);
//        }
    }
    else  {
        ui->lblAction->setText("Select an option or power off and remove the USB Key");
        ui->progressBar->setVisible(false);
    }
}

bool MainCloner::loadInfo()
{
    bool    fRes = false;

    // Cloner Version from Environment
    szClonerVersion = QString("%1.%2.%3") .arg(MECT_BUILD_MAJOR) .arg(MECT_BUILD_MINOR) .arg(MECT_BUILD_BUILD);

    QFile file(ROOTFS_VERSION);
    if(file.exists()) {
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);

        // TODO: Come ottenere la versione corrente del Cloner

        // Cerca nel file le righe relative al Target e alla Release corrente
        while(!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (! line.isEmpty())  {
                infoList.append(line);
                // Modello
                if (line.startsWith("Target:"))  {
                    line.replace("Target:", "");
                    szModel = line.trimmed();
                    ui->lblModel->setText(szModel);
                    fRes = true;
                }
                // Versione corrente    Release:
                if (line.startsWith("Release:"))  {
                    line.replace("Release:", "");
                    szTargetVersion = line.trimmed();
                    ui->lblVersion->setText(QString(" v%1") .arg(szTargetVersion));
                }

            }
        }
        file.close();
        // Model Dependent Info
        if (! szModel.isEmpty())  {
            sysUpdateModelFile = QString(MODEL_SYSUPDATE_FILE) .arg(szClonerVersion) .arg(szModel);
            mfgToolsModelDir = QString(MODEL_IMAGE_DIR) .arg(szModel) .arg(szClonerVersion);
            // fprintf(stderr, "SysUpdate File:[%s]-Simple Local Image Directory:[%s]\n",
            //        sysUpdateModelFile.toLatin1().data(),
            //        mfgToolsModelDir.toLatin1().data());
        }
    }
    // Load the exclude list for the root file system.
    excludesRFSList.prepend("");
    QDir distDir(MOUNTED_FS);
    if (distDir.exists(EXCLUDES_RFS)) {
        QFile excludesRFS(distDir.filePath(EXCLUDES_RFS));
        if (excludesRFS.open(QIODevice::ReadOnly))  {
            while (! excludesRFS.atEnd())    {
                excludesRFSList.append(excludesRFS.readLine().simplified());
            }
        }
    }
    // Load the exclude list for the local file system.
    excludesLFSList.prepend("");
    if (distDir.exists(EXCLUDES_LFS)) {
        QFile excludesLFS(distDir.filePath(EXCLUDES_LFS));
        if (excludesLFS.open(QIODevice::ReadOnly))  {
            while (! excludesLFS.atEnd())  {
                excludesLFSList.append(excludesLFS.readLine().simplified());
            }
        }
    }

    return fRes;
}

QString MainCloner::getDefaultDirName()
{
    QString szReturn = QDate::currentDate().toString("yyyy_MM_dd");
    if (! szModel.isEmpty())  {
        szReturn.append(QString("_%1") .arg(szModel));
    }
    if (! szTargetVersion.isEmpty())  {
        szReturn.append(QString("_%1") .arg(szTargetVersion));
    }
    return szReturn;
}

void MainCloner::on_cmdBackup_clicked()
{
    alphanumpad * dk;
    char        value[256];
    QString     szLocalTar;

    szDestination.clear();
    commandList.clear();
    dk = new alphanumpad(value, false, getDefaultDirName().toAscii().data(), false, this);
    QString szStyle = QString("font-size: 10pt;\n");
    szStyle.append(QString(
    "background-color: Azure;\n"
    "color: Navy;\n"
    ));
    dk->setStyleSheet(szStyle);
    dk->showFullScreen();
    if (dk->exec() == QDialog::Accepted && strlen(value) != 0)
    {
        QString szDirImage = QString(value);
        szDirImage.replace(" ", "_");
        szDirImage.append("/");
        szDestination = szDirImage;
        szDirImage.prepend(CLONED_IMAGES_DIR);
        QDir backupDir(szDirImage);
        szLocalTar = szDirImage;
        szLocalTar.append(LOCAL_FS_TAR);
        // Check Backup Dir Existence
        if (backupDir.exists(LOCAL_FS_TAR))  {
            if (QMessageBox::question(this, "Confirm Backup Overwrite",
                    QString("The Backup destination\n\n[%1]\n\nalready exists!\n\nOverwrite ?") .arg(szDirImage),
                    QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Ok)  {
                // do nothing
                return;
            }
            else {
                QFile::remove(szLocalTar);
            }
        }
        // Create Path
        QString szCommand = QString("mkdir -p %1") .arg(szDirImage);
        system(szCommand.toLatin1().data());
        // Comandi di Backup
        runningCommand = QString("/etc/rc.d/init.d/sdcheck stop");
        commandList.append(QString("tar cf %1%2 -C /local .") .arg(szDirImage) .arg(LOCAL_FS_TAR));
        commandList.append("sync");
        // Avvio del Processo di Backup
        // Creazione processo
        myProcess = new QProcess(this);
        // Impostazione dei parametri del Processo
        myProcess->setWorkingDirectory(MOUNTED_FS);
        // Slot di Gestione del processo
        connect(myProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(actionCompleted(int,QProcess::ExitStatus)));
        connect(myProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(actionFailed(QProcess::ProcessError)));
        // Avvio del Task
        runningAction = ACTION_BACKUP;
        ui->lblAction->setText(runningCommand);
        myProcess->start(runningCommand);
        // Restart del Timer
        startElapsed.start();
    }
    dk->deleteLater();
}

void MainCloner::on_cmdRestore_clicked()
{
    ChooseImage     *selectImage;
    QString         image2Restore;
    int             nRetentiveMode = -1;

    selectImage = new ChooseImage(this);
    selectImage->showFullScreen();
    if (selectImage->exec() == QDialog::Accepted)   {
        // Revert resore Options
        image2Restore = selectImage->getSelectedImage(nRetentiveMode);
        if (! image2Restore.isEmpty())  {
            // Restore Dir
            szSource = image2Restore;
            /* before 2.0 */
            if (! QFile::exists("/etc/mac.conf")) {
                /* extract MAC0 from /local/etc/sysconfig/net.conf and put it into /etc/mac.conf */
                system(
                    "mount -o rw,remount /"
                    " && "
                    "grep MAC0 /local/etc/sysconfig/net.conf "
                    " && "
                    "grep MAC0 /local/etc/sysconfig/net.conf > /etc/mac.conf"
                    " && "
                    "mount -o ro,remount /"
                );

                /* delete MAC0 from /local/etc/sysconfig/net.conf */
                system(
                    "grep -v MAC0 /local/etc/sysconfig/net.conf > /tmp/net.conf"
                    " && "
                    "mv /tmp/net.conf /local/etc/sysconfig/net.conf"
                );
            }
            // Start restore procedure
            QString sourceTar = QString("%1%2/%3") .arg(CLONED_IMAGES_DIR) .arg(image2Restore) .arg(LOCAL_FS_TAR);
            restoreLocalFile(sourceTar, excludesLFSList, nRetentiveMode);

        }
    }
    selectImage->deleteLater();
}

void MainCloner::on_cmdVPN_clicked()
{
    ManageVPN     *openVPNManager;

    openVPNManager = new ManageVPN(this);
    openVPNManager->showFullScreen();
    openVPNManager->exec();
    openVPNManager->deleteLater();
}

void MainCloner::on_cmdSSH_clicked()
{
    ManageSSH   *sshKeyManager;
    sshKeyManager = new ManageSSH(this);
    sshKeyManager->showFullScreen();
    sshKeyManager->exec();
    sshKeyManager->deleteLater();
}

void MainCloner::on_cmdMectSuite_clicked()
{
    if (QMessageBox::question(this, "Confirm Mect Suite Update",
                              QString("Confirm Mect Suite Update for Model:\n\n[%1]\n\nFrom Version: [%2]\n\nTo Version: [%3]\n\nUpdate File:[%4]")
                              .arg(szModel) .arg(szTargetVersion) .arg(szClonerVersion) .arg(sysUpdateModelFile),
                    QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok)  {
        // Uscita da programma con Exit 42
        exit(START_SYSUPDATE);
    }
}

void MainCloner::on_cmdClock_clicked()
{
    TimeSet     *clockManager;
    clockManager = new TimeSet(this);
    clockManager->showFullScreen();
    clockManager->exec();
    clockManager->deleteLater();
}


void MainCloner::on_cmdNetwork_clicked()
{
    NetCfg      *netManager;
    netManager = new NetCfg(this);
    netManager->showFullScreen();
    netManager->exec();
    netManager->deleteLater();
}

void MainCloner::on_cmdSimple_clicked()
{
    if (QMessageBox::question(this, "Confirm Simple Restore",
                              QString("Confirm Mect Suite Simple Restore for Model:\n[%1]\nVersion: [%2]\nFile:[%3]")
                              .arg(szModel) .arg(szClonerVersion) .arg(mfgToolsModelDir),
                    QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok)  {
        // Start restore procedure
        szSource = QString("%1_%2") .arg(szModel) .arg(szClonerVersion);
        int         nRestore = ACTION_RESTORE;
        QString sourceTar = mfgToolsModelDir;
        sourceTar.append(LOCAL_FS_TAR);
        restoreLocalFile(sourceTar, excludesLFSList, nRestore);
    }
}

void MainCloner::on_cmdMenu_clicked()
{
    Info      *infoPage;
    infoPage = new Info(this);
    infoPage->showFullScreen();
    infoPage->exec();
    infoPage->deleteLater();
}

void MainCloner::restoreLocalFile(QString &szLocalTar, QStringList &files2Exclude, int nRetentiveMode)
{
    QStringList localExclude = files2Exclude;

    // Add retentive file to exclude list
    if (nRetentiveMode == RETENTIVE_IGNORE)   {
        localExclude.append(RETENTIVE_FILE);
    }
    // Extract list of Exclude files
    QString excludesToLocal = localExclude.join(" --exclude ");
    runningCommand = QString("/etc/rc.d/init.d/sdcheck stop");
    // Create Ram Disk Mount Point
    commandList.append(QString("mkdir -p %1") .arg(TMP_DIR));
    // Mount Ram Disk
    commandList.append(QString("/bin/mount -t tmpfs -o size=%1M tmpfs %2") .arg(RAMDISK_SIZE) .arg(TMP_DIR));
    // Expand local tar to ram disk
    commandList.append(QString("tar xf %1 -C %2") .arg(szLocalTar) .arg(TMP_DIR));
    // Rsync to target local partition
    commandList.append(QString("rsync -Havxc --delete %1/ /local/ %2") .arg(TMP_DIR) .arg(excludesToLocal));
    // Final Sync
    commandList.append(QString("sync"));
    // Umount Ram Disk
    commandList.append(QString("/bin/umount %1") .arg(TMP_DIR));
    // Clear variabili ritentive
    if (nRetentiveMode == RETENTIVE_RESET)  {
        commandList.append(QString("dd if=/dev/zero of=%1 bs=768 count=1") .arg(RETENTIVE_FILE));
    }
    // Avvio del Processo di Backup
    // Creazione processo
    myProcess = new QProcess(this);
    // Impostazione dei parametri del Processo
    myProcess->setWorkingDirectory("/");
    // Slot di Gestione del processo
    connect(myProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(actionCompleted(int,QProcess::ExitStatus)));
    connect(myProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(actionFailed(QProcess::ProcessError)));
    // Avvio del Task
    runningAction = ACTION_RESTORE;
    ui->lblAction->setText(runningCommand);
    nCommandCount = commandList.count();
    ui->progressBar->setMaximum(nCommandCount);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);
    myProcess->start(runningCommand);
    // Restart del Timer
    startElapsed.start();
}

void MainCloner::actionCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString     szTitle;
    QString     szMessage;

    if (commandList.isEmpty())  {
        ui->progressBar->setValue(ui->progressBar->maximum());
        if (runningAction == ACTION_BACKUP)  {
            szTitle = "Back-Up";
            szMessage = QString("Back-Up to [%1] Successfully completed!") .arg(szDestination);
        }
        else if (runningAction == ACTION_RESTORE)  {
            szTitle = "Restore";
            szMessage = QString("Restore from [%1] Successfully completed!") .arg(szSource);
        }
        QMessageBox::information(this, szTitle, szMessage, QMessageBox::Ok);
        ui->lblAction->setText("");
        ui->progressBar->setVisible(false);
        QObject::disconnect(myProcess, 0, 0, 0);
        myProcess->deleteLater();
        runningAction = ACTION_NONE;
    }
    else  {
        QString szNextCommand = commandList.takeFirst();
        // fprintf(stderr, "Running Command Done [%s] - Starting Next Command [%s] exitCode: %d exitStatus: %d\n", runningCommand.toLatin1().data(),
        //                                szNextCommand.toLatin1().data(), exitCode, exitStatus);
        myProcess->start(szNextCommand);
        runningCommand = szNextCommand;
        ui->lblAction->setText(runningCommand);
        ui->progressBar->setValue(ui->progressBar->value() + 1);
    }
}

void MainCloner::actionFailed(QProcess::ProcessError errorCode)
{
    QString     szTitle;
    QString     szMessage;

    if (runningAction == ACTION_BACKUP)  {
        szTitle = "Back-Up";
        szMessage = QString("Back-Up to [%1] Failed!") .arg(szDestination);
    }
    else if (runningAction == ACTION_RESTORE)  {
        szTitle = "Restore";
        szMessage = QString("Restore from [%1] Failed!") .arg(szSource);
    }
    fprintf(stderr, "%s errorCode: %d\n", szMessage.toLatin1().data(), errorCode);
    QMessageBox::critical(this, szTitle, szMessage, QMessageBox::Ok);
    QObject::disconnect(myProcess, 0, 0, 0);
    myProcess->deleteLater();
    runningAction = ACTION_NONE;
}

