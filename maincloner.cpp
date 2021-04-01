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
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDialog>



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
    // Abilitazione dei bottoni d'interfaccia
    // Verifica che sulla chiavetta esista il file sysupdate di versione nella Root della Chiavetta
    QDir dirRootUSB(MOUNTED_USB);
    ui->cmdMectSuite->setEnabled(false);
    if (! sysUpdateModelFile.isEmpty() && dirRootUSB.exists(sysUpdateModelFile))  {
        ui->cmdMectSuite->setEnabled(true);
    }
    // Verifica che nell'immagine Cloner esista il Simple del Modello
    ui->cmdSimple->setEnabled(false);
    QDir dirSimple(SIMPLE_DIR);
    if (! mfgToolsModelFile.isEmpty() && dirSimple.exists(mfgToolsModelFile))  {
        ui->cmdSimple->setEnabled(true);
    }
    // TODO: Verifica che sulla chiavetta esistano dei file OVPN
    ui->cmdVPN->setEnabled(true);
    // TODO: Verifica che sulla chiavetta esistano dei file SSH
    ui->cmdSSH->setEnabled(true);
    szDestination.clear();
    szSource.clear();
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
        if (runningAction == ACTION_BACKUP)  {
            szMessage = QString("Backup Image: [%1] - Elapsed: [%2]") .arg(szDestination) .arg(startElapsed.elapsed() / 1000);
        }
        else if (runningAction == ACTION_RESTORE)  {
            szMessage = QString("Restore Image: [%1] - Elapsed: [%2]") .arg(szSource) .arg(startElapsed.elapsed() / 1000);
        }
    }
    else  {
        ui->lblAction->setText("");
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
                    ui->lblVersion->setText(szTargetVersion);
                }

            }
        }
        file.close();
        // Model Dependent Info
        if (! szModel.isEmpty())  {
            sysUpdateModelFile = QString(MODEL_SYSUPDATE_FILE) .arg(szClonerVersion) .arg(szModel);
            mfgToolsModelFile = QString(MODEL_IMAGE_FILE) .arg(szModel) .arg(MECT_BUILD_MAJOR) .arg(MECT_BUILD_MINOR) .arg(MECT_BUILD_BUILD);
            fprintf(stderr, "SysUpdate File:[%s] Simple Image File:[%s]\n", sysUpdateModelFile.toLatin1().data(), mfgToolsModelFile.toLatin1().data());
        }
    }
    // Load the exclude list for the root file system.
    excludesRFSList.prepend("");
    QDir distDir(MOUNTED_FS);
    if (distDir.exists(EXCLUDES_RFS)) {
        QFile excludesRFS(distDir.filePath(EXCLUDES_RFS));
        if (excludesRFS.open(QIODevice::ReadOnly))  {
            while (!excludesRFS.atEnd())    {
                excludesRFSList.append(excludesRFS.readLine().simplified());
            }
            fprintf(stderr, "%s Read:%d Items\n", EXCLUDES_RFS, excludesRFSList.count());
        }
    }
    // Load the exclude list for the local file system.
    excludesLFSList.prepend("");
    if (distDir.exists(EXCLUDES_LFS)) {
        QFile excludesLFS(distDir.filePath(EXCLUDES_LFS));
        if (excludesLFS.open(QIODevice::ReadOnly))  {
            while (!excludesLFS.atEnd())  {
                excludesLFSList.append(excludesLFS.readLine().simplified());
            }
            fprintf(stderr, "%s Read:%d Items\n", EXCLUDES_LFS, excludesLFSList.count());
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
    dk->showFullScreen();
    if (dk->exec() == QDialog::Accepted && strlen(value) != 0)
    {
        QString szDirImage = QString(value);
        szDirImage.replace(" ", "_");
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
        fprintf(stderr, "First Backup Command: [%s]\n", runningCommand.toLatin1().data());
        // Creazione processo
        myProcess = new QProcess(this);
        // Impostazione dei parametri del Processo
        myProcess->setWorkingDirectory(MOUNTED_FS);
        // Slot di Gestione del processo
        connect(myProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(actionCompleted(int,QProcess::ExitStatus)));
        connect(myProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(actionFailed(QProcess::ProcessError)));
        // Avvio del Task
        runningAction = ACTION_BACKUP;
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

    selectImage = new ChooseImage(this);
    selectImage->showFullScreen();
    if (selectImage->exec() == QDialog::Accepted)   {
        image2Restore = selectImage->getSelectedImage();
        if (! image2Restore.isEmpty())  {

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
                              QString("Confirm Mect Suite Simple Restore for Model:\n\n[%1]\n\nVersion: [%2]\n\nFile:[%3]")
                              .arg(szModel) .arg(szClonerVersion) .arg(mfgToolsModelFile),
                    QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok)  {
        // TODO: Simple Restore
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

void MainCloner::actionCompleted(int exitCode, QProcess::ExitStatus exitStatus)
{
    QString     szTitle;
    QString     szMessage;

    if (commandList.isEmpty())  {
        if (runningAction == ACTION_BACKUP)  {
            szTitle = "Back-Up";
            szMessage = QString("Back-Up to [%1] Successfully completed!") .arg(szDestination);
        }
        else if (runningAction == ACTION_RESTORE)  {
            szTitle = "Restore";
            szMessage = QString("Restore from [%1] Successfully completed!") .arg(szSource);
        }
        fprintf(stderr, "Last Command Done [%s]. %s exitCode: %d exitStatus: %d\n", runningCommand.toLatin1().data(), szMessage.toLatin1().data(), exitCode, exitStatus);
        QMessageBox::information(this, szTitle, szMessage, QMessageBox::Ok);
        QObject::disconnect(myProcess, 0, 0, 0);
        myProcess->deleteLater();
    }
    else  {
        QString szNextCommand = commandList.takeFirst();
        fprintf(stderr, "Running Command Done [%s] - Starting Next Command [%s] exitCode: %d exitStatus: %d\n", runningCommand.toLatin1().data(),
                                        szNextCommand.toLatin1().data(), exitCode, exitStatus);
        myProcess->start(szNextCommand);
        runningCommand = szNextCommand;
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
}
