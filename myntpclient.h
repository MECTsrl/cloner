#ifndef MYNTPCLIENT_H
#define MYNTPCLIENT_H

#include <QThread>
#include <QMutex>
#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QElapsedTimer>
#include <QDateTime>
#include <QSemaphore>
#include <sys/types.h>

#define THE_NTP_MAX_PERIOD_H    2982       // Massimo numero di ore convertite in ms per Int32
#define THE_NTP_SERVER     "tempo.ien.it"

class MyNtpClient : public QThread
{
    Q_OBJECT

public:
    explicit    MyNtpClient(QObject *parent = 0);
    QString     getNtpServer();
    int         getTimeout_s();
    int         getOffset_h();
    bool        getDst();
    int         getPeriod_h();
    void        setNtpParams(const QString &server, int timeout_s, int offset_h, int period_h, bool dst = false);

signals:
    void        ntpSyncFinish(bool timeOut);                    // ntp Sync Ended
    void        ntpDateTimeChangeFinish(bool setOk);            // Manual Sync Ended

public slots:
    void        requestNTPSync();                               // Starts ntp time sync
    void        requestDateTimeChange(QDateTime newTime);       // Force RTC to newTime

protected:
    virtual void    run();
    bool            ntpSyncOrChangeRequested();     // check if a Clock change is requested
    void            doSyncOrChange();               // do a Clock change
    bool            isTimeChanged();
    QDateTime       getTimeBefore();


private:
    int64_t         getTimestamp();
    void            setTimestamp(int64_t TargetTimestamp);
    bool            ntpClientProcedure();


    QElapsedTimer   ntpSyncTimer;           // Timer on ntpPeriod
    bool            doSync;
    QDateTime       newDateTime;            // Manual QDateTime to set
    QDateTime       invalidDateTime;        // Invalid QDateTime

    QString         ntpServerName;          // ntp params
    int             ntpTimeoutSecs;
    int             ntpOffsetHours;
    bool            ntpDst;
    int             ntpPeriodHours;
    qint64          ntpPeriodms;

    bool            timeChanged;            // Rilevato un cambio di data e ora genera una riga vuota nei log per segnare il cambio di tempo
    QDateTime       timeBeforeChange;
    QMutex          mutexNTP;
    QSemaphore      ntpSem;

};

extern MyNtpClient    *ntpClient;


#endif //  MYNTPCLIENT_H
