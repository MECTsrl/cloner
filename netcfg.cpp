#include "netcfg.h"
#include "ui_netcfg.h"

#include "publics.h"
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <stdio.h>
#include <QTimer>


#define NONE     "-"
#define NONE_LEN 1
#define ZEROIP "0.0.0.0"
#define NET_CONF_FILE "/local/etc/sysconfig/net.conf"


NetCfg::NetCfg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetCfg)
{
    ui->setupUi(this);
    // ui->lblModel->setText(szModel);
    startTimer(REFRESH_MS);
    QTimer *refresh_timer = new QTimer(this);
    connect(refresh_timer, SIGNAL(timeout()), this, SLOT(updateData()));
    refresh_timer->start(REFRESH_MS);
    ui->comboBox_wlan0_essid->clear();
    ui->comboBox_wlan0_essid->addItem(NONE);
    ui->pushButton_hidden_wlan0->setText(NONE);
    ui->checkBox_hiddenESSID->setChecked(false);
    ui->pushButton_hidden_wlan0->setVisible(false);
    is_eth0_enabled = (system("grep -c INTERFACE0 /etc/rc.d/rc.conf >/dev/null 2>&1") == 0);
    ui->tab_eth0->setEnabled(is_eth0_enabled);
    if (!is_eth0_enabled)
    {
        ui->tabWidget->removeTab(0);
    }
    ui->checkBox_wlan0_DHCP->setChecked(true);
    wlan0_essid = "";
    wlan0_pwd = "";
    loadETH0cfg();
    loadWAN0cfg();
    loadWLAN0cfg();

}

NetCfg::~NetCfg()
{
    delete ui;
}

void NetCfg::updateData()
{
    if (isWanOn()) {
        ui->label_wan_connect->setText("Disconnect");
        ui->pushButton_wan0_enable->setIcon(QIcon(QPixmap(":/libicons/img/disconnect.png")));
        ui->label_wan0_IP->setText(getIPAddr("ppp0"));
    } else {
        ui->label_wan_connect->setText("Connect");
        ui->pushButton_wan0_enable->setIcon(QIcon(QPixmap(":/libicons/img/4g_connect.png")));
        ui->label_wan0_IP->setText(NONE);

    }
    if (isWlanOn()) {
        ui->label_Wlan_connect->setText("Disconnect");
        ui->pushButton_wlan0_IP->setText(getIPAddr("wlan0"));
        ui->pushButton_wlan0_enable->setIcon(QIcon(QPixmap(":/libicons/img/disconnect.png")));
    } else {
        ui->label_Wlan_connect->setText("Connect");
        ui->pushButton_wlan0_enable->setIcon(QIcon(QPixmap(":/libicons/img/wifi_connect.png")));
        ui->pushButton_wlan0_IP->setText(NONE);
    }
}


bool NetCfg::netcfg_ini_set(QString setting, QString value, QString file)
{
    FILE *fp;
    bool fRes;
    QString command;
    command ="grep -q \"^" + setting + "=\" "+ file + " && sed -i 's/.*" + setting + "=.*/" + setting + "=" + value + "/g' " + file + " ||  sed -i \"$ a\""+ setting + "="+ value +" "+file;
    fp = popen(command.toLatin1().data(),"r");
    if (fp == NULL) {
        fRes = false;
        QMessageBox::critical(0,"Network configuration", "Problem during writing the network settings" + setting);
    } else {
        fRes = true;
    }
    pclose(fp);
    return fRes;
}

QString NetCfg::netcfg_ini_get(QString setting, QString file)
{
    QString command;
    QString resultValue;
    command = "fgrep -r \""+setting+"\" " + file;

    QString readResult;
    QProcess readSettings;
    readSettings.start(command);
    readSettings.waitForFinished();

    if (readSettings.exitCode() != 0) {
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Problem during reading the network settings"));
        resultValue = "";

    } else {
        readResult = QString(readSettings.readAll());
        if(readResult.isEmpty()) {
            resultValue = "";
        } else {
            QStringList results = readResult.split("\n");

            if (results.at(0).isEmpty()) {
                resultValue = "";
            } else {
                resultValue = results.at(0);
                resultValue = resultValue.split("=").at(1);
                if(resultValue.isEmpty()) {
                    resultValue = "";
                }
            }
        }
    }
    return resultValue;
}

bool NetCfg::checkNetAddr(char * ipaddr)
{
    QStringList ipaddrStr = QString(ipaddr).split(".");

    if (ipaddrStr.count() != 4)
    {
        return false;
    }

    bool ok;
    for (int i = 0; i < 4; i++)
    {
        int a = ipaddrStr.at(i).toInt(&ok);
        if (ok == false || a < 0 || a > 255)
        {
            return false;
        }
    }
    return true;
}

void NetCfg::timerEvent(QTimerEvent *event)
{
    // Aggiornamento orologio
    // QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    // ui->lblDateTime->setText(szDateTime);
}

void NetCfg::on_cmdCancel_clicked()
{
    this->reject();
}

void NetCfg::on_cmdOk_clicked()
{
    if (saveETH0cfg() && saveWLAN0cfg() && saveWAN0cfg()) {
        QMessageBox::information(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("The new configuration is saved and active."));
    }
    this->accept();
}

QString NetCfg::getMacAddr(QString interface)
{
    QString resultValue;

    QString readResult;
    QProcess readSettings;
    readSettings.start("/bin/sh", QStringList() << "-c" << " ifconfig | grep " + interface);
    readSettings.waitForFinished();

    if (readSettings.exitCode() != 0) {
        resultValue = NONE;
    } else {
        readResult = QString(readSettings.readAll());
        if(readResult.isEmpty()) {
            resultValue = NONE;
        } else {
            QStringList results = readResult.split(" ");
            for (int i = 0;i < results.count();i++) {
                if(results.at(i) == "HWaddr") {
                    resultValue = results.at(i+1);
                    break;
                }
            }
            if (resultValue.isEmpty()) {
                resultValue = NONE;
            }
        }
    }
    return resultValue;
}

QString NetCfg::getIPAddr(QString interface)
{
    QString resultValue;

    QString readResult;
    QProcess readSettings;
    readSettings.start("/bin/sh", QStringList() << "-c" << " ip a | grep " + interface);
    readSettings.waitForFinished();

    if (readSettings.exitCode() != 0) {
        resultValue = NONE;
    } else {
        readResult = QString(readSettings.readAll());
        if(!readResult.isEmpty()) {
            QStringList results = readResult.split(" ");
            if (!results.isEmpty()) {
                for (int i=0; i< results.count(); i++) {
                    if (results.at(i) == "inet") {
                        if (!results.at(i+1).isEmpty()) {
                            resultValue = results.at(i+1);
                            break;
                        }
                    }
                }
            }
        }
    }
    if (resultValue.isEmpty()) {
        resultValue = NONE;
    }
    return resultValue;
}



//////////////////////
///     Mobile     ///
//////////////////////
bool NetCfg::isWanOn()
{
    bool fRes = false;
    QProcess readSettings;
    readSettings.start("/bin/sh", QStringList() << "-c" << "ip a | grep ppp0");
    readSettings.waitForFinished();

    if (readSettings.exitCode() != 0) {
        fRes = false;
    } else {
        QString readResult = QString(readSettings.readAll());
        if (readResult.contains("DOWN")) {
            fRes = false;
        } else {
            fRes = true;
        }
    }
    // True = connected  || False = not connected
    return fRes;
}

void NetCfg::on_pushButton_wan0_dialnb_clicked()
{
    char value [32];
    nk = new numpad(value, DIALNB, ui->pushButton_wan0_dialnb->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted)
    {
        ui->pushButton_wan0_dialnb->setText(value);
    }
}

void NetCfg::on_pushButton_wan0_apn_clicked()
{
    char value [32];
    dk = new alphanumpad(value, ui->pushButton_wan0_apn->text().toAscii().data());
    dk->showFullScreen();
    if(dk->exec() == QDialog::Accepted)
    {
        ui->pushButton_wan0_apn->setText(value);
    }
}

void NetCfg::on_pushButton_wan0_DNS1_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_wan0_DNS1->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wan0_DNS1->setText(value);
    }
}

void NetCfg::on_pushButton_wan0_DNS2_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_wan0_DNS2->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wan0_DNS2->setText(value);
    }
}

bool NetCfg::checkUSBwanKey()
{
    return system("lsmod | grep -q ^usb_wwan && test -e /dev/ttyUSB0") == 0;
}

bool NetCfg::checkUSBwlanKey()
{
    return system("ifconfig wlan0 >/dev/null 2>&1") == 0;
}

void NetCfg::on_pushButton_wan0_enable_clicked()
{
    QString icon;
    setEnableWidgets(false);
    ui->tab_wan0->repaint();
    // Mobile Current Cfg Update forced

    if (!isWanOn())
    {
        saveWAN0cfg();
        if (!netcfg_ini_set("ONBOOTP0","1",NET_CONF_FILE))
        {
            /* error */
            ui->tab_wan0->setEnabled(true);
            ui->tab_wan0->repaint();
            return;
        }
        system("/usr/sbin/usb3g.sh start >/dev/null 2>&1 &");
        ui->label_wan_connect->setText("Disconnect");
        icon = ":/libicons/img/disconnect.png";
    }
    else
    {
        if (!netcfg_ini_set("ONBOOTP0","0",NET_CONF_FILE))
        {
            /* error */
            ui->tab_wan0->setEnabled(true);
            ui->tab_wan0->repaint();
            return;
        }
        system("/usr/sbin/usb3g.sh stop >/dev/null 2>&1 &");
        ui->label_wan_connect->setText("Connect");
        icon = ":/libicons/img/4g_connect.png";
    }
    ui->pushButton_wan0_enable->setIcon(QIcon(QPixmap(icon)));
    setEnableWidgets(true);
    ui->tab_wan0->repaint();
}

bool NetCfg::saveWAN0cfg()
{
    /* DIALNB */
    if (!ui->pushButton_wan0_dialnb->text().isEmpty())
    {
        netcfg_ini_set("DIALNBP0", ui->pushButton_wan0_dialnb->text(),NET_CONF_FILE);
    }
    /* APN */
    if (!ui->pushButton_wan0_apn->text().isEmpty())
    {
        netcfg_ini_set( "APNP0", ui->pushButton_wan0_apn->text(),NET_CONF_FILE);
    }
    /* DNS1 */
    if (!ui->pushButton_wan0_DNS1->text().isEmpty())
    {
        netcfg_ini_set("NAMESERVERP01", ui->pushButton_wan0_DNS1->text(),NET_CONF_FILE);
    }
    /* DNS2 */
    if (!ui->pushButton_wan0_DNS2->text().isEmpty())
    {
        netcfg_ini_set("NAMESERVERP02", ui->pushButton_wan0_DNS2->text(),NET_CONF_FILE);
    }
    char command[256];
    system("/usr/sbin/usb3g.sh stop"); // do wait
    sprintf(command, "/usr/sbin/usb3g.sh setup \"%s\" \"%s\" >/dev/null 2>&1 ",
            ui->pushButton_wan0_dialnb->text().toAscii().data(),
            ui->pushButton_wan0_apn->text().toAscii().data()
            );
    system(command);
//    if (system(command))
//    {
//        /* error */
//        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot setup the ppp network configuration for '%1'").arg(ui->pushButton_wan0_dialnb->text()));
//        return false;
//    }
    return true;
}

void NetCfg::loadWAN0cfg()
{
    /* WAN0 */
    /* DIALNB */
    QString DIALNBP0 = netcfg_ini_get("DIALNBP0",NET_CONF_FILE);
    if(!DIALNBP0.isEmpty()) {

        ui->pushButton_wan0_dialnb->setText(DIALNBP0.remove("\""));
    } else {
        ui->pushButton_wan0_dialnb->setText(NONE);
    }

    /* APN */
    QString APN = netcfg_ini_get("APNP0",NET_CONF_FILE);
    if (!APN.isEmpty()) {
        ui->pushButton_wan0_apn->setText(APN.remove("\""));
    } else {
        ui->pushButton_wan0_apn->setText(NONE);
    }
    /* DNS1 */
    QString DNS1 = netcfg_ini_get("NAMESERVERP01",NET_CONF_FILE);
    if (!DNS1.isEmpty()) {
        ui->pushButton_wan0_DNS1->setText(DNS1.remove("\""));
    } else {
        ui->pushButton_wan0_DNS1->setText(NONE);
    }
    /* DNS2 */
    QString DNS2 = netcfg_ini_get("NAMESERVERP02",NET_CONF_FILE);
    if (!DNS2.isEmpty()) {
        ui->pushButton_wan0_DNS1->setText(DNS2.remove("\""));
    } else {
        ui->pushButton_wan0_DNS1->setText(NONE);
    }
}

//////////////////////
///      WIFI      ///
//////////////////////

void NetCfg::on_pushButton_hidden_wlan0_clicked()
{
    char value [64];
    QString currentText = ui->pushButton_hidden_wlan0->text();
    if(currentText.isEmpty()) {
       currentText = NONE;
    }
    dk = new alphanumpad(value, currentText.toAscii().data());
    dk->showFullScreen();
    if(dk->exec() == QDialog::Accepted)
    {
        QString szValue = QString(value).trimmed();
        ui->pushButton_hidden_wlan0->setText(szValue);
        wlan0_essid = szValue;
    }
}

void NetCfg::on_checkBox_hiddenESSID_toggled(bool checked)
{
    if (checked)  {
        ui->comboBox_wlan0_essid->setEnabled(false);
        ui->pushButton_hidden_wlan0->setVisible(true);
        ui->pushButton_wlan0_scan->setEnabled(false);
        wlan0_essid = ui->pushButton_hidden_wlan0->text();
    }
    else  {
        ui->comboBox_wlan0_essid->setEnabled(true);
        ui->pushButton_hidden_wlan0->setVisible(false);
        ui->pushButton_wlan0_scan->setEnabled(true);
        wlan0_essid = ui->comboBox_wlan0_essid->currentText();
    }
}

void NetCfg::on_pushButton_wlan0_IP_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_wlan0_IP->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_IP->setText(value);
    }
}

void NetCfg::on_pushButton_wlan0_NM_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_wlan0_NM->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_NM->setText(value);
    }
}

void NetCfg::on_pushButton_wlan0_GW_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_wlan0_GW->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_GW->setText(value);
    }
}

void NetCfg::on_pushButton_wlan0_DNS1_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_wlan0_DNS1->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_DNS1->setText(value);
    }
}

void NetCfg::on_pushButton_wlan0_DNS2_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_wlan0_DNS2->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_DNS2->setText(value);
    }
}

void NetCfg::on_pushButton_wlan0_scan_clicked()
{
    ui->comboBox_wlan0_essid->clear();
    ui->comboBox_wlan0_essid->addItem("...Scanning...");
    setEnableWidgets(false);


    QString wifiScanResult;
    QProcess wifiScan;
    QCoreApplication::processEvents();
    wifiScan.start("/usr/sbin/wifi.sh scan");
    wifiScan.waitForFinished();

    ui->comboBox_wlan0_essid->clear();
    if (wifiScan.exitCode() != 0) {
        ui->comboBox_wlan0_essid->addItem(NONE);
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Problem during wifi network scanning"));
        return;

    } else {
        wifiScanResult = QString(wifiScan.readAll());

        if (!wifiScanResult.isEmpty()) {            
            QStringList wifiList = wifiScanResult.split('\n');
            foreach (QString wifiName, wifiList) {

                wifiName = wifiName.split('\t').at(0);

                if(!wifiName.isEmpty()) {
                    ui->comboBox_wlan0_essid->addItem(wifiName);
                }
            }

            int index = ui->comboBox_wlan0_essid->findText(wlan0_essid);
            if (index < 0){
                ui->comboBox_wlan0_essid->addItem(wlan0_essid);
                index = ui->comboBox_wlan0_essid->findText(wlan0_essid);
            }

            if (index != ui->comboBox_wlan0_essid->currentIndex()){
                ui->comboBox_wlan0_essid->setCurrentIndex(index);
            }
        } else {
            ui->comboBox_wlan0_essid->addItem(NONE);
        }
    }
    setEnableWidgets(true);
}

bool NetCfg::isWlanOn(void)
{

    bool fRes = false;
    QProcess readSettings;
    readSettings.start("/bin/sh", QStringList() << "-c" << "iwconfig wlan0 | grep 'Access Point: Not-Associate'");
    readSettings.waitForFinished();

    if (readSettings.exitCode() != 0) {
        fRes = false;
    } else {
        QString readResult = QString(readSettings.readAll());
        if (readResult.contains("Not-Associated")) {
            fRes = false;
        } else {
            fRes = true;
        }
     }
    // True = connected  || False = not connected
   return fRes;
}

bool NetCfg::saveWLAN0cfg()
{
    if(!wlan0_essid.isEmpty()) {
        netcfg_ini_set("ESSIDW0",wlan0_essid,NET_CONF_FILE);
    }
    if(!wlan0_pwd.isEmpty()) {
        netcfg_ini_set("PASSWORDW0",wlan0_pwd,NET_CONF_FILE);
    } else {
        netcfg_ini_set("PASSWORDW0","",NET_CONF_FILE);
    }

    if (ui->checkBox_wlan0_DHCP->isChecked()){
        netcfg_ini_set("BOOTPROTOW0","[DHCP]",NET_CONF_FILE);
    }
    else {
        netcfg_ini_set("BOOTPROTOW0","[none]",NET_CONF_FILE);

        if(!ui->pushButton_wlan0_IP->text().isEmpty()) {
            netcfg_ini_set("IPADDRW0",ui->pushButton_wlan0_IP->text(),NET_CONF_FILE);
        }

        if (!ui->pushButton_wlan0_GW->text().isEmpty()) {
            QString  szGW =ui->pushButton_wlan0_GW->text();
            if (szGW == ZEROIP) {
                netcfg_ini_set("GATEWAYW0","",NET_CONF_FILE);
            } else {
                netcfg_ini_set("GATEWAYW0",szGW,NET_CONF_FILE);
            }
        }

        if (!ui->pushButton_wlan0_NM->text().isEmpty()) {
            netcfg_ini_set("NETMASKW0",ui->pushButton_wlan0_NM->text(),NET_CONF_FILE);
        }

        if(!ui->pushButton_wlan0_DNS1->text().isEmpty()) {
            netcfg_ini_set("NAMESERVERW01",ui->pushButton_wlan0_DNS1->text(),NET_CONF_FILE);
        }

        if(!ui->pushButton_wlan0_DNS2->text().isEmpty()) {
            netcfg_ini_set("NAMESERVERW02",ui->pushButton_wlan0_DNS2->text(),NET_CONF_FILE);
        }
    }

    QString command;
    system("/usr/sbin/wifi.sh stop"); // do wait
    command = "/usr/sbin/wifi.sh setup \""+wlan0_essid.toAscii()+"\" \""+wlan0_pwd.toAscii()+"\" >/dev/null 2>&1 &";
    system(command.toLatin1().data());
    //    if (system(command))
    //    {
    //        /* error */
    //        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot setup the wifi network configuration for '%1'").arg(wlan0_essid));
    //        return false;
    //    }
    return true;
}

void NetCfg::loadWLAN0cfg()
{

    QString ESSIDW0 = netcfg_ini_get("ESSIDW0",NET_CONF_FILE);
    if (!ESSIDW0.isEmpty()) {
        int index = ui->comboBox_wlan0_essid->findText(wlan0_essid);
        if (index  <  0) {
            ui->comboBox_wlan0_essid->addItem(ESSIDW0.remove("\""));
             ui->comboBox_wlan0_essid->setCurrentIndex(1);
        } else {
            ui->comboBox_wlan0_essid->setCurrentIndex(index);
        }
        ui->pushButton_hidden_wlan0->setText(ESSIDW0.remove("\""));
    } else {
        ui->pushButton_hidden_wlan0->setText(NONE);
    }
    /* DHCP */
    QString BOOTPROTOW0 = netcfg_ini_get("BOOTPROTOW0",NET_CONF_FILE);
    if (!BOOTPROTOW0.isEmpty() && BOOTPROTOW0 == "[DHCP]")
    {
        ui->checkBox_wlan0_DHCP->setChecked(true);
        on_checkBox_wlan0_DHCP_clicked(true);
    }
    else
    {
        ui->checkBox_wlan0_DHCP->setChecked(false);
        on_checkBox_wlan0_DHCP_clicked(false);
    }
    /* IP */
    QString IPADDRW0 = netcfg_ini_get("IPADDRW0",NET_CONF_FILE);
    if (!IPADDRW0.isEmpty()) {
        ui->pushButton_wlan0_IP->setText(IPADDRW0.remove("\""));
    } else {
        ui->pushButton_wlan0_IP->setText(NONE);
    }

    /* GATEWAY */
    QString GATEWAYW0 = netcfg_ini_get("GATEWAYW0",NET_CONF_FILE);
    if (!GATEWAYW0.isEmpty()) {
        ui->pushButton_wlan0_GW->setText(GATEWAYW0.remove("\""));
    } else {
        ui->pushButton_wlan0_GW->setText(NONE);
    }
    /* NETMASK */
    QString NETMASKW0 = netcfg_ini_get("NETMASKW0",NET_CONF_FILE);
    if (!NETMASKW0.isEmpty()) {
        ui->pushButton_wlan0_NM->setText(NETMASKW0.remove("\""));
    } else {
        ui->pushButton_wlan0_NM->setText(NONE);
    }
    /* DNS1 */
    QString NAMESERVERW01 = netcfg_ini_get("NAMESERVERW01",NET_CONF_FILE);
    if (!NAMESERVERW01.isEmpty()) {
        ui->pushButton_wlan0_DNS1->setText(NAMESERVERW01.remove("\""));
    } else {
        ui->pushButton_wlan0_DNS1->setText(NONE);
    }
    /* DNS2 */
    QString NAMESERVERW02 = netcfg_ini_get("NAMESERVERW02",NET_CONF_FILE);
    if (!NAMESERVERW02.isEmpty()) {
        ui->pushButton_wlan0_DNS2->setText(NAMESERVERW02.remove("\""));
    } else {
        ui->pushButton_wlan0_DNS2->setText(NONE);
    }
    /* MAC */
    QString MACW0 = getMacAddr("wlan0");
    if (MACW0.isEmpty()) {
        ui->label_wlan0_MAC->setText(MACW0.remove("\""));
    } else {
        ui->label_wlan0_MAC->setText(NONE);
    }
}

void NetCfg::on_pushButton_wlan0_enable_clicked()
{
    QString icon;
    setEnableWidgets(false);

    ui->tab_wlan0->repaint();
    // WiFi Current Cfg Update forced
    if (!isWlanOn()) {
        saveWLAN0cfg();
        if (!netcfg_ini_set("ONBOOTW0","1",NET_CONF_FILE)){
            /* error */
            ui->tab_wlan0->setEnabled(true);
            ui->tab_wlan0->repaint();
            return;
        }
        system("/usr/sbin/wifi.sh start >/dev/null 2>&1 &");
        ui->label_Wlan_connect->setText("Disconnect");
        icon = ":/libicons/img/disconnect.png";
    } else {
        if (!netcfg_ini_set("ONBOOTW0","0",NET_CONF_FILE)) {
            /* error */
            ui->tab_wlan0->setEnabled(true);
            ui->tab_wlan0->repaint();
            return;
        }
        system("/usr/sbin/wifi.sh stop >/dev/null 2>&1 &");
        ui->label_Wlan_connect->setText("Connect");
        icon = ":/libicons/img/wifi_connect.png";
    }
    ui->pushButton_wlan0_IP->setText(getIPAddr(getIPAddr("wlan0")));
    ui->pushButton_wlan0_enable->setIcon(QIcon(QPixmap(icon)));
    setEnableWidgets(true);
    ui->tab_wlan0->repaint();
}

void NetCfg::on_pushButton_wlan0_pwd_clicked()
{
    char value [32];
    dk = new alphanumpad(value, wlan0_pwd.toAscii().data(), true);
    dk->showFullScreen();
    if(dk->exec()==QDialog::Accepted)
    {
        wlan0_pwd = value;
        ui->pushButton_wlan0_pwd->setText(QString("*").repeated(wlan0_pwd.length()));
    }
}

void NetCfg::on_checkBox_wlan0_DHCP_clicked(bool checked)
{
    ui->frame_wlan0->setEnabled(!checked);
}

void NetCfg::on_comboBox_wlan0_essid_currentIndexChanged(const QString &arg1)
{
    wlan0_essid = arg1;
}



//////////////////////
///       ETH      ///
//////////////////////

void NetCfg::on_pushButton_eth0_IP_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_eth0_IP->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_IP->setText(value);
    }
}

void NetCfg::on_pushButton_eth0_NM_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_eth0_NM->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_NM->setText(value);
    }
}

void NetCfg::on_pushButton_eth0_GW_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_eth0_GW->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_GW->setText(value);
    }
}

void NetCfg::on_pushButton_eth0_DNS1_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_eth0_DNS1->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_DNS1->setText(value);
    }
}

void NetCfg::on_pushButton_eth0_DNS2_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_eth0_DNS2->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_DNS2->setText(value);
    }
}

void NetCfg::on_checkBox_eth0_DHCP_clicked(bool checked)
{
    ui->frame_eth0->setEnabled(!checked);
}


bool NetCfg::saveETH0cfg()
{
    if (!is_eth0_enabled)
    {
        return true;
    }

    /* DHCP */
    if (ui->checkBox_eth0_DHCP->isChecked()) {
        if (!netcfg_ini_set("BOOTPROTO0","[DHCP]",NET_CONF_FILE)) {
            /* error */
            return false;
        }
    } else {
        if (!netcfg_ini_set("BOOTPROTO0","[none]",NET_CONF_FILE)){
            /* error */
            return false;
        }
        /* IP */
        if (ui->pushButton_eth0_IP->text() != NONE && !netcfg_ini_set("IPADDR0", ui->pushButton_eth0_IP->text(),NET_CONF_FILE)){
            /* error */
            return false;
        }
        /* GATEWAY */
        if (ui->pushButton_eth0_GW->text() != NONE){
            QString     szGW = ui->pushButton_eth0_GW->text();
            if (szGW == ZEROIP)
                szGW = "";
            if (!netcfg_ini_set("GATEWAY0",szGW,NET_CONF_FILE)){
                /* error */
                return false;
            }
        }
        /* NETMASK */
        if (ui->pushButton_eth0_NM->text() != NONE && !netcfg_ini_set("NETMASK0",ui->pushButton_eth0_NM->text(),NET_CONF_FILE)){
            /* error */
            return false;
        }
        /* DNS1 */
        if (ui->pushButton_eth0_DNS1->text() != NONE && !netcfg_ini_set("NAMESERVER01",ui->pushButton_eth0_DNS1->text(),NET_CONF_FILE)){
            /* error */
            return false;
        }
        /* DNS2 */
        if (ui->pushButton_eth0_DNS2->text() != NONE && !netcfg_ini_set("NAMESERVER02", ui->pushButton_eth0_DNS2->text(),NET_CONF_FILE)){
            /* error */
            return false;
        }
    }
    if (system("/etc/rc.d/init.d/network restart >/dev/null 2>&1"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot setup the eth0 network configuration."));
        return false;
    }
    return true;
}

void NetCfg::loadETH0cfg()
{
    if (is_eth0_enabled)
    {
        /* ETH0 */
        /* DHCP */
        QString DHCP = netcfg_ini_get("BOOTPROTO0",NET_CONF_FILE);
        if(DHCP == "[DHCP]" ) {
            ui->checkBox_eth0_DHCP->setChecked(true);
            ui->frame_eth0->setEnabled(false);

        } else {
            ui->checkBox_eth0_DHCP->setChecked(false);
            ui->frame_eth0->setEnabled(true);
        }
        /* IP */
        QString IP = netcfg_ini_get("IPADDR0",NET_CONF_FILE);
        if (!IP.isEmpty()) {
            ui->pushButton_eth0_IP->setText(IP.remove("\""));
        } else {
            ui->pushButton_eth0_IP->setText(NONE);
        }
        /* GATEWAY */
        QString GATEWAY = netcfg_ini_get("GATEWAY0",NET_CONF_FILE);
        if (!GATEWAY.isEmpty()) {
            ui->pushButton_eth0_GW->setText(GATEWAY.remove("\""));
        } else {
            ui->pushButton_eth0_GW->setText(NONE);
        }
        /* NETMASK */
        QString NETMASK = netcfg_ini_get("NETMASK0",NET_CONF_FILE);
        if (!NETMASK.isEmpty()) {
            ui->pushButton_eth0_NM->setText(NETMASK.remove("\""));
        } else {
            ui->pushButton_eth0_NM->setText(NONE);
        }
        /* DNS1 */
        QString DNS1 = netcfg_ini_get("NAMESERVER01",NET_CONF_FILE);
        if (!DNS1.isEmpty()) {
            ui->pushButton_eth0_DNS1->setText(DNS1.remove("\""));
        } else {
            ui->pushButton_eth0_DNS1->setText(NONE);
        }
        /* DNS2 */
        QString DNS2 = netcfg_ini_get("NAMESERVER02",NET_CONF_FILE);
        if (!DNS2.isEmpty()) {
            ui->pushButton_eth0_DNS2->setText(DNS2.remove("\""));
        } else {
            ui->pushButton_eth0_DNS2->setText(NONE);
        }
        /* MAC */
        ui->label_eth0_MAC->setText(getMacAddr("eth0").remove("\""));

        if (ui->checkBox_eth0_DHCP->isChecked()) {
            QString IP = getIPAddr("eth0");
            if (!IP.isEmpty()) {
                ui->pushButton_eth0_IP->setText(IP);
            } else {
                ui->pushButton_eth0_IP->setText(NONE);
            }
        }
    }
}


void NetCfg::setEnableWidgets(bool enabled)
{
    ui->cmdOk->setEnabled(enabled);
    ui->cmdCancel->setEnabled(enabled);
    ui->tab_eth0->setEnabled(enabled);
    ui->tab_wlan0->setEnabled(enabled);
    ui->tab_wan0->setEnabled(enabled);
}
