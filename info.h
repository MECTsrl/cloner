#ifndef INFO_H
#define INFO_H

#include <QDialog>

namespace Ui {
class Info;
}

class Info : public QDialog
{
    Q_OBJECT

public:
    explicit Info(QWidget *parent = 0);
    ~Info();

protected:
    void    timerEvent(QTimerEvent *event);

private slots:
    void on_cmdBack_clicked();
    void on_cmdRelolad_clicked();

private:
    void refreshAllTabs();
    void refreshSystemTab();
    void refreshNetworkingTabs();
    void refreshNTPInfo();

    Ui::Info *ui;
};

#endif // INFO_H
