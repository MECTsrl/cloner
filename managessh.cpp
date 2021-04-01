#include "managessh.h"
#include "ui_managessh.h"

#include "publics.h"

ManageSSH::ManageSSH(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManageSSH)
{
    ui->setupUi(this);
    ui->lblModel->setText(szModel);
    startTimer(REFRESH_MS);
}

ManageSSH::~ManageSSH()
{
    delete ui;
}

void ManageSSH::timerEvent(QTimerEvent *event)
{
    // Aggiornamento orologio
    QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->lblDateTime->setText(szDateTime);
}

void ManageSSH::on_cmdCancel_clicked()
{
    this->reject();
}
