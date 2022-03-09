#ifndef MANAGEVPN_H
#define MANAGEVPN_H

#include <QDialog>

namespace Ui {
class ManageVPN;
}

class ManageVPN : public QDialog
{
    Q_OBJECT

public:
    explicit ManageVPN(QWidget *parent = 0);
    ~ManageVPN();
    int     getSelectedAction();        // Return user selected Action

protected:
    void    timerEvent(QTimerEvent *event);

private slots:
    void on_cmdCancel_clicked();
    void on_cmdOk_clicked();
    void on_cmdRemove_clicked();
    void on_cmdAdd_clicked();

private:
    Ui::ManageVPN   *ui;
    int             userAction;
};

#endif // MANAGEVPN_H
