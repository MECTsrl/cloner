#include "cloner.h"
#include "ui_cloner.h"
#include <QMessageBox>
#include <QTimer>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <errno.h>
#include <stdio.h>

#define REFRESH_MS 1000
#define POINTS_NB 30
#define USB_MOUNT_POINT "/tmp/mnt"
#define APP_DIR_PATH QApplication::applicationDirPath().toAscii().data()

char arrayStepName[step_nb][16] = {
    "None",
    "LocalFS",
    "RootFS",
    "Kernel",
    "LocalFS",
    "RootFS",
    "Kernel"
};

int actualStep = step_none_e;
int exitArray[step_nb];
char stringError[COMMAND_LEN];

bool myProcess::setCommand(char * command)
{
    if (command != NULL && strlen(command) > 0)
    {
        fprintf(stderr, "SET @%s@\n", command);
        strcpy(_command, command);
        return true;
    }
    else
    {
        return false;
    }
}

myProcess::myProcess(void)
{
    _command[0] = '\0';
}

myProcess::~myProcess(void)
{
}

void myProcess::run(void)
{
    stringError[0] = '\0';
    if (strlen(_command) > 0)
    {
        fprintf(stderr, "RUNNING @%s@\n", _command);
        exitArray[actualStep] = system(_command);
        if (exitArray[actualStep] != 0)
        {
            strcpy(stringError, strerror(errno));
        }
    }
    else
    {
        strcpy(stringError, "No command to run");
        fprintf(stderr, "ERROR @%s@\n", _command);
        exitArray[actualStep] = 1;
    }
    fprintf(stderr, "EXIT @%d@\n", exitArray[actualStep]);
    exit (exitArray[actualStep]);
}

cloner::cloner(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::cloner)
{
    ui->setupUi(this);
    /* check which restore file is avalable on the USB pen drive */
    /* check if a licenced sd card is mounted */
    loadInfo();
    refresh_timer = new QTimer(this);
    connect(refresh_timer, SIGNAL(timeout()), this, SLOT(updateData()));
    refresh_timer->start(REFRESH_MS);
    
	connect(&mp, SIGNAL(finished()), this, SLOT(finishProcess()));

    ui->label->setText(QString("Cloner rev.%1").arg(SVN_REV));

    QDir().mkdir(QString("%1/%2").arg(USB_MOUNT_POINT).arg("backup"));


    char command[COMMAND_LEN];
    sprintf(command, "%s/restore/imx28_ivt_linux.sb", USB_MOUNT_POINT);
    if (QFile::exists(command))
    {
        ui->checkBoxRestoreKernel->setCheckable(true);
        ui->checkBoxRestoreKernel->setChecked(true);
    }
    sprintf(command, "%s/restore/rootfs.tar.bz2", USB_MOUNT_POINT);
    if (QFile::exists(command))
    {
        ui->checkBoxRestoreRootFS->setCheckable(true);
        ui->checkBoxRestoreRootFS->setChecked(true);
    }
    sprintf(command, "%s/restore/local.tar.bz2", USB_MOUNT_POINT);
    if (QFile::exists(command))
    {
        ui->checkBoxRestoreLocalFS->setCheckable(true);
        ui->checkBoxRestoreLocalFS->setChecked(true);
    }

    ui->labelCloneKernelStatus->setVisible(false);
    ui->checkBoxBackupKernel->setVisible(true);

    ui->labelCloneLocalfsStatus->setVisible(false);
    ui->checkBoxBackupLocalFS->setVisible(true);

    ui->labelCloneRootfsStatus ->setVisible(false);
    ui->checkBoxBackupRootFS->setVisible(true);

    ui->labelRestoreKernelStatus ->setVisible(false);
    ui->checkBoxRestoreKernel->setVisible(true);

    ui->labelRestoreLocalfsStatus ->setVisible(false);
    ui->checkBoxRestoreLocalFS->setVisible(true);

    ui->labelRestoreRootfsStatus ->setVisible(false);
    ui->checkBoxRestoreRootFS->setVisible(true);
}

void cloner::updateData()
{
    if (actualStep == step_none_e && arrayQueue[step_bkup_localfs_e] == 1)
    {
        /* start step 3 */
        actualStep = step_bkup_localfs_e;
        if (backupLocalFs() == false)
        {
            finishProcess();
            return;
        }
    }
    else if (actualStep == step_none_e && arrayQueue[step_bkup_rootfs_e] == 1)
    {
        /* start step 2 */
        actualStep = step_bkup_rootfs_e;
        if (backupRootFs() == false)
        {
            finishProcess();
            return;
        }
    }
    else if (actualStep == step_none_e && arrayQueue[step_bkup_kernel_e] == 1)
    {
        /* start step 1 */
        actualStep = step_bkup_kernel_e;
        if (backupKernel() == false)
        {
            finishProcess();
            return;
        }
    }
    else if (actualStep == step_none_e && arrayQueue[step_restore_localfs_e] == 1)
    {
        /* start step 3 */
        actualStep = step_restore_localfs_e;
        if (restoreLocalFs() == false)
        {
            finishProcess();
            return;
        }
    }
    else if (actualStep == step_none_e && arrayQueue[step_restore_rootfs_e] == 1)
    {
        /* start step 2 */
        actualStep = step_restore_rootfs_e;
        if (restoreRootFs() == false)
        {
            finishProcess();
            return;
        }
    }
    else if (actualStep == step_none_e && arrayQueue[step_restore_kernel_e] == 1)
    {
        /* start step 1 */
        actualStep = step_restore_kernel_e;
        if (restoreKernel() == false)
        {
            finishProcess();
            return;
        }
    }
    else
    {
        /* nothing to do */
        if (actualStep == step_none_e)
        {
            loadInfo();
            ui->checkBoxBackupKernel->setEnabled(true);
            ui->checkBoxBackupLocalFS->setEnabled(true);
            ui->checkBoxBackupRootFS->setEnabled(true);
            ui->checkBoxRestoreKernel->setEnabled(true);
            ui->checkBoxRestoreLocalFS->setEnabled(true);
            ui->checkBoxRestoreRootFS->setEnabled(true);
            ui->pushButtonStart->setEnabled(true);
        }
        /* working */
        else
        {
            ui->checkBoxBackupKernel->setEnabled(false);
            ui->checkBoxBackupLocalFS->setEnabled(false);
            ui->checkBoxBackupRootFS->setEnabled(false);
            ui->checkBoxRestoreKernel->setEnabled(false);
            ui->checkBoxRestoreLocalFS->setEnabled(false);
            ui->checkBoxRestoreRootFS->setEnabled(false);
            ui->pushButtonStart->setEnabled(false);
            if (points.length() > POINTS_NB)
            {
                points.clear();
            }
            else
            {
                points += ".";
            }
            ui->labelinfo->setText(
                        QString("%1 %2%3")
                        .arg((actualStep <= step_bkup_localfs_e)?"Saving":"Restoring")
                        .arg(arrayStepName[actualStep])
                        .arg(points)
                        );
            QPixmap picture = QPixmap(QString(":/icons/img/Loading%1.png").arg(points.length()%4));
            switch (actualStep)
            {
            case step_bkup_kernel_e:
                ui->labelCloneKernelStatus->setPixmap(picture);
                break;
            case step_bkup_localfs_e:
                ui->labelCloneLocalfsStatus->setPixmap(picture);
                break;
            case step_bkup_rootfs_e:
                ui->labelCloneRootfsStatus->setPixmap(picture);
                break;
            case step_restore_kernel_e:
                ui->labelRestoreKernelStatus->setPixmap(picture);
                break;
            case step_restore_localfs_e:
                ui->labelRestoreLocalfsStatus->setPixmap(picture);
                break;
            case step_restore_rootfs_e:
                ui->labelRestoreRootfsStatus->setPixmap(picture);
                break;
            }
        }
        ui->labelinfo->update();
    }
}

cloner::~cloner()
{
    delete ui;
}

void cloner::on_pushButtonStart_clicked()
{
    actualStep = step_none_e;
    loadInfo();
    /* perform the backup */
    if (ui->checkBoxBackupLocalFS->isChecked())
    {
        arrayQueue[step_bkup_localfs_e] = 1;
        ui->labelCloneLocalfsStatus->setVisible(true);
    }
    else
    {
        ui->labelCloneLocalfsStatus->setVisible(false);
    }
    ui->checkBoxBackupLocalFS->setVisible(!ui->labelCloneLocalfsStatus->isVisible());
    if (ui->checkBoxBackupRootFS->isChecked())
    {
        arrayQueue[step_bkup_rootfs_e] = 1;
        ui->labelCloneRootfsStatus ->setVisible(true);
    }
    else
    {
        ui->labelCloneRootfsStatus->setVisible(false);
    }
    ui->checkBoxBackupRootFS->setVisible(!ui->labelCloneRootfsStatus->isVisible());
    if (ui->checkBoxBackupKernel->isChecked())
    {
        arrayQueue[step_bkup_kernel_e] = 1;
        ui->labelCloneKernelStatus->setVisible(true);
    }
    else
    {
        ui->labelCloneKernelStatus->setVisible(false);
    }
    ui->checkBoxBackupKernel->setVisible(!ui->labelCloneKernelStatus->isVisible());

    /* perform the restore */
    if (ui->checkBoxRestoreLocalFS->isChecked())
    {
        arrayQueue[step_restore_localfs_e] = 1;
        ui->labelRestoreLocalfsStatus ->setVisible(true);
    }
    else
    {
        ui->labelRestoreLocalfsStatus->setVisible(false);
    }
    ui->checkBoxRestoreLocalFS->setVisible(!ui->labelRestoreLocalfsStatus->isVisible());
    if (ui->checkBoxRestoreRootFS->isChecked())
    {
        arrayQueue[step_restore_rootfs_e] = 1;
        ui->labelRestoreRootfsStatus ->setVisible(true);
    }
    else
    {
        ui->labelRestoreRootfsStatus->setVisible(false);
    }
    ui->checkBoxRestoreRootFS->setVisible(!ui->labelRestoreRootfsStatus->isVisible());
    if (ui->checkBoxRestoreKernel->isChecked())
    {
        arrayQueue[step_restore_kernel_e] = 1;
        ui->labelRestoreKernelStatus ->setVisible(true);
    }
    else
    {
        ui->labelRestoreKernelStatus->setVisible(false);
    }
    ui->checkBoxRestoreKernel->setVisible(!ui->labelRestoreKernelStatus->isVisible());
}

void cloner::finishProcess()
{
    fprintf(stderr, "exitCode: %d\n",exitArray[actualStep]);
    sync();
    QPixmap picture;
    if (exitArray[actualStep] != 0)
    {
        QMessageBox::critical(0,"Cloner", QString("Operation '%1' Fail! [%2][%3]").arg(arrayStepName[actualStep]).arg(exitArray[actualStep]).arg(stringError));
        picture = QPixmap(":/icons/img/Delete.png");
    }
    else
    {
        picture = QPixmap(":/icons/img/Apply.png");
    }
    arrayQueue[actualStep] = step_none_e;
    switch (actualStep)
    {
    case step_bkup_localfs_e:
        ui->labelCloneLocalfsStatus->setPixmap(picture);
        break;
    case step_bkup_rootfs_e:
        ui->labelCloneRootfsStatus->setPixmap(picture);
        break;
    case step_bkup_kernel_e:
        ui->labelCloneKernelStatus->setPixmap(picture);
        break;
    case step_restore_localfs_e:
        ui->labelRestoreLocalfsStatus->setPixmap(picture);
        break;
    case step_restore_rootfs_e:
        ui->labelRestoreRootfsStatus->setPixmap(picture);
        break;
    case step_restore_kernel_e:
        ui->labelRestoreKernelStatus->setPixmap(picture);
        break;
    }
    actualStep = step_none_e;
}

bool cloner::backupLocalFs()
{
    /* tar.bz2 for the local folder into the actual path */
    char command[COMMAND_LEN];
    sprintf(command, "%s/backup/local.tar.bz2", USB_MOUNT_POINT);
    if (QFile::exists(command))
    {
        if (QMessageBox::question(this, tr("Cloner"), tr("a %1 clone already exist. Do you want overwrite?").arg(arrayStepName[actualStep]), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
        {
            if (QFile::remove(command) == false)
            {
                return false;
            }
        }
        else
        {
            exitArray[actualStep] = 0;
            return false;
        }
    }
    sprintf(command,
            "/etc/rc.d/init.d/sdcheck stop"
            " && "
            "cd /local"
            " && "
            "rm -rf data"
            " && "
            "tar cf %s/backup/local.tar *"
            " && "
            "%s/bzip2 %s/backup/local.tar",
            USB_MOUNT_POINT,
            APP_DIR_PATH,
            USB_MOUNT_POINT
            );

    fprintf(stderr, "backupLocalFs@%s@\n", command);

    if (mp.setCommand(command) == false)
    {
        return false;
    }
    mp.start();
    fprintf(stderr, "backupLocalFs@%s@\n", command);
    return true;
}

bool cloner::backupRootFs()
{

    /* tar.bz2 for the rootfs folder into the actual path */
    char command[COMMAND_LEN];
    sprintf(command, "%s/backup/rootfs.tar.bz2", USB_MOUNT_POINT);
    if (QFile::exists(command))
    {
        if (QMessageBox::question(this, tr("Cloner"), tr("a %1 clone already exist. Do you want overwrite?").arg(arrayStepName[actualStep]), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
        {
            if (QFile::remove(command) == false)
            {
                return false;
            }
        }
        else
        {
            exitArray[actualStep] = 0;
            return false;
        }
    }
    sprintf(command,
            "cd /"
            " && "
            "tar cf %s/backup/rootfs.tar bin dev etc lib opt sbin share usr"
            " && "
            "%s/bzip2 %s/backup/rootfs.tar",
            USB_MOUNT_POINT,
            APP_DIR_PATH,
            USB_MOUNT_POINT
            );
    if (mp.setCommand(command) == false)
    {
        return false;
    }
    mp.start();
    return true;
}

bool cloner::backupKernel()
{
    QMessageBox::information(0,"Cloner", "Backup kernel not implemented yet!");
    exitArray[actualStep] = 1;
    return false;
}

bool cloner::restoreLocalFs()
{
    char command[COMMAND_LEN];

    if (ui->checkBoxResetSnNet->isChecked())
    {
        sprintf(command,
				"/etc/rc.d/init.d/sdcheck stop; "
                "mount -o rw,remount /"
                " && "
                "cp %s/restore/local.tar.bz2 %s/restore/.local.tar.bz2"
                " && "
                "%s/bunzip2 %s/restore/.local.tar.bz2"
                " && "
                "rm -rf /local/*; "
                "tar xf %s/restore/.local.tar -C /local; ERROR=$?; "
                "rm %s/restore/.local.tar; "
                "mount -o ro,remount /; "
                "[ $ERROR = 0 ]",
                USB_MOUNT_POINT,
                USB_MOUNT_POINT,
                APP_DIR_PATH,
                USB_MOUNT_POINT,
                USB_MOUNT_POINT,
                USB_MOUNT_POINT
                );
    }
    else
    {
        sprintf(command,
				"/etc/rc.d/init.d/sdcheck stop; "
                "mount -o rw,remount /"
                " && "
                "cp /local/etc/sysconfig/serial.conf %s/serial.conf; "
                "cp /local/etc/sysconfig/net.conf %s/net.conf; "
                "cp %s/restore/local.tar.bz2 %s/restore/.local.tar.bz2"
                " && "
                "%s/bunzip2 %s/restore/.local.tar.bz2"
                " && "
                "rm -rf /local/*; "
                "tar xf %s/restore/.local.tar -C /local; ERROR=$?; "
                "rm %s/restore/.local.tar; "
                "mv %s/serial.conf /local/etc/sysconfig/serial.conf; "
                "mv %s/net.conf /local/etc/sysconfig/net.conf; "
                "mount -o ro,remount /; "
                "[ $ERROR = 0 ]",
                USB_MOUNT_POINT,
                USB_MOUNT_POINT,
                USB_MOUNT_POINT,
                USB_MOUNT_POINT,
                APP_DIR_PATH,
                USB_MOUNT_POINT,
                USB_MOUNT_POINT,
                USB_MOUNT_POINT,
                USB_MOUNT_POINT,
                USB_MOUNT_POINT
                );
    }
    if (mp.setCommand(command) == false)
    {
        return false;
    }
    mp.start();
    return true;
}

bool cloner::restoreRootFs()
{
    char command[COMMAND_LEN];
    sprintf(command,
            "mount -o rw,remount /"
            " && "
            "cp %s/restore/rootfs.tar.bz2 %s/restore/.rootfs.tar.bz2"
            " && "
            "%s/bunzip2 %s/restore/.rootfs.tar.bz2"
            " && "
            "tar xf %s/restore/.rootfs.tar -C /; ERROR=$?"
            " && "
            "rm %s/restore/.rootfs.tar; "
            "mount -o ro,remount /; "
            "[ $ERROR = 0 ]",
            USB_MOUNT_POINT,
            USB_MOUNT_POINT,
            APP_DIR_PATH,
            USB_MOUNT_POINT,
            USB_MOUNT_POINT,
            USB_MOUNT_POINT
            );
    if (mp.setCommand(command) == false)
    {
        return false;
    }
    mp.start();
    return true;
}

bool cloner::restoreKernel()
{
    char command[COMMAND_LEN];
    sprintf(command, "%s/flash_eraseall /dev/mtd0 && %s/kobs-ng init %s/restore/imx28_ivt_linux.sb",
            QApplication::applicationDirPath().toAscii().data(),
            QApplication::applicationDirPath().toAscii().data(),
            USB_MOUNT_POINT
            );
    if (mp.setCommand(command) == false)
    {
        return false;
    }
    mp.start();
    return true;
}

bool cloner::loadInfo()
{
    char line[1024];
#if 0
    FILE * fp = popen(
                "sed -e 's/#/\\n/g' /proc/version | sed -e 's/) (/)\\n(/g'"
                " && "
                "cat /proc/cpuinfo | grep Hardware | cut -d: -f2 ",
                "r");
#endif
    FILE * fp = popen(
                "echo -n 'IP:' "
                " && "
                "ip addr show eth0  | tail -2"
                " && "
                "echo -n 'SN: '"
                " && "
                "[ -e /local/etc/sysconfig/serial.conf ]"
                " && "
                "cat /local/etc/sysconfig/serial.conf"
                " || "
                "echo '-'",
                "r");
    if (fp == NULL)
    {
        return false;
    }
    QString info;
    while (fgets(line, 1024, fp) != NULL)
    {
        info.append(line);
    }
    ui->labelinfo->setText(info);
    fclose(fp);
    return true;
}

void cloner::on_pushButtonBackup_toggled(bool checked)
{
    ui->checkBoxBackupKernel->setChecked(checked);
    ui->checkBoxBackupLocalFS->setChecked(checked);
    ui->checkBoxBackupRootFS->setChecked(checked);

}

void cloner::on_pushButtonRestore_toggled(bool checked)
{
    ui->checkBoxRestoreKernel->setChecked(checked);
    ui->checkBoxRestoreLocalFS->setChecked(checked);
    ui->checkBoxRestoreRootFS->setChecked(checked);
}
