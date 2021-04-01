#include "info.h"
#include "ui_info.h"

#include "publics.h"

#include <QList>
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
    startTimer(REFRESH_MS);
}

Info::~Info()
{
    delete ui;
}

void Info::timerEvent(QTimerEvent *event)
{
    // Aggiornamento orologio
    QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->lblDateTime->setText(szDateTime);
}

void Info::on_cmdBack_clicked()
{
    this->reject();
}
