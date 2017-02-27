#include "cloner.h"
#include "ui_cloner.h"
#include "alphanumpad.h"
#include <QMessageBox>
#include <QTimer>
#include <QDateTime>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <errno.h>
#include <stdio.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>


#define REFRESH_MS 1000
#define POINTS_NB 30
#define CLONED_IMAGES_DIR "/tmp/mnt/cloner"
#define TMP_DIR "/tmp/tmp"
#define EXCLUDES_RFS "excludes_rootfs.lst"
#define EXCLUDES_LFS "excludes_localfs.lst"

char arrayStepName[step_nb][18] = {
    "None",
    "Local file system",
    "Root file system",
    "Kernel",
    "Local file system",
    "Root file system",
    "Kernel"
};

int actualStep = step_none_e;
int exitArray[step_nb];
char stringError[COMMAND_LEN];

// Exclude lists for all clonable file systems.
QStringList excludesRFSList;    // Root file system.
QStringList excludesLFSList;    // Local file system.

myProcess::myProcess(void)
{
    _command[0] = '\0';
}

myProcess::~myProcess(void)
{
}

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
    exit(exitArray[actualStep]);
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

    ui->label->setText(QString("Cloner v%1").arg(SVN_REV));

    QDir imagesDir(CLONED_IMAGES_DIR);
    QDir distDir(QCoreApplication::applicationDirPath());

    /* Fill the list with all existing clones. */
    ui->comboBoxImages->clear();        // FIXME Not working
    ui->comboBoxImages->addItems(imagesDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot));

    /* Reset the status line. */
    ui->labelStatus->setText("");


    // Load the exclude list for the root file system.
    excludesRFSList.prepend("");
    if (distDir.exists(EXCLUDES_RFS)) {
        QFile excludesRFS(distDir.filePath(EXCLUDES_RFS));
        if (excludesRFS.open(QIODevice::ReadOnly))
            while (!excludesRFS.atEnd())
                excludesRFSList.append(excludesRFS.readLine().simplified());
    }

    // Load the exclude list for the local file system.
    excludesLFSList.prepend("");
    if (distDir.exists(EXCLUDES_LFS)) {
        QFile excludesLFS(distDir.filePath(EXCLUDES_LFS));
        if (excludesLFS.open(QIODevice::ReadOnly))
            while (!excludesLFS.atEnd())
                excludesLFSList.append(excludesLFS.readLine().simplified());
    }
}

cloner::~cloner()
{
    delete ui;
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
            ui->pushButtonBackup->setEnabled(true);
            ui->pushButtonInstall->setEnabled(true);
        }
        /* working */
        else
        {
            ui->pushButtonBackup->setEnabled(false);
            ui->pushButtonInstall->setEnabled(false);

            if (points.length() > POINTS_NB)
                points.clear();
            points += ".";

            ui->labelStatus->setStyleSheet("color: rgb(0,0,128);");
            ui->labelStatus->setText(QString("Transferring %1%2").arg(arrayStepName[actualStep]).arg(points));
        }
        ui->labelStatus->update();
    }
}

void cloner::finishProcess()
{
    fprintf(stderr, "exitCode: %d\n",exitArray[actualStep]);

    sync();

    points.clear();
    if (exitArray[actualStep] != 0) {
        QMessageBox::critical(0,"Cloner", QString("Operation '%1' failed. [%2][%3]").arg(arrayStepName[actualStep]).arg(exitArray[actualStep]).arg(stringError));
    }
    else {
        ui->labelStatus->setStyleSheet("color: rgb(0,128,0);");
        ui->labelStatus->setText( QString("Operation '%1' completed.").arg(arrayStepName[actualStep]));
    }

    arrayQueue[actualStep] = step_none_e;
    actualStep = step_none_e;
    //loadInfo();
}

bool cloner::backupLocalFs()
{
    QDir().mkdir(backupDir);
    /* tar for the local folder into the actual path */
    char command[COMMAND_LEN];
    sprintf(command, "%s/localfs.tar", backupDir);
    if (QFile::exists(command))
    {
        if (QMessageBox::question(this, tr("Cloner"), tr("Clone for %1 exists. Overwrite?").arg(arrayStepName[actualStep]), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
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

    /*** LOCAL FS
     *  /etc/rc.d/init.d/sdcheck stop
     *  tar cf /mnt/floppy/backup/localfs.tar -C /local .
     */
    sprintf(command,
        "/etc/rc.d/init.d/sdcheck stop"
        " ; "
        "tar cf %s/localfs.tar -C /local ."
        " ; "
        "sync"
        ,
        backupDir
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
    QDir().mkdir(backupDir);
    /* tar for the rootfs folder into the actual path */
    char command[COMMAND_LEN];
    sprintf(command, "%s/rootfs.tar", backupDir);
    if (QFile::exists(command)) {
        if (QMessageBox::question(this, tr("Cloner"), tr("Clone for %1 exists. Overwrite?").arg(arrayStepName[actualStep]), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok) {
            if (QFile::remove(command) == false)
                return false;
        }
        else {
            exitArray[actualStep] = 0;

            return false;
        }
    }

    /*** ROOT FS
     *  tar cf /mnt/floppy/backup/rootfs.tar -C / .
     */
    sprintf(command,
        "tar cf %s/rootfs.tar -C / `find / -maxdepth 1 -type d -print | grep -v '^/\\(local\\|tmp\\|mnt\\|dev\\|sys\\|proc\\|\\)$'`"
        " ; "
        "sync"
        ,
        backupDir
    );
    if (mp.setCommand(command) == false)
        return false;

    mp.start();

    return true;
}

// TODO MTL
bool cloner::backupKernel()
{
    QMessageBox::information(0,"Cloner", "Kernel clone not yet implemented.");

    exitArray[actualStep] = 1;

    return false;
}

bool cloner::restoreLocalFs()
{
    char command[COMMAND_LEN];

    /*** LOCAL FS
     *  cd /
     *  /etc/rc.d/init.d/sdcheck stop
     *  mkdir -p /tmp/mnt
     *  /bin/mount -t tmpfs -o size=128M tmpfs /tmp/mnt
     *  tar xf /mnt/floppy/restore/localfs.tar -C /tmp/mnt
     *  cp /local/etc/sysconfig/net.conf /tmp/
     *  rsync -Hlrax /tmp/mnt/ /local
     *  cp /tmp/net.conf /local/etc/sysconfig/
     *  /bin/umount /tmp/mnt
     */
    /* before 2.0 */
    if (!QFile::exists("/etc/mac.conf")) {
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

    QByteArray excludesLFSListBA = excludesLFSList.join(" --exclude ").toLatin1();
    sprintf(command,
        "cd /"
        " ; "
        "/etc/rc.d/init.d/sdcheck stop"
        " ; "
        "mkdir -p %s"
        " ; "
        "/bin/mount -t tmpfs -o size=128M tmpfs %s"
        " && "
        "tar xf %s/localfs.tar -C %s"
        " && "
        "rsync -Havx %s/ /local/ %s"
        " ; "
        "sync"
        " ; "
        "/bin/umount %s"
        ,
        TMP_DIR,
        TMP_DIR,
        restoreDir,
        TMP_DIR,
        TMP_DIR,
        excludesLFSListBA.data(),
        TMP_DIR
    );

    if (mp.setCommand(command) == false)
        return false;

    mp.start();

    return true;
}

bool cloner::restoreRootFs()
{
    /*** ROOT FS
     * /etc/rc.d/init.d/boa stop
     * cd /
     * mkdir -p /tmp/mnt
     * /bin/mount -t tmpfs -o size=128M tmpfs /tmp/mnt
     * tar xf /mnt/floppy/restore/rootfs.tar -C /tmp/mnt
     * mount -o rw,remount /
     * rsync -Hlrax /tmp/mnt/ /  --exclude=/etc/mac.conf --exclude=/etc/serial.conf
     * mount -o ro,remount /
     * /bin/umount /tmp/mnt
     */
    char command[COMMAND_LEN];
    QByteArray excludesRFSListBA = excludesRFSList.join(" --exclude ").toLatin1();
    sprintf(command,
        "/etc/rc.d/init.d/boa stop"
        " ; "
        "cd /"
        " ; "
        "mkdir -p %s"
        " && "
        "/bin/mount -t tmpfs -o size=128M tmpfs %s"
        " && "
        "tar xf %s/rootfs.tar -C %s"
        " && "
        "mount -o rw,remount /"
        " && "
        "rsync -Havx %s/ / %s"
        " && "
        "mount -o ro,remount /"
        " ; "
        "sync"
        " ; "
        "/bin/umount %s"
        ,
        TMP_DIR,
        TMP_DIR,
        restoreDir,
        TMP_DIR,
        TMP_DIR,
        excludesRFSListBA.data(),
        TMP_DIR
    );

    if (mp.setCommand(command) == false)
        return false;

    mp.start();

    return true;
}

// TODO MTL
bool cloner::restoreKernel()
{
    char command[COMMAND_LEN];
    sprintf(command, "%s/flash_eraseall /dev/mtd0 && %s/kobs-ng init %s/imx28_ivt_linux.sb",
        QApplication::applicationDirPath().toAscii().data(),
        QApplication::applicationDirPath().toAscii().data(),
        restoreDir
    );
    if (mp.setCommand(command) == false)
        return false;

    mp.start();

    return true;
}

int cloner::getMAC(const char *interface, char * mac)
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "interface" */
    strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) != 0)
        return -1;

    //close(fd);
    unsigned char mac_address[6]= "";
    memcpy(mac_address, ifr.ifr_hwaddr.sa_data, 6);
    int j = 0;
    int i = 0;
    for (i = 0, j = 0; i < 6; i++, j += 3)
        sprintf(&(mac[j]), "%02X:", mac_address[i]);
    mac[j-1] = '\0';

    return 0;
}

int cloner::getIP(const char * interface, char * ip)
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want IP address attached to "interface" */
    strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);

    if (ioctl(fd, SIOCGIFADDR, &ifr) != 0)
    {
        return -1;
    }

    //close(fd);

    /* display result */
    strcpy(ip, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

    return 0;
}


/*
SN:
MAC:
IP:
Release: 2.0rc4
Target:  TP1043_01_A
Qt:      4.8.5
Qwt:     6.1-multiaxes
RunTime: master/v1.007
MectPlugin: mect_suite_2.0/v7.0rc24
MectApps: mect_suite_2.0/v2.0rc4
*/
bool cloner::loadInfo()
{
    QString info;
    char line[1024];
    FILE * fp;

    fp = fopen("/etc/serial.conf", "r");
    if (fp)
    {
        if (fscanf(fp, "%s", line) != 1)
        {
            info.append("SN:");
            info.append(line);
        }
        fclose(fp);
    }
    else
    {
        fp = fopen("/local/etc/sysconfig/serial.conf", "r");
        if (fp)
        {
            if (fscanf(fp, "%s", line) != 1)
            {
                info.append("SN:");
                info.append(line);
            }
            fclose(fp);
        }
    }
    if (getIP("eth0", line) == 0)
    {
        info.append("IP:");
        info.append(line);
        info.append("\n");
    }

    ui->labelStatus->setStyleSheet("color: rgb(0,0,0);");
    ui->labelStatus->setText(info);
    return true;
}


void cloner::on_comboBoxImages_currentIndexChanged(const QString &arg1)
{
    sprintf(restoreDir, "%s/%s", CLONED_IMAGES_DIR, arg1.toAscii().data());
    sprintf(backupDir, "%s/%s", CLONED_IMAGES_DIR, arg1.toAscii().data());
    if(arg1.length() > 0)
    {
        char command[COMMAND_LEN];
        sprintf(command, "%s/localfs.tar", restoreDir);
        if (QFile::exists(command))
        {
            ui->pushButtonInstall->setEnabled(true);
        }
        else
        {
            ui->pushButtonInstall->setEnabled(false);
        }
    }
}

QString cloner::getDefaultDirName()
{
    FILE * fp;
    char version[32] = "";
    char target[32] = "";
    char line[256] = "";
    fp = fopen("/rootfs_version", "r");
    if (fp != NULL)
    {
        /*
         * Release: 2.0rc4
         * Target:  TPAC1007_03
         */
        if (fscanf(fp, "%*s %s", version) != 1)
        {
            fclose(fp);
            return QDateTime::currentDateTime().toString("yyyy_MM_dd");
        }
        if (fscanf(fp, "%*s %s", target) != 1)
        {
            fclose(fp);
            return QDateTime::currentDateTime().toString("yyyy_MM_dd");
        }
        fclose(fp);
        return  QDateTime::currentDateTime().toString("yyyy_MM_dd") + QString("_%1_%2").arg(target).arg(version);
    }
    else
    {
        fp = fopen("/proc/cpuinfo", "r");
        if (fp != NULL)
        {
            while (fgets(line, 1024, fp) != NULL)
            {
                if (strncmp(line, "Hardware        :", strlen("Hardware        :")) == 0)
                {
                    fclose(fp);
                    return QString(line).split(":").at(2).simplified().replace(" ","_");
                }
            }
        }
        else
        {
            return QDateTime::currentDateTime().toString("yyyy_MM_dd");
        }
    }
    return QDateTime::currentDateTime().toString("yyyy_MM_dd");
}

void cloner::on_pushButtonBackup_clicked()
{
    actualStep = step_none_e;
    loadInfo();

        alphanumpad * dk;
        char value[256];
        dk = new alphanumpad(value, getDefaultDirName().toAscii().data());
        dk->showFullScreen();

        if (dk->exec() == QDialog::Accepted && strlen(value) != 0)
        {
            QDir().mkdir(QString("%1/%2").arg(CLONED_IMAGES_DIR).arg(value));
            ui->comboBoxImages->addItem(value);
            ui->comboBoxImages->setCurrentIndex(ui->comboBoxImages->count()-1);
            fprintf(stderr, "%s\n", backupDir);
            sprintf(backupDir, "%s/%s", CLONED_IMAGES_DIR, value);
        }
        else
        {
            return;
        }

    char command[256];
    sprintf(command, "mkdir -p %s", backupDir);
    system(command);

    /* Request backup operations. */
    arrayQueue[step_bkup_localfs_e] = 1;
}

void cloner::on_pushButtonInstall_clicked()
{
    actualStep = step_none_e;
    loadInfo();

    if (ui->comboBoxImages->currentText().length() == 0)
    {
        alphanumpad * dk;
        char value[256];
        dk = new alphanumpad(value, getDefaultDirName().toAscii().data());
        dk->showFullScreen();

        if (dk->exec() == QDialog::Accepted && strlen(value) != 0)
        {
            QDir().mkdir(QString("%1/%2").arg(CLONED_IMAGES_DIR).arg(value));
            ui->comboBoxImages->addItem(value);
            ui->comboBoxImages->setCurrentIndex(ui->comboBoxImages->count()-1);
            fprintf(stderr, "%s\n", backupDir);
            sprintf(backupDir, "%s/%s", CLONED_IMAGES_DIR, value);
        }
        else
        {
            return;
        }
    }
    else
    {
        sprintf(backupDir, "%s/%s", CLONED_IMAGES_DIR, ui->comboBoxImages->currentText().toAscii().data());
    }

    char command[256];
    sprintf(command, "mkdir -p %s", backupDir);
    system(command);

    /* Request restore operations. */
    arrayQueue[step_restore_localfs_e] = 1;
}
