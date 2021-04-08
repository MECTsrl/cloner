#ifndef MAINCLONER_H
#define MAINCLONER_H

#include <QWidget>
#include <QTimer>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QElapsedTimer>

namespace Ui {
class MainCloner;
}

class MainCloner : public QWidget
{
    Q_OBJECT

public:
    explicit MainCloner(QWidget *parent = 0);
    ~MainCloner();

private slots:
    void updateData();
    void on_cmdBackup_clicked();
    void on_cmdRestore_clicked();
    void on_cmdVPN_clicked();
    void on_cmdSSH_clicked();
    void on_cmdMectSuite_clicked();
    void on_cmdClock_clicked();
    void on_cmdNetwork_clicked();
    void on_cmdSimple_clicked();
    void on_cmdMenu_clicked();
    void actionCompleted(int exitCode, QProcess::ExitStatus exitStatus);
    void actionFailed(QProcess::ProcessError errorCode);

private:
    bool        loadInfo();
    void        restoreLocalFile(QString &szLocalTar, QStringList &files2Exclude, int nRetentiveMode);
    QString     getDefaultDirName();


    Ui::MainCloner *ui;
    // Variabili nuove
    QStringList     infoList;
    QProcess        *myProcess;
    QElapsedTimer   startElapsed;
    int             runningAction;
    int             nCommandCount;
    QString         szDestination;
    QString         szSource;
    QString         runningCommand;
    QStringList     commandList;
    QStringList     excludesRFSList;        // Root file system  Exclude
    QStringList     excludesLFSList;        // Local file system Exclude

    // Variabili di Cloner
    QTimer      *refresh_timer;
    char backupDir[256];
    char restoreDir[256];

};

#endif // MAINCLONER_H
