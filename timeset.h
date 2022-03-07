#ifndef TIMESET_H
#define TIMESET_H



#include <QDialog>
#include <QString>
#include <QElapsedTimer>
#include <QDateTime>



namespace Ui {
class TimeSet;
}

class TimeSet : public QDialog
{
    Q_OBJECT

public:
    explicit TimeSet(QWidget *parent = 0);
    ~TimeSet();

protected:
    void    timerEvent(QTimerEvent *event);

private slots:
    void on_cmdBack_clicked();

    void ntpSyncDone(bool timeOut);
    void ntpManualSetDone(bool setOk);
    void lockUI(bool setLocked);

private:
    void            updateIface();

    Ui::TimeSet     *ui;
    int             nOffset;
    bool            isDst;
    int             nTimeOut;
    int             nPeriod;
    QString         szTimeServer;
    bool            lockInterface;
    bool            ntpSyncRunning;
    QElapsedTimer   syncElapsed;
    QDateTime       datetimeTarget;
};

#endif // TIMESET_H
