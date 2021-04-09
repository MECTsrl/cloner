#include "maincloner.h"
#include "ntpclient.h"

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
QString     sysUpdateModelFile;     // Sysupdate Model File
QString     mfgToolsModelDir;       // MFG Tools Model Directory (contains Local)
QString     szAlphaStyle;           // Alphanumpad Stylesheet String
int         screen_width;           // Screen width  in Pixel
int         screen_height;          // Screen height in Pixel
NtpClient   *ntpclient;             // NTP Interface

int main(int argc, char *argv[])
{
#ifdef Q_WS_QWS
#define  WIDTH 480
#define  HEIGHT 272
#define   ARGNO 4

    int myargc = ARGNO;
    char *myargv[ARGNO];
    myargv[0] = argv[0];
    myargv[1] = strdup("-qws");
    myargv[2] = strdup("-display");

    screen_width = WIDTH;
    screen_height = HEIGHT;
    // Reading Frame Buffer Pixel Size
    QFile virtual_size("/sys/devices/platform/mxs-fb.0/graphics/fb0/virtual_size");
    if (virtual_size.open(QIODevice::ReadOnly)) {
        char buf[42];
        if (virtual_size.readLine(buf, 42) > 0) {
            int w = screen_width, h = screen_width;

            if (sscanf(buf, "%d,%d", &w, &h) == 2) {
                screen_width = w;
                screen_height = h;
            }
        }
    }
    // Starting QApplication
    if (screen_width == 800)  {
        myargv[3] = strdup("VNC:LinuxFb:mmWidth=152:mmHeight=91");
    }
    else  {
        myargv[3] = strdup("VNC:LinuxFb:mmWidth=95:mmHeight=56");
    }
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
