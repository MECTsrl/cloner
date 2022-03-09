#include "info.h"
#include "ui_info.h"

#include "myntpclient.h"

#include "publics.h"

#include <QPlainTextEdit>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>


Info::Info(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Info)
{
    ui->setupUi(this);
    ui->lblAction->setText("Target Info");
    ui->lblModel->setText(szModel);
    refreshAllTabs();
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
    ui->sys_text->setPlainText("");
    ui->sys_text->appendPlainText(QString("Product Id:\t[ %1]") .arg(szModel));
    ui->sys_text->appendPlainText(QString("S/N:\t\t[%1]") .arg(szSerialNO));
    ui->sys_text->appendPlainText(QString("Release:\t[%1]") .arg(szTargetVersion));
    ui->sys_text->appendPlainText(QString("Qt:\t\t[%1]") .arg(szQtVersion));
    ui->sys_text->moveCursor(QTextCursor::Start);

}

void Info::refreshNetworkingTabs()
{

}

void Info::refreshNTPInfo()
{
    // Clear NPT Text
    ui->ntp_text->setPlainText("");
    // Refresh NTP Perams
    ui->ntp_text->appendPlainText(QString("NTP Server: \t%1") .arg(ntpClient->getNtpServer()));
    ui->ntp_text->appendPlainText(QString("NTP Offset: \t%1 [Hours]") .arg(ntpClient->getOffset_h(), 4, 10));
    ui->ntp_text->appendPlainText(QString("NTP TimeOut:\t%1 [Seconds]") .arg(ntpClient->getTimeout_s(), 4, 10));
    ui->ntp_text->appendPlainText(QString("NTP Period: \t%1 [Hours, 0=Sync disabled]") .arg(ntpClient->getPeriod_h(), 4, 10));
    ui->ntp_text->appendPlainText(QString("NTP DST:    \t%1") .arg(ntpClient->getDst() ? QString("ON") : QString("OFF")));
    ui->ntp_text->moveCursor(QTextCursor::Start);

}
