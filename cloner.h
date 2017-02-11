#ifndef CLONER_H
#define CLONER_H

#include <QWidget>
#include <QTimer>
#include <QProcess>
#include <QThread>

#define COMMAND_LEN 65536

namespace Ui {
class cloner;
}

enum step_e{
    step_none_e,
    step_bkup_localfs_e,
    step_bkup_rootfs_e,
    step_bkup_kernel_e,
    step_restore_localfs_e,
    step_restore_rootfs_e,
    step_restore_kernel_e,
    step_nb
};

class myProcess : public QThread
{
public:
    myProcess(void);
    ~myProcess(void);
    bool setCommand(char * command);
    virtual void run(void);
private:
    char _command[COMMAND_LEN];
    int exitCode;
};

class cloner : public QWidget
{
    Q_OBJECT
    
public:
    explicit cloner(QWidget *parent = 0);
    ~cloner();
    
private slots:

    void updateData();

    void finishProcess();

    void on_comboBoxImages_currentIndexChanged(const QString &arg1);

    void on_pushButtonBackup_clicked();

    void on_pushButtonInstall_clicked();

private:
    bool backupLocalFs();
    bool backupRootFs();
    bool backupKernel();
    bool restoreLocalFs();
    bool restoreRootFs();
    bool restoreKernel();
    bool loadInfo();
    QString getDefaultDirName();
    int getIP(const char * interface, char * ip);
    int getMAC(const char *interface, char * mac);

private:
    Ui::cloner *ui;
    QTimer * refresh_timer;
    QString points;
    myProcess mp;
    int arrayQueue[step_nb];
    char backupDir[256];
    char restoreDir[256];
};

#endif // CLONER_H
