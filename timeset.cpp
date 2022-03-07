#include "timeset.h"
#include "ui_timeset.h"

#include "ntpclient.h"
#include "timepopup.h"
#include "calendar.h"

#include "publics.h"

#include <QMessageBox>


#define TIME_MASK "HH:mm:ss"
#define DATE_MASK "yyyy-MM-dd"



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

void TimeSet::updateIface()
{

}

void TimeSet::lockUI(bool setLocked)
{

}

void TimeSet::ntpManualSetDone(bool setOk)
{
    QObject::disconnect(ntpclient, 0, 0, 0);
    if (setOk)  {
        QMessageBox::information(this,trUtf8("Manual Date Time Set"), trUtf8("Current Date and Time set to:\n%1") .arg(datetimeTarget.toString(DATE_MASK" "TIME_MASK)));
    }
    else {
        QMessageBox::critical(this,trUtf8("Manual Date Time Set"), trUtf8("Error setting Date and Time to:\n%1") .arg(datetimeTarget.toString(DATE_MASK" "TIME_MASK)));
    }
    lockInterface = false;
    lockUI(lockInterface);
}

void TimeSet::ntpSyncDone(bool timeOut)
{
    QObject::disconnect(ntpclient, 0, 0, 0);
    ntpSyncRunning = false;
    if (timeOut)  {
        QMessageBox::warning(this,trUtf8("NTP Time Error"), trUtf8("Time Out syncing Date and Time with NTP Server:\n%1") .arg(szTimeServer));
    }
    else  {
        QMessageBox::information(this,trUtf8("NTP Time Set"), trUtf8("Current Date and Time set from NTP Server:\n%1") .arg(szTimeServer));
    }
    updateIface();
    lockInterface = false;
    lockUI(lockInterface);
}
