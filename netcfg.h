#ifndef NETCFG_H
#define NETCFG_H

#include <QDialog>

namespace Ui {
class NetCfg;
}

class NetCfg : public QDialog
{
    Q_OBJECT

public:
    explicit NetCfg(QWidget *parent = 0);
    ~NetCfg();

protected:
    void    timerEvent(QTimerEvent *event);

private slots:
    void on_cmdCancel_clicked();

private:
    Ui::NetCfg *ui;
};

#endif // NETCFG_H
