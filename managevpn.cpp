#include "managevpn.h"
#include "ui_managevpn.h"

#include "publics.h"

ManageVPN::ManageVPN(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManageVPN)
{
    ui->setupUi(this);
    ui->lblModel->setText(szModel);
    startTimer(REFRESH_MS);
    userAction = OVPN_NO_ACTION;
}

ManageVPN::~ManageVPN()
{
    delete ui;
}

void ManageVPN::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    // Aggiornamento orologio
    QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->lblDateTime->setText(szDateTime);
}

void ManageVPN::on_cmdCancel_clicked()
{
    this->reject();
}

void ManageVPN::on_cmdOk_clicked()
{
    this->accept();
}

int     ManageVPN::getSelectedAction()
// Return user selected Action
{
    return userAction;
}

