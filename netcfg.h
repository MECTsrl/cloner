#ifndef NETCFG_H
#define NETCFG_H

#include <QDialog>
#include "alphanumpad.h"
#include "numpad.h"


namespace Ui {
class NetCfg;
}

class NetCfg : public QDialog
{
    Q_OBJECT

public:
    explicit NetCfg(QWidget *parent = 0);
    ~NetCfg();

protected:
    void    timerEvent(QTimerEvent *event);

private slots:
    void on_cmdCancel_clicked();
    void updateData();


    ///MOBILE
    void on_pushButton_wan0_dialnb_clicked();
    void on_pushButton_wan0_apn_clicked();
    void on_pushButton_wan0_DNS1_clicked();
    void on_pushButton_wan0_DNS2_clicked();
    void on_pushButton_wan0_enable_clicked();

    ///WIFI
    void on_pushButton_wlan0_scan_clicked();
    void on_pushButton_wlan0_IP_clicked();
    void on_pushButton_wlan0_NM_clicked();
    void on_pushButton_wlan0_GW_clicked();
    void on_pushButton_wlan0_DNS1_clicked();
    void on_pushButton_wlan0_DNS2_clicked();
    void on_checkBox_hiddenESSID_toggled(bool checked);
    void on_pushButton_hidden_wlan0_clicked();
    void on_pushButton_wlan0_pwd_clicked();
    void on_checkBox_wlan0_DHCP_clicked(bool checked);
    void on_pushButton_wlan0_enable_clicked();
    void on_comboBox_wlan0_essid_currentIndexChanged(const QString &arg1);

    ///ETH

    void on_pushButton_eth0_DNS2_clicked();
    void on_pushButton_eth0_DNS1_clicked();
    void on_pushButton_eth0_GW_clicked();
    void on_pushButton_eth0_NM_clicked();
    void on_pushButton_eth0_IP_clicked();
    void on_checkBox_eth0_DHCP_clicked(bool checked);


    void on_cmdOk_clicked();

private:
    bool checkNetAddr(char * ipaddr);
    bool saveETH0cfg();
    bool saveWLAN0cfg();
    bool saveWAN0cfg();

    void loadETH0cfg();
    void loadWAN0cfg();
    void loadWLAN0cfg();

    bool netcfg_ini_set(QString setting, QString value, QString file);
    QString netcfg_ini_get(QString setting, QString file);
    QString getMacAddr(QString interface);
    QString getIPAddr(QString interface);
    void setEnableWidgets(bool enabled);



    bool isWlanOn(void);
    bool isWanOn(void);

    bool checkUSBwanKey();
    bool checkUSBwlanKey();

    void updateIcons();

private:
    Ui::NetCfg *ui;
    QString wlan0_pwd;
    QString wlan0_essid;
    bool is_eth0_enabled;
    bool is_wlan_active;
    bool is_wan_active;
    bool setup;
    alphanumpad * dk;
    numpad *nk;
};

#endif // NETCFG_H
