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
    // Verifica che esista il certificato OVPN originale o da copiare
    ui->cmdVPN->setEnabled(false);
    if ((not szVPNOriginalFile.isEmpty() && QFile::exists(szVPNOriginalFile)) || (not szVPNNewFile.isEmpty() && QFile::exists(szVPNNewFile)))  {
        // Il bottone è abilitato se esiste un file nella cartella dei certificati con nome <SERIALNO>.ovpn
        ui->cmdVPN->setEnabled(true);
    }
    // Abilitazione bottone SSH_KEYS
    ui->cmdSSH->setEnabled(QFile::exists(SSH_KEY_FILE));
    // TODO: Abilitazione del bottone Time Set
    // ui->cmdClock->setVisible(false);
    ui->cmdClock->setEnabled(true);
    // TODO: Abilitazione del bottone Info (Menu)
    ui->cmdMenu->setEnabled(true);
    // Clear values
    szDestination.clear();
    szSource.clear();
    ui->lblAction->setText("");
    ui->lblAction->setMaximumWidth((screen_width * 2 / 3) -10);
    ui->progressBar->setMaximumWidth((screen_width / 3) -10);
    ui->progressBar->setVisible(false);

    szAlphaStyle = QString("font-size: 10pt;\n");
    szAlphaStyle.append(QString( "background-color: Azure;\ncolor: Navy;\n"));
    // ntpClient->start();
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
    if (runningAction == ACTION_NONE)  {
        ui->lblAction->setText("Select an option or power off and remove the usb key");
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
                else if (line.startsWith("Release:"))  {
                    line.replace("Release:", "");
                    szTargetVersion = line.trimmed();
                    ui->lblVersion->setText(QString(" v%1") .arg(szTargetVersion));
                }
                // Versione QT
                else if (line.startsWith("Qt:"))  {
                    line.replace("Qt:", "");
                    szQtVersion = line.trimmed();
                }
            }
        }
        file.close();
        // Model Dependent Info
        if (! szModel.isEmpty())  {
            sysUpdateModelFile = QString(MODEL_SYSUPDATE_FILE) .arg(szClonerVersion) .arg(szModel);
            mfgToolsModelDir = QString(MODEL_IMAGE_DIR) .arg(szModel) .arg(szClonerVersion);
        }
    }
    // Board Serial #
    szSerialNO.clear();
    QFile fileSerialNO(SERIAL_FILE);
    // Caricamento del Serial Number della scheda
    if (fileSerialNO.exists() && fileSerialNO.open(QIODevice::ReadOnly | QIODevice::Text))  {
        char buf[STR_LEN];
        if (fileSerialNO.readLine(buf, STR_LEN) > 0) {
            szSerialNO = QString(buf).trimmed();
        }
        fileSerialNO.close();
    }
    // Open VPN Certificates
    szVPNOriginalFile.clear();
    szVPNNewFile.clear();
    if (! szSerialNO.isEmpty())  {
        szVPNOriginalFile = QString(OVPN_CERT_LOCFILE) .arg(szSerialNO);
        szVPNNewFile   = QString(OVPN_CERT_NEWFILE) .arg(szSerialNO);
    }
    // qDebug("Board S/N: %s", szSerialNO.toLatin1().data());
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
    char        value[STR_LEN];
    QString     szLocalTar;

    szDestination.clear();
    commandList.clear();
    dk = new alphanumpad(value, true, getDefaultDirName().toAscii().data());
    dk->setStyleSheet(szAlphaStyle);
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
        // system(szCommand.toLatin1().data());
        commandList.append(szCommand);
        // Comandi di Backup
        // NON Smonta SD Card
        // runningCommand = QString("/etc/rc.d/init.d/sdcheck stop");
        szCommand = QString("tar cf %1%2 -C /local .") .arg(szDirImage) .arg(LOCAL_FS_TAR);
        commandList.append(szCommand);
        commandList.append("sync");
        // Avvio del Processo di Backup
        QDir::setCurrent(MOUNTED_FS);
        runningAction = ACTION_BACKUP;
        startElapsed.start();
        execCommandList();
    }
    dk->deleteLater();
}

void MainCloner::on_cmdRestore_clicked()
{
    ChooseImage     *selectImage;
    QString         image2Restore;
    int             nRetentiveMode = -1;
    int             nHmiIniMode = -1;
    int             nLogMode = -1;

    selectImage = new ChooseImage(this);
    selectImage->showFullScreen();
    if (selectImage->exec() == QDialog::Accepted)   {
        // Revert resore Options
        image2Restore = selectImage->getSelectedImage(nRetentiveMode, nHmiIniMode, nLogMode);
        if (! image2Restore.isEmpty())  {
            // Restore Dir
            szSource = image2Restore;
//            /* before 2.0 */
//            if (! QFile::exists("/etc/mac.conf")) {
//                /* extract MAC0 from /local/etc/sysconfig/net.conf and put it into /etc/mac.conf */
//                system(
//                    "mount -o rw,remount /"
//                    " && "
//                    "grep MAC0 /local/etc/sysconfig/net.conf "
//                    " && "
//                    "grep MAC0 /local/etc/sysconfig/net.conf > /etc/mac.conf"
//                    " && "
//                    "mount -o ro,remount /"
//                );

//                /* delete MAC0 from /local/etc/sysconfig/net.conf */
//                system(
//                    "grep -v MAC0 /local/etc/sysconfig/net.conf > /tmp/net.conf"
//                    " && "
//                    "mv /tmp/net.conf /local/etc/sysconfig/net.conf"
//                );
//            }
            // Start restore procedure
            QString sourceTar = QString("%1%2/%3") .arg(CLONED_IMAGES_DIR) .arg(image2Restore) .arg(LOCAL_FS_TAR);
            restoreLocalFile(sourceTar, excludesLFSList, nRetentiveMode, nHmiIniMode, nLogMode);

        }
    }
    selectImage->deleteLater();
}

void MainCloner::on_cmdVPN_clicked()
{
    ManageVPN       *openVPNManager;
    QString         szCommand;

    openVPNManager = new ManageVPN(this);
    openVPNManager->showFullScreen();
    if (openVPNManager->exec() == QDialog::Accepted)   {
        // Get Action to be performed
        int nAction = openVPNManager->getSelectedAction();
        if (nAction > OVPN_NO_ACTION)  {
            commandList.clear();
            // Stop VPN Service
            szCommand = QString(OVPN_COMMAND) .arg("stop");
            commandList.append(szCommand);
            szCommand = QString("sleep 2");
            commandList.append(szCommand);
            // Mount rootfs as R/W
            szCommand = QString(MOUNT_ROOTFS_RW);
            commandList.append(szCommand);
            // Check Open VPN Config Dir
            QDir vpnConfigDir(OVPN_CONF_DIR);
            if (! vpnConfigDir.exists())  {
                szCommand = QString("mkdir -p %1") .arg(OVPN_CONF_DIR);
                commandList.append(szCommand);
            }
            // Remove Existing Cert
            if (nAction & OVPN_CERT_REMOVE && QFile::exists(szVPNOriginalFile))  {
                // Remove
                szCommand = QString("rm -f %1") .arg(szVPNOriginalFile);
                commandList.append(szCommand);
            }
            // Add new Cert
            if (nAction & OVPN_CERT_ADD && QFile::exists(szVPNNewFile))  {
                // Copy file
                szCommand = QString("cp %1 %2") .arg(szVPNNewFile) .arg(szVPNOriginalFile);
                commandList.append(szCommand);
                // Change Mode to 644
                szCommand = QString("chmod 644 %1") .arg(szVPNOriginalFile);
                commandList.append(szCommand);
            }
            // Sync
            szCommand = QString("sync");
            commandList.append(szCommand);
            // Mount Root File System as RO
            szCommand = QString(MOUNT_ROOTFS_RO);
            commandList.append(szCommand);
            // Restart VPN Service
            szCommand = QString(OVPN_COMMAND) .arg("start");
            commandList.append(szCommand);
            // Avvio del Task
            runningAction = ACTION_OVPN;
            // Restart del Timer
            startElapsed.start();
            execCommandList();
        }
    }
    else  {
        qDebug("VPN: No action required");
    }
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
    if (QMessageBox::question(this, "Mect Suite Update",
                              QString("Confirm Mect Suite Update for\n\nModel: [%1]\nFrom Version: [%2]\nTo Version: [%3]")
                              .arg(szModel) .arg(szTargetVersion) .arg(szClonerVersion),
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
    if (QMessageBox::question(this, "Factory Restore",
                              QString("Confirm Mect Suite Factory Restore\n\nModel: [%1]\nVersion: [%2]")
                              .arg(szModel) .arg(szClonerVersion),
                    QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok)  {
        // Start restore procedure
        szSource = QString("%1_%2") .arg(szModel) .arg(szClonerVersion);
        int         nRetentive = RESTORE_RESTORE;
        int         nHmiIni = RESTORE_RESTORE;
        int         nLogs = RESTORE_RESET;
        QString sourceTar = mfgToolsModelDir;
        sourceTar.append(LOCAL_FS_TAR);
        restoreLocalFile(sourceTar, excludesLFSList, nRetentive, nHmiIni, nLogs);
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

void MainCloner::restoreLocalFile(QString &szLocalTar, QStringList &files2Exclude, int nRetentiveMode, int nHmiIniMode, int nLogMode)
{
    QStringList localExclude = files2Exclude;

    commandList.clear();
    qDebug("Restore File: [%s] Retentive:%d Hmi.ini:%d Log:%d", szLocalTar.toLatin1().data(), nRetentiveMode, nHmiIniMode, nLogMode);
    // Add retentive file to exclude list
    if (nRetentiveMode == RESTORE_IGNORE)   {
        localExclude.append(RETENTIVE_FILE);
    }
    // Add hmi.ini to exclude list
    if (nHmiIniMode == RESTORE_IGNORE)   {
        localExclude.append(INI_FILE);
    }
    // Add store dir to exclude list (both IGNORE and RESET) do not restore backup logs
    if (nLogMode == RESTORE_IGNORE || RESTORE_RESET)   {
        localExclude.append(TAR_STORE_DIR);
    }
    // Extract list of Exclude files
    QString excludesToLocal = localExclude.join(" --exclude ");
    // NON smonta SD Card
    // runningCommand = QString("/etc/rc.d/init.d/sdcheck stop");
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
    if (nRetentiveMode == RESTORE_RESET)  {
        commandList.append(QString("dd if=/dev/zero of=/local/%1 bs=768 count=1") .arg(RETENTIVE_FILE));
    }
    // Clear hmi.ini
    if (nHmiIniMode == RESTORE_RESET)  {
        commandList.append(QString("echo -e \"[General]\nrotation=0\nplc_host=127.0.0.1\n\" > /local/flash/root/hmi.ini"));
    }
    // Clear Logs
    if (nLogMode == RESTORE_RESET)  {
        commandList.append(QString("rm -rf /local/flash/data/store/*"));
    }
    // Final Sync
    commandList.append(QString("sync"));
    // Avvio del Processo di Backup
    QDir::setCurrent("/");
    // Avvio del Task
    runningAction = ACTION_RESTORE;
    // Restart del Timer
    startElapsed.start();
    execCommandList();
}

void MainCloner::execCommandList()
{
    bool        allOK = true;
    QString     szTitle;
    QString     szMessage;

    // Show Progress
    ui->progressBar->setMaximum(commandList.count());
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);
    // Commands execution Loop
    while (commandList.count() > 0 && allOK)  {
        szRunningCommand = commandList.takeFirst();
        qDebug("Starting Next Command [%s]", szRunningCommand.toLatin1().data());
        ui->lblAction->setText(szRunningCommand);
        int nRes = system(szRunningCommand.toLatin1().data());
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        allOK = (nRes == 0);
    }
    // Loop ended
    if (allOK)  {
        ui->progressBar->setValue(ui->progressBar->maximum());
        if (runningAction == ACTION_BACKUP)  {
            szTitle = "Back-Up";
            szMessage = QString("Back-Up to [%1] Successfully completed!") .arg(szDestination);
        }
        else if (runningAction == ACTION_RESTORE)  {
            szTitle = "Restore";
            szMessage = QString("Restore from [%1] Successfully completed!") .arg(szSource);
        }
        else if (runningAction == ACTION_OVPN)  {
            szTitle = "Open VPN";
            szMessage = QString("Update OVPN Config Successfully completed!");
        }
        qDebug("Execution Ended: elapsed [%d]s", (int) (startElapsed.elapsed() / 1000));
        QMessageBox::information(this, szTitle, szMessage, QMessageBox::Ok);
    }
    else  {
        if (runningAction == ACTION_BACKUP)  {
            szTitle = "Back-Up";
            szMessage = QString("Back-Up to [%1] Failed!") .arg(szDestination);
        }
        else if (runningAction == ACTION_RESTORE)  {
            szTitle = "Restore";
            szMessage = QString("Restore from [%1] Failed!") .arg(szSource);
        }
        else if (runningAction == ACTION_OVPN)  {
            szTitle = "Open VPN";
            szMessage = QString("Update OVPN Failed!");
        }
        qCritical("Failed Command [%s]", szRunningCommand.toLatin1().data());
        QMessageBox::critical(this, szTitle, szMessage, QMessageBox::Ok);
    }
    ui->lblAction->setText("");
    ui->progressBar->setVisible(false);
    runningAction = ACTION_NONE;
}
