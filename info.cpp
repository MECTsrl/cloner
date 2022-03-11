#include "info.h"
#include "ui_info.h"

#include "myntpclient.h"

#include "publics.h"

#include <arpa/inet.h>
#include <QPlainTextEdit>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>

#define DNS_FILE        "/etc/resolv.conf"
#define ROUTING_FILE    "/proc/net/route"

#define USB0_DEVICE     "/proc/bus/usb/001/"
#define USB1_DEVICE     "/proc/bus/usb/002/"
#define SDCARD_DEVICE   "/dev/mmcblk0"


Info::Info(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Info)
{
    ui->setupUi(this);
    ui->lblAction->setText("Target Info");
    ui->lblModel->setText(szModel);
    refreshAllTabs();
    ui->tabWidget->setCurrentIndex(0);
    startTimer(REFRESH_MS);
}

Info::~Info()
{
    delete ui;
}

void Info::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    // Aggiornamento orologio
    QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->lblDateTime->setText(szDateTime);
}

void Info::on_cmdBack_clicked()
{
    this->reject();
}

void Info::on_cmdRelolad_clicked()
{
    refreshAllTabs();
}

void Info::refreshAllTabs()
{
    refreshSystemTab();
    refreshNetworkingTabs();
    refreshNTPInfo();
}

void Info::refreshSystemTab()
{
    bool    usb0Inserted = false;
    bool    usb1Inserted = false;
    bool    sdCardPresent = false;
    int     nFiles = 0;

    ui->sys_text->setPlainText("");
    ui->sys_text->appendPlainText(QString("Product Id:\t[ %1]") .arg(szModel));
    ui->sys_text->appendPlainText(QString("S/N:\t[%1]") .arg(szSerialNO));
    ui->sys_text->appendPlainText(QString("Release:\t[%1]") .arg(szTargetVersion));
    ui->sys_text->appendPlainText(QString("Qt:\t[%1]") .arg(szQtVersion));
    ui->sys_text->appendPlainText("");
    // USB0
    QDir usb0(USB0_DEVICE);
    if (usb0.exists())  {
        nFiles = usb0.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name).count();
        // Se nel Device Path esiste almeno un file OLTRE all'Hub, è presente un Device
        if (nFiles > 1)  {
            usb0Inserted = true;
        }
    }
    ui->sys_text->appendPlainText(QString("USB 0:\t[%1]") .arg(usb0Inserted ? "Plugged" : "------", 6));
    // USB1
    QDir usb1(USB1_DEVICE);
    if (usb1.exists())  {
        nFiles = usb1.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name).count();
        // Se nel Device Path esiste almeno un file OLTRE all'Hub, è presente un Device
        if (nFiles > 1)  {
            usb1Inserted = true;
        }
    }
    ui->sys_text->appendPlainText(QString("USB 1:\t[%1]") .arg(usb1Inserted ? "Plugged" : "------", 6));
    sdCardPresent = QFile::exists(SDCARD_DEVICE);
    ui->sys_text->appendPlainText(QString("SD Card:\t[%1]") .arg(sdCardPresent ? "Present" : "------", 6));
    ui->sys_text->moveCursor(QTextCursor::Start);

}

void Info::refreshNetworkingTabs()
{

    ui->dns_text->setPlainText("");
    ui->eth0_text->setPlainText("");
    ui->wlan0_text->setPlainText("");
    ui->ppp0_text->setPlainText("");
    ui->tun0_text->setPlainText("");

    FILE *fp = NULL;
    char buff[1024];

    // Reading DSN Info
    QFile file(DNS_FILE);
    if(file.exists()) {
        file.open(QIODevice::ReadOnly);
        QTextStream in(&file);
        // Cerca nel file le righe relative ai nameserver
        while(!in.atEnd()) {
            int nServer = 0;
            QString line = in.readLine().trimmed();
            if (! line.isEmpty())  {
                // dns server
                if (line.startsWith("nameserver"))  {
                    line.replace("nameserver:", "");
                    ui->dns_text->appendPlainText(
                        QString("DNS[%1]\t%2")
                            .arg(++nServer)
                            .arg(line.trimmed()));
                }
            }
        }
        file.close();
    }
    ui->dns_text->moveCursor(QTextCursor::Start);

    // Scanning Network interfaces
    // [eth0],[wlan0],[ppp0],[tun_mrs]: MACaddress and IPaddress(es)
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    QNetworkInterface iface;
    foreach(iface, allInterfaces) {
        QList<QNetworkAddressEntry> allEntries = iface.addressEntries();
        QPlainTextEdit *plainText = NULL;

        if (iface.name() == QString("eth0"))  {
            plainText = ui->eth0_text;
        } else if (iface.name() == QString("wlan0"))  {
            plainText = ui->wlan0_text;
        } else if (iface.name() == QString("ppp0"))  {
            plainText = ui->ppp0_text;
        } else if (iface.name() == QString("tun_mrs"))  {
            plainText = ui->tun0_text;
        }
        if (plainText) {
            // MAC 70:B3:D5:62:52:11
            // IP[1] 192.168.5.211/24
            // IP[2] 192.168.0.211/24
            // No MAC for tun0
            if (plainText != ui->tun0_text)  {
                plainText->appendPlainText("MAC:\t" + iface.hardwareAddress());
                plainText->appendPlainText("");
            }
            // Ip/prefix
            for (int n = 0; n < allEntries.count(); ++n) {
                plainText->appendPlainText(
                    QString("IP[%1]:\t%2/%3")
                    .arg(n + 1)
                    .arg(QString(allEntries[n].ip().toString()))
                    .arg(QString("%1").arg(allEntries[n].prefixLength()))
                );
            }
            plainText->appendPlainText("");
        }
    }

    // Reading Routing Tables
/*
    $ cat /proc/net/route
    Iface   Destination     Gateway         Flags   RefCnt  Use     Metric  Mask            MTU     Window  IRTT
    tun_mrs 1500010A        00000000        0005    0       0       0       FFFFFFFF        0       0       0
    eth0    0005A8C0        00000000        0001    0       0       0       00FFFFFF        0       0       0
    tun_mrs 0000010A        1500010A        0003    0       0       0       0000FFFF        0       0       0
    lo      0000007F        00000000        0001    0       0       0       000000FF        0       0       0
    eth0    00000000        0A05A8C0        0003    0       0       0       00000000        0       0       0
    $ route -n
    Kernel IP routing table
    Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
    10.1.0.21       0.0.0.0         255.255.255.255 UH    0      0        0 tun_mrs
    192.168.5.0     0.0.0.0         255.255.255.0   U     0      0        0 eth0
    10.1.0.0        10.1.0.21       255.255.0.0     UG    0      0        0 tun_mrs
    127.0.0.0       0.0.0.0         255.0.0.0       U     0      0        0 lo
    0.0.0.0         192.168.5.10    0.0.0.0         UG    0      0        0 eth0
*/
    fp = fopen(ROUTING_FILE, "r");
    if (fp) {
        char        Iface[16];
        unsigned    Destination, Gateway;
        int         Flags, RefCnt, Use, Metric;
        unsigned    Mask;
        int         MTU, Window, IRTT;
        const char *fmt = "%16s %X %X %X %d %d %d %X %d %d %d";
        while (fgets(buff, 1023, fp)) {
            int num = sscanf(buff, fmt,
                Iface, &Destination, &Gateway, &Flags, &RefCnt, &Use, &Metric, &Mask, &MTU, &Window, &IRTT);
            if (num < 11) {
                continue;
            }
            // Dumping routing info
            QPlainTextEdit *plainText = NULL;
            if (strcmp(Iface, "eth0") == 0)  {
                plainText = ui->eth0_text;
            } else if (strcmp(Iface, "wlan0") == 0)  {
                plainText = ui->wlan0_text;
            } else if (strcmp(Iface, "ppp0") == 0)  {
                plainText = ui->ppp0_text;
            } else if (strcmp(Iface, "tun_mrs") == 0)  {
                plainText = ui->tun0_text;
            }
            if (plainText ) {
                // /usr/include/linux/route.h
#define	RTF_UP          0x0001		/* route usable		  	*/
#define	RTF_GATEWAY     0x0002		/* destination is a gateway	*/
#define	RTF_HOST        0x0004		/* host entry (net otherwise)	*/
#define RTF_REINSTATE	0x0008		/* reinstate route after tmout	*/
#define	RTF_DYNAMIC     0x0010		/* created dyn. (by redirect)	*/
#define	RTF_MODIFIED	0x0020		/* modified dyn. (by redirect)	*/
#define RTF_MTU         0x0040		/* specific MTU for this route	*/
#define RTF_WINDOW      0x0080		/* per route window clamping	*/
#define RTF_IRTT        0x0100		/* Initial round trip time	*/
#define RTF_REJECT      0x0200		/* Reject route			*/
                if (Flags & RTF_UP) { // no RTF_GATEWAY
                    if (Destination == 0 && Mask == 0) {
                        plainText->appendPlainText(
                            QString("default route via %1")
                            .arg(QHostAddress(ntohl(Gateway)).toString())
                        );
                    } else {
                        QNetworkAddressEntry x = QNetworkAddressEntry();
                        x.setIp(QHostAddress(ntohl(Destination)));
                        x.setNetmask(QHostAddress(ntohl(Mask)));
                        plainText->appendPlainText(
                            QString("route:\t%1/%2 via %3")
                            .arg(QHostAddress(ntohl(Destination)).toString())
                            .arg(x.prefixLength())
                            .arg(QHostAddress(ntohl(Gateway)).toString())
                        );
                    }
                }
            }
        }
        fclose(fp);
        fp = NULL;
    }
}

void Info::refreshNTPInfo()
{
    // Clear NPT Text
    ui->ntp_text->setPlainText("");
    // Refresh NTP Perams
    ui->ntp_text->appendPlainText(QString("NTP Server: \t\t%1") .arg(ntpClient->getNtpServer()));
    ui->ntp_text->appendPlainText(QString("NTP Offset: \t\t%1  [Hours]") .arg(ntpClient->getOffset_h(), 4, 10));
    ui->ntp_text->appendPlainText(QString("NTP TimeOut:\t%1 [Seconds]") .arg(ntpClient->getTimeout_s(), 4, 10));
    ui->ntp_text->appendPlainText(QString("NTP Period: \t\t%1  [Hours, 0=Sync disabled]") .arg(ntpClient->getPeriod_h(), 4, 10));
    ui->ntp_text->appendPlainText(QString("NTP DST:    \t\t%1") .arg(ntpClient->getDst() ? QString("ON") : QString("OFF")));
    ui->ntp_text->moveCursor(QTextCursor::Start);

}
