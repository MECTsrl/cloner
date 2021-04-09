#ifndef MANAGESSH_H
#define MANAGESSH_H

#include <QDialog>
#include <QStringList>
#include <QTableWidget>


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
    void on_tblUsb_itemClicked(QTableWidgetItem *item);
    void on_tblTPac_itemClicked(QTableWidgetItem *item);
    void on_cmdAdd_clicked();
    void on_cmdRemove_clicked();

    void on_cmdOk_clicked();

private:
    bool            loadSSHKeys();                      // Read authorized_keys file on TPAC
    bool            loadSSHFiles();                     // Read ssh keys files on USB Key
    bool            saveSSHKeys();                      // Save authorized_keys file on TPAC
    void            clearTable(QTableWidget *table);
    void            lockTableRow(QTableWidget *table, int nRow2Lock);  // Lock a table Row
    void            updateUSBInfo(int nRow);            // Update selected file Info
    void            updateTPacInfo(int nRow);           // Update selected certificate Info

    Ui::ManageSSH   *ui;
    QStringList     lstSSH_TPacKeys;                    // SSH Key on TPAC      (all Info)
    QStringList     lstSSH_USBKeys;                     // SSH Key on USB Files (all Info)
    int             nCurrentFile;                       // Current USB File
    int             nCurrentKey;                        // Current SSH Key on TPAC
    int             nSmilyKey;                          // Position of sMily Key
    QString         szSmilyKey;                         // Value of sMily Key
    int             nChanges;                           // Number ok changes done
};

#endif // MANAGESSH_H
