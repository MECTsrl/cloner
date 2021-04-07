#include "netcfg.h"
#include "ui_netcfg.h"

#include "publics.h"
#include <QProcess>
#include <QDebug>
#include <QFile>


#define NONE     "-"
#define NONE_LEN 1
#define ZEROIP "0.0.0.0"
#define NET_CONF_FILE "/local/etc/sysconfig/net.conf"


NetCfg::NetCfg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetCfg)
{
    ui->setupUi(this);
    ui->lblModel->setText(szModel);
    startTimer(REFRESH_MS);

    ui->comboBox_wlan0_essid->clear();
    ui->comboBox_wlan0_essid->addItem(NONE);
    ui->pushButton_hidden_wlan0->setText(NONE);
    ui->checkBox_hiddenESSID->setChecked(false);
    ui->pushButton_hidden_wlan0->setVisible(false);
    is_eth0_enabled = (system("grep -c INTERFACE0 /etc/rc.d/rc.conf >/dev/null 2>&1") == 0);
    is_eth1_enabled = (system("grep -c INTERFACE1 /etc/rc.d/rc.conf >/dev/null 2>&1") == 0);
    ui->tab_eth0->setEnabled(is_eth0_enabled);
    ui->tab_eth1->setEnabled(is_eth1_enabled);
    if (!is_eth1_enabled)
    {
        ui->tabWidget->removeTab(1);
    }
    if (!is_eth0_enabled)
    {
        ui->tabWidget->removeTab(0);
    }
    wlan0_essid = "";
    wlan0_pwd = "";
}

NetCfg::~NetCfg()
{
    delete ui;
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
    QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->lblDateTime->setText(szDateTime);
}

void NetCfg::on_cmdCancel_clicked()
{
    this->reject();
}

//////////////////////
///     Mobile     ///
//////////////////////
bool NetCfg::isWanOn()
{
    return system("test -e /var/pppd/up.stat && source /var/pppd/up.stat && test -e /proc/$PPPD_PID") == 0;
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

void NetCfg::on_pushButton_eth1_IP_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_eth1_IP->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_IP->setText(value);
    }
}

void NetCfg::on_pushButton_eth1_NM_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_eth1_NM->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_NM->setText(value);
    }
}

void NetCfg::on_pushButton_eth1_GW_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_eth1_GW->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_GW->setText(value);
    }
}

void NetCfg::on_pushButton_eth1_DNS1_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_eth1_DNS1->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_DNS1->setText(value);
    }
}

void NetCfg::on_pushButton_eth1_DNS2_clicked()
{
    char value [32];
    nk = new numpad(value, IPADDR, ui->pushButton_eth1_DNS2->text().toAscii().data());
    nk->showFullScreen();
    if(nk->exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_DNS2->setText(value);
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
    ui->comboBox_wlan0_essid->addItem(NONE);

    QString wifiScanResult;
    QProcess wifiScan;
    wifiScan.start("/usr/sbin/wifi.sh scan");
    wifiScan.waitForFinished();

    if (wifiScan.exitCode() != 0) {
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
        }
    }
}

bool NetCfg::isWlanOn(void)
{
    return system("iwconfig wlan0 2> /dev/null | grep -q 'Access Point: Not-Associate'");
}

/* WLAN0 */
bool NetCfg::saveWLAN0cfg()
{

//    QString command;
//            if(!wlan0_essid.isEmpty()) {
//                command ="sed -i 's/.*ESSIDW0=.*/ESSIDW0=\"Cioa\"/g'";
//                popen(command);
//            } else {
//                QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nESSIDW0"));
//            }

//            if(!wlan0_pwd.isEmpty()) {
//            command ="sed -i 's/.*ESSIDW0=.*/ESSIDW0=\"Cioa\"/g'";
//            popen(command);
//            } else {
//                line = "PASSWORDW0=\"\"";
//            }
//        }

//        if (ui->checkBox_wlan0_DHCP->isChecked()){
//            if (line.contains("BOOTPROTOW0")) {
//                line = "BOOTPROTOW0=\"[DHCP]\"";
//            }

//        }else {
//            if (line.contains("BOOTPROTOW0")) {
//                line = "BOOTPROTOW0=\"[none]\"";
//            }

//            if (line.contains("IPADDRW0")) {
//                if(!ui->pushButton_wlan0_IP->text().isEmpty()) {
//                    line = "IPADDRW0=\""+ui->pushButton_wlan0_IP->text()+"\"";
//                } else {
//                    QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nIPADDRW0"));
//                }
//            }

//            if (line.contains("GATEWAYW0")) {
//                if (!ui->pushButton_wlan0_GW->text().isEmpty()) {
//                    QString  szGW =ui->pushButton_wlan0_GW->text();
//                    if (szGW == ZEROIP)
//                        szGW = "";
//                    line = "GATEWAYW0=\""+szGW+"\"";
//                } else {
//                    QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nGATEWAYW0"));
//                }
//            }

//            if (line.contains("NETMASKW0")) {
//                if (!ui->pushButton_wlan0_NM->text().isEmpty()) {
//                    line = "NETMASKW0=\""+ui->pushButton_wlan0_NM->text()+"\"";
//                } else {
//                    QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNETMASKW0"));
//                }
//            }

//            if (line.contains("NAMESERVERW01")) {
//                if(!ui->pushButton_wlan0_DNS1->text().isEmpty()) {
//                    line = "NAMESERVERW01=\""+ui->pushButton_wlan0_DNS1->text()+"\"";
//                } else {
//                    QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVERW01"));
//                }
//            }

//            if (line.contains("NAMESERVERW02")) {
//                if(!ui->pushButton_wlan0_DNS2->text().isEmpty()) {
//                    line = "NAMESERVERW02=\""+ui->pushButton_wlan0_DNS2->text()+"\"";
//                } else {
//                    QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVERW02"));
//                }
//            }
//        }
//        netStreamtemp<<line<<"\n";
//    }

//    char command[256];
//    system("/usr/sbin/wifi.sh stop"); // do wait
//    sprintf(command, "/usr/sbin/wifi.sh setup \"%s\" \"%s\" >/dev/null 2>&1 &",
//            wlan0_essid.toAscii().data(),
//            wlan0_pwd.toAscii().data()
//            );
//    system(command);
//    //    if (system(command))
//    //    {
//    //        /* error */
//    //        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot setup the wifi network configuration for '%1'").arg(wlan0_essid));
//    //        return false;
//    //    }
//    return true;

}


void NetCfg::on_pushButton_wlan0_enable_clicked()
{
    ui->tab_wlan0->setEnabled(false);
    ui->tab_wlan0->repaint();
    // WiFi Current Cfg Update forced
   saveWLAN0cfg();
//    if (!is_wlan_active)
//    {
//        if (app_netconf_item_set("1", "ONBOOTW0"))
//        {
//            /* error */
//            ui->tab_wlan0->setEnabled(true);
//            ui->tab_wlan0->repaint();
//            return;
//        }
//        system("/usr/sbin/wifi.sh start >/dev/null 2>&1 &");
//    }
//    else
//    {
//        if (app_netconf_item_set("0", "ONBOOTW0"))
//        {
//            /* error */
//            ui->tab_wlan0->setEnabled(true);
//            ui->tab_wlan0->repaint();
//            return;
//        }
//        system("/usr/sbin/wifi.sh stop >/dev/null 2>&1 &");
//    }

    ui->tab_wlan0->setEnabled(true);
    ui->tab_wlan0->repaint();
//    updateIcons();
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

void NetCfg::on_checkBox_eth0_DHCP_clicked(bool checked)
{
    ui->frame_eth0->setEnabled(!checked);
}

void NetCfg::on_checkBox_eth1_DHCP_clicked(bool checked)
{
    ui->frame_eth1->setEnabled(!checked);
}

void NetCfg::on_checkBox_wlan0_DHCP_clicked(bool checked)
{
    ui->frame_wlan0->setEnabled(!checked);
}

void NetCfg::on_comboBox_wlan0_essid_currentIndexChanged(const QString &arg1)
{
    wlan0_essid = arg1;
}









