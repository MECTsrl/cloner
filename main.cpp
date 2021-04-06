#include "maincloner.h"
// #include "ntpclient.h"

#include <QApplication>
#include <QFile>
#include <QString>
#include <QStringList>

#if QT_VERSION < 0x050000
#include <QWSServer>
#endif

#define QSS_FILE ":/qss/cloner.qss"


QString     szModel;                // Target Model
QString     szTargetVersion;        // MS Target Version
QString     szClonerVersion;        // Cloner App Version
QStringList excludesRFSList;        // Root file system  Exclude
QStringList excludesLFSList;        // Local file system Exclude
QString     sysUpdateModelFile;     // Sysupdate Model File
QString     mfgToolsModelDir;       // MFG Tools Model Directory (contains Local)
QString     simpleModelFile;        // Simple Local file for current Target Model

// NtpClient   *ntpclient;             // NTP Interface

int main(int argc, char *argv[])
{
#ifdef Q_WS_QWS

    int myargc = 4;
    char *myargv[] =
    {
        argv[0],
        strdup("-qws"),
        strdup("-display"),
        strdup("VNC:LinuxFb")
    };
    QApplication a(myargc, myargv);
#else
    QApplication a(argc, argv);
#endif
    QWSServer::setCursorVisible(false);
    // ntpclient = new NtpClient(NULL);
    // Qss globale di applicazione
    QFile   fileQSS(QSS_FILE);
    if (fileQSS.exists())  {
        fileQSS.open(QFile::ReadOnly);
        QString styleSheet = QString(fileQSS.readAll());
        fileQSS.close();
        a.setStyleSheet(styleSheet);
    }
    MainCloner w;
    w.showFullScreen();

    return a.exec();
}
