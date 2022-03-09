#ifndef MAINCLONER_H
#define MAINCLONER_H

#include "publics.h"

#include <QWidget>
#include <QTimer>
#include <QString>
#include <QStringList>
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
    void execCommandList();

private:
    bool        loadInfo();
    void        restoreLocalFile(QString &szLocalTar, QStringList &files2Exclude, int nRetentiveMode, int nHmiIniMode, int nLogMode);
    QString     getDefaultDirName();


    Ui::MainCloner *ui;
    // Variabili nuove
    QStringList     infoList;
    QElapsedTimer   startElapsed;
    int             runningAction;
    QString         szDestination;
    QString         szSource;
    QString         szRunningCommand;
    QStringList     commandList;
    QStringList     excludesRFSList;        // Root file system  Exclude
    QStringList     excludesLFSList;        // Local file system Exclude

    // Variabili di Cloner
    QTimer      *refresh_timer;
    char backupDir[STR_LEN];
    char restoreDir[STR_LEN];

};

#endif // MAINCLONER_H
