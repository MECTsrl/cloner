#include "managessh.h"
#include "ui_managessh.h"

#include "publics.h"

#include <QDir>
#include <QFile>
#include <QTableWidgetItem>
#include <QHeaderView>


ManageSSH::ManageSSH(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManageSSH)
{
    ui->setupUi(this);
    ui->lblModel->setText(szModel);
    // Clear Tables
    clearTable(ui->tblTPac);
    clearTable(ui->tblUsb);
    // Load SSH Key on TPac
    if (loadSSHKeys())  {
        ui->tblTPac->setEnabled(true);
    }
    startTimer(REFRESH_MS);
}

ManageSSH::~ManageSSH()
{
    delete ui;
}

void ManageSSH::timerEvent(QTimerEvent *event)
{
    // Aggiornamento orologio
    QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->lblDateTime->setText(szDateTime);
}

void ManageSSH::on_cmdCancel_clicked()
{
    this->reject();
}

bool    ManageSSH::loadSSHKeys()
{
    bool                fRes = false;
    QStringList         lstRawKeys;
    QTableWidgetItem    *tItem;
    int                 nRows = 0;
    int                 nSmilyRow = -1;
    int                 nFirstSelectable = -1;

    lstSSHKeys.clear();
    QFile sshFile(SSH_KEY_FILE);
    if (sshFile.open(QIODevice::ReadOnly)) {
        while (! sshFile.atEnd())  {
            lstRawKeys.append(sshFile.readLine().simplified());
        }
        sshFile.close();
        fprintf(stderr, "%s: Read:%d Items\n", SSH_KEY_FILE, lstRawKeys.count());
        // Parse and load Keys
        if (lstRawKeys.count() > 0)  {
            ui->tblTPac->setColumnCount(3);
            for (int nKey = 0; nKey < lstRawKeys.count(); nKey++)  {
                QString     sshKey = lstRawKeys.at(nKey);
                QStringList lstItems = sshKey.split(" ", QString::SkipEmptyParts);
                if (lstItems.count() > SSH_KEY_COMMENT)  {
                    lstSSHKeys.append(sshKey);
                    ui->tblTPac->insertRow(nRows++);
                    // Add Row Info
                    tItem = new QTableWidgetItem(QString::number(nKey));
                    ui->tblTPac->setItem(ui->tblTPac->rowCount() - 1, 0, tItem);
                    // Add Type Info
                    tItem = new QTableWidgetItem(lstItems.at(SSH_KEY_TYPE));
                    ui->tblTPac->setItem(ui->tblTPac->rowCount() - 1, 1, tItem);
                    // Add Comment Info
                    tItem = new QTableWidgetItem(lstItems.at(SSH_KEY_COMMENT));
                    ui->tblTPac->setItem(ui->tblTPac->rowCount() - 1, 2, tItem);
                    // Lock Smily Row
                    if (lstItems.at(SSH_KEY_COMMENT) == QString(SSH_KEY_SMILY))  {
                        nSmilyRow = ui->tblTPac->rowCount() - 1;
                        ui->tblTPac->item(nSmilyRow, 0)->setFlags(Qt::NoItemFlags);
                        ui->tblTPac->item(nSmilyRow, 1)->setFlags(Qt::NoItemFlags);
                        ui->tblTPac->item(nSmilyRow, 2)->setFlags(Qt::NoItemFlags);
                    }
                    else if (nFirstSelectable < 0)  {
                        nFirstSelectable = ui->tblTPac->rowCount() - 1;
                    }
                }
            }
            // Column Headers
            if (lstSSHKeys.count() > 0)  {
                QStringList lstCols;
                lstCols.append("Row");
                lstCols.append("Type");
                lstCols.append("Comment");
                ui->tblTPac->setHorizontalHeaderLabels(lstCols);
                ui->tblTPac->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
                ui->tblTPac->horizontalHeader()->setStretchLastSection(true);
                ui->tblTPac->verticalHeader()->hide();
                ui->tblTPac->setColumnHidden(0, true);
                fRes = true;
            }
            // Select first user available row
            if (nFirstSelectable >= 0)  {
                ui->tblTPac->selectRow(nFirstSelectable);
            }
            ui->tblTPac->update();
        }
    }
    return fRes;
}

void  ManageSSH::clearTable(QTableWidget *table)
{
    table->clear();
    table->setRowCount(0);
    table->setColumnCount(0);
    table->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    table->horizontalHeader()->reset();
    table->setHorizontalHeaderLabels(QStringList());
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
}
