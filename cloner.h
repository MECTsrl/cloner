#ifndef CLONER_H
#define CLONER_H

#include <QWidget>
#include <QTimer>
#include <QProcess>
#include <QThread>

#define COMMAND_LEN 1024

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

    void on_pushButtonStart_clicked();

    void updateData();

    void finishProcess();

    void on_pushButtonBackup_toggled(bool checked);

    void on_pushButtonRestore_toggled(bool checked);

private:
    bool backupLocalFs();
    bool backupRootFs();
    bool backupKernel();
    bool restoreLocalFs();
    bool restoreRootFs();
    bool restoreKernel();
    bool loadInfo();

private:
    Ui::cloner *ui;
    QTimer * refresh_timer;
    QString points;
    myProcess mp;
    int arrayQueue[step_nb];
};

#endif // CLONER_H
