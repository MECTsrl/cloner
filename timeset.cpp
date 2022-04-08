#include "timeset.h"
#include "ui_timeset.h"
#include "publics.h"
#include "myntpclient.h"


#include "numpad.h"
#include "alphanumpad.h"

#include <sys/time.h>
#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QObject>



#define TIME_MASK "HH:mm:ss"
#define DATE_MASK "yyyy-MM-dd"



TimeSet::TimeSet(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimeSet)
{
    ui->setupUi(this);
    ui->lblModel->setText(szModel);
    ntpSyncRunning = false;
    // ntp Server Parameters
    nOffset = ntpClient->getOffset_h();
    isDst = ntpClient->getDst();
    nTimeOut = ntpClient->getTimeout_s();
    nPeriod = ntpClient->getPeriod_h();
    szTimeServer = ntpClient->getNtpServer();
    updateIface(true);
    ui->progressBarElapsed->setMinimum(0);
    ui->progressBarElapsed->setMaximum(nTimeOut);
    ui->progressBarElapsed->setVisible(false);
    lockInterface = false;
    lockUI(lockInterface);

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
    lockUI(lockInterface);
    ui->progressBarElapsed->setVisible(ntpSyncRunning);
    // Progress Bar di aggiornamento NTP
    if (ntpSyncRunning && nTimeOut && syncElapsed.isValid())  {
        int nElapsed = syncElapsed.elapsed() / 1000;
        if (nElapsed <= ui->progressBarElapsed->maximum())  {
            ui->progressBarElapsed->setValue(nElapsed);
        }
    }
}

void TimeSet::on_cmdBack_clicked()
{
    this->reject();
}



void TimeSet::on_pushButtonSetManual_clicked()
{
    QDateTime currentDT = iface2DateTime();
    if (currentDT.isValid())  {
        lockInterface = true;
        lockUI(lockInterface);
        QObject::connect(ntpClient, SIGNAL(ntpDateTimeChangeFinish(bool)), this, SLOT(ntpManualSetDone(bool)));
        datetimeTarget = currentDT;
        ntpClient->requestDateTimeChange(currentDT);
    }
}

void TimeSet::on_pushButtonNTPServer_clicked()
{
    char value [64];
    alphanumpad *tastiera_alfanum;
    tastiera_alfanum = new  alphanumpad(value, true, ui->pushButtonNTPServer->text().toAscii().data());
    tastiera_alfanum->setStyleSheet(szAlphaStyle);
    tastiera_alfanum->showFullScreen();
    if (tastiera_alfanum->exec()==QDialog::Accepted)
    {
        szTimeServer = QString(value);
        updateIface();
    }
    tastiera_alfanum->deleteLater();

}

void TimeSet::on_pushButtonNTPSet_clicked()
{
    if (! szTimeServer.isEmpty())  {
        ntpClient->setNtpParams(szTimeServer, nTimeOut, nOffset, nPeriod, isDst);
    }
}

void TimeSet::on_pushButtonNTPOffset_clicked()
{
    numpad * dk;
    int value = nOffset;
    int min = -12;
    int max = +12;

    dk = new numpad(&value, nOffset, min, max);
    dk->showFullScreen();
    if (dk->exec() == QDialog::Accepted)
    {
        nOffset = value;
        updateIface();
    }
}

void TimeSet::on_pushButtonNTPTimeOut_clicked()
{
    numpad * dk;
    int value = nTimeOut;
    int min = 1;
    int max = 60;

    dk = new numpad(&value, nTimeOut, min, max);
    dk->showFullScreen();

    if (dk->exec() == QDialog::Accepted)
    {
        nTimeOut = value;
        updateIface();
    }
}

void TimeSet::on_checkBoxDst_stateChanged(int state)
{
    switch (state) {
    case Qt::Checked:
        isDst = true;
        break;
    case Qt::Unchecked:
    default:
        isDst = false;
    }
}

void TimeSet::on_pushButtonNTPPeriod_clicked()
{
    numpad * dk;
    int value = nPeriod;
    int min = 0;
    int max = THE_NTP_MAX_PERIOD_H;

    dk = new numpad(&value, nPeriod, min, max);
    dk->showFullScreen();

    if (dk->exec() == QDialog::Accepted)
    {
        nPeriod = value;
        updateIface();
    }
}

void TimeSet::on_pushButtonNTPSync_clicked()
{
    // Avvio della sincronizzazione via NTP
    if (! szTimeServer.isEmpty() && nTimeOut)  {
        lockInterface = true;
        lockUI(lockInterface);
        ui->progressBarElapsed->setMaximum(nTimeOut);
        ntpClient->setNtpParams(szTimeServer, nTimeOut, nOffset, nPeriod, isDst);
        QObject::connect(ntpClient, SIGNAL(ntpSyncFinish(bool )), this, SLOT(ntpSyncDone(bool)));
        ntpClient->requestNTPSync();
        ntpSyncRunning = true;
        syncElapsed.restart();
    }
}

void TimeSet::updateIface(bool updateTime)
{
    ui->pushButtonNTPServer->setText(szTimeServer);
    ui->pushButtonNTPOffset->setText(QString("%1 h") .arg(nOffset,2,10));
    ui->checkBoxDst->setCheckState(isDst ? Qt::Checked : Qt::Unchecked);
    ui->pushButtonNTPTimeOut->setText(QString("%1 s") .arg(nTimeOut,2,10));
    ui->pushButtonNTPPeriod->setText(QString("%1 h") .arg(nPeriod,4,10));
    if (updateTime)  {
        // Current Time Settings
        QDate dNow = QDate::currentDate();
        QTime tNow = QTime::currentTime();
        if (dNow.year()  > 1970)  {
            ui->spinBoxAnno->setValue(dNow.year());
            ui->spinBoxMese->setValue(dNow.month());
            ui->spinBoxGiorno->setValue(dNow.day());
            ui->spinBoxOre->setValue(tNow.hour());
            ui->spinBoxMinuti->setValue(tNow.minute());
        }
        else  {
            ui->spinBoxAnno->setValue(2022);
            ui->spinBoxMese->setValue(06);
            ui->spinBoxGiorno->setValue(15);
            ui->spinBoxOre->setValue(9);
            ui->spinBoxMinuti->setValue(0);
        }
    }
}

void TimeSet::lockUI(bool setLocked)
{
    // Abilitazione interfaccia utente
    ui->spinBoxGiorno->setEnabled(! setLocked);
    ui->spinBoxMese->setEnabled(! setLocked);
    ui->spinBoxAnno->setEnabled(! setLocked);
    ui->spinBoxOre->setEnabled(! setLocked);
    ui->spinBoxMinuti->setEnabled(! setLocked);
    ui->pushButtonNTPServer->setEnabled(! setLocked);
    ui->pushButtonSetManual->setEnabled(! setLocked);
    ui->pushButtonNTPSet->setEnabled(! setLocked);
    ui->pushButtonNTPSync->setEnabled(! setLocked);
    ui->pushButtonNTPOffset->setEnabled(! setLocked);
    ui->checkBoxDst->setEnabled(! setLocked);
    ui->pushButtonNTPTimeOut->setEnabled(! setLocked);
    ui->pushButtonNTPPeriod->setEnabled(! setLocked);
    ui->cmdBack->setEnabled(! setLocked);
}

QDateTime TimeSet::iface2DateTime()
{
    int year = ui->spinBoxAnno->value();
    int mon = ui->spinBoxMese->value();
    int mday = ui->spinBoxGiorno->value();

    int hour = ui->spinBoxOre->value();
    int min = ui->spinBoxMinuti->value();
    int sec = 0;
    QDate d = QDate(year, mon, mday);
    QTime t = QTime(hour, min, sec);

    QDateTime ifaceDT = QDateTime(d, t);

    return ifaceDT;
}

void TimeSet::ntpManualSetDone(bool setOk)
{
    QObject::disconnect(ntpClient, 0, 0, 0);
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
    QObject::disconnect(ntpClient, 0, 0, 0);
    ntpSyncRunning = false;
    if (timeOut)  {
        QMessageBox::warning(this,trUtf8("NTP Time Error"), trUtf8("Time Out syncing Date and Time with NTP Server:\n%1") .arg(szTimeServer));
    }
    else  {
        QMessageBox::information(this,trUtf8("NTP Time Set"), trUtf8("Current Date and Time set from NTP Server:\n%1") .arg(szTimeServer));
    }
    updateIface(true);
    lockInterface = false;
    lockUI(lockInterface);
}

