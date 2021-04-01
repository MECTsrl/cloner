#ifndef MANAGESSH_H
#define MANAGESSH_H

#include <QDialog>

namespace Ui {
class ManageSSH;
}

class ManageSSH : public QDialog
{
    Q_OBJECT

public:
    explicit ManageSSH(QWidget *parent = 0);
    ~ManageSSH();

protected:
    void    timerEvent(QTimerEvent *event);

private slots:
    void on_cmdCancel_clicked();

private:
    Ui::ManageSSH *ui;
};

#endif // MANAGESSH_H
