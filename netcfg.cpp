#include "netcfg.h"
#include "ui_netcfg.h"

#include "publics.h"


NetCfg::NetCfg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetCfg)
{
    ui->setupUi(this);
    ui->lblModel->setText(szModel);
    startTimer(REFRESH_MS);
}

NetCfg::~NetCfg()
{
    delete ui;
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
