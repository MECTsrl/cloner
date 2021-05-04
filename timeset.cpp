#include "timeset.h"
#include "ui_timeset.h"
#include "timepopup.h"
#include "calendar.h"

#include "publics.h"


TimeSet::TimeSet(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimeSet)
{
    ui->setupUi(this);
    ui->lblModel->setText(szModel);

//    nOffset = ntpclient->getOffset_h();
//    isDst = ntpclient->getDst();
//    nTimeOut = ntpclient->getTimeout_s();
//    nPeriod = ntpclient->getPeriod_h();
//    szTimeServer = ntpclient->getNtpServer();
    startTimer(REFRESH_MS);
}

TimeSet::~TimeSet()
{
    delete ui;
}

void TimeSet::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    // Aggiornamento orologio
    QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->lblDateTime->setText(szDateTime);
}

void TimeSet::on_cmdBack_clicked()
{
    this->reject();
}
