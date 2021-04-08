#include "managessh.h"
#include "ui_managessh.h"

#include "publics.h"

#include <QDir>
#include <QFile>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>


ManageSSH::ManageSSH(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManageSSH)
{
    ui->setupUi(this);
    ui->lblModel->setText(szModel);
    ui->lblKey->setText("");
    ui->lblFile->setText("");
    nCurrentFile = -1;
    nCurrentKey = -1;
    // Clear Tables
    clearTable(ui->tblTPac);
    clearTable(ui->tblUsb);
    // Load SSH Key on TPac
    if (loadSSHKeys())  {
        ui->tblTPac->setEnabled(true);
    }
    // Load SSH Key files on USB
    if (loadSSHFiles())  {
        ui->tblUsb->setEnabled(true);
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
// Read authorized_keys file on TPAC
{
    bool                fRes = false;
    QStringList         lstRawKeys;
    QTableWidgetItem    *tItem;
    int                 nRows = 0;
    int                 nFirstSelectable = -1;

    lstSSH_TPacKeys.clear();
    QFile sshFile(SSH_KEY_FILE);
    if (sshFile.open(QIODevice::ReadOnly)) {
        while (! sshFile.atEnd())  {
            lstRawKeys.append(sshFile.readLine().simplified());
        }
        sshFile.close();
        // Parse and load Keys
        if (lstRawKeys.count() > 0)  {
            ui->tblTPac->setColumnCount(2);
            for (int nKey = 0; nKey < lstRawKeys.count(); nKey++)  {
                QString     sshKey = lstRawKeys.at(nKey);
                QStringList lstItems = sshKey.split(" ", QString::SkipEmptyParts);
                if (lstItems.count() > SSH_KEY_COMMENT)  {
                    lstSSH_TPacKeys.append(sshKey);
                    ui->tblTPac->insertRow(nRows);
                    // Add Type Info
                    tItem = new QTableWidgetItem(lstItems.at(SSH_KEY_TYPE));
                    ui->tblTPac->setItem(nRows, 0, tItem);
                    // Add Comment Info
                    tItem = new QTableWidgetItem(lstItems.at(SSH_KEY_COMMENT));
                    ui->tblTPac->setItem(nRows, 1, tItem);
                    // Lock Smily Row
                    if (lstItems.at(SSH_KEY_COMMENT) == QString(SSH_KEY_SMILY))  {
                        lockTableRow(ui->tblTPac, nRows);
                    }
                    else if (nFirstSelectable < 0)  {
                        nFirstSelectable = nRows;
                    }
                    nRows++;
                }
            }
            // Column Headers
            if (lstSSH_TPacKeys.count() > 0)  {
                QStringList lstCols;
                lstCols.append("Type");
                lstCols.append("Comment");
                ui->tblTPac->setHorizontalHeaderLabels(lstCols);
                ui->tblTPac->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
                ui->tblTPac->horizontalHeader()->setStretchLastSection(true);
                ui->tblTPac->verticalHeader()->hide();
                fRes = true;
            }
            // Select first user available row
            if (nFirstSelectable >= 0)  {
                ui->tblTPac->selectRow(nFirstSelectable);
                ui->lblKey->setText(QString("Key:%1") .arg(nFirstSelectable + 1, 3, 10));
                nCurrentKey = nFirstSelectable;
            }
            ui->tblTPac->update();
        }
        ui->lblNumTP->setText(QString::number(lstSSH_TPacKeys.count()));
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

bool  ManageSSH::loadSSHFiles()
// Read ssh keys files on USB Key
{
    bool                fRes = false;
    QTableWidgetItem    *tItem;
    int                 nRows = 0;
    QString             szFirstFile;
    QDir                dirUSB(SSH_KEY_DIR);
    QStringList         lstFilesUSB = dirUSB.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Readable,
                                                       QDir::Name | QDir::IgnoreCase);

    if (lstFilesUSB.count() > 0)  {
        ui->tblUsb->setColumnCount(3);
        for (int nFile = 0; nFile < lstFilesUSB.count(); nFile++)  {
            QString szFileName = QString("%1%2") .arg(SSH_KEY_DIR) .arg(lstFilesUSB.at(nFile));
            QFile   sshFile(szFileName);
            if (sshFile.open(QIODevice::ReadOnly) && ! sshFile.atEnd()) {
                if (! sshFile.atEnd())  {
                    // Read only first line of each file on USB Key
                    QString     sshKey = sshFile.readLine().simplified();
                    QStringList lstItems = sshKey.split(" ", QString::SkipEmptyParts);
                    if (lstItems.count() > SSH_KEY_COMMENT)  {
                        lstSSH_USBKeys.append(sshKey);
                        ui->tblUsb->insertRow(nRows++);
                        if (szFirstFile.isEmpty())  {
                            szFirstFile = lstFilesUSB.at(nFile);
                        }
                        // Add file name Info
                        tItem = new QTableWidgetItem(lstFilesUSB.at(nFile));
                        ui->tblUsb->setItem(ui->tblUsb->rowCount() - 1, 0, tItem);
                        // Add Type Info
                        tItem = new QTableWidgetItem(lstItems.at(SSH_KEY_TYPE));
                        ui->tblUsb->setItem(ui->tblUsb->rowCount() - 1, 1, tItem);
                        // Add Comment Info
                        tItem = new QTableWidgetItem(lstItems.at(SSH_KEY_COMMENT));
                        ui->tblUsb->setItem(ui->tblUsb->rowCount() - 1, 2, tItem);
                    }
                }
                sshFile.close();
            }
        }
        // Almost one key loaded, adjust Column headers
        if (lstSSH_USBKeys.count() > 0)  {
            QStringList lstCols;
            lstCols.append("File");
            lstCols.append("Type");
            lstCols.append("Comment");
            ui->tblUsb->setHorizontalHeaderLabels(lstCols);
            ui->tblUsb->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
            ui->tblUsb->horizontalHeader()->setStretchLastSection(true);
            ui->tblUsb->setColumnHidden(0, true);
            ui->tblUsb->verticalHeader()->hide();
            ui->tblUsb->selectRow(0);
            ui->lblNumUSB->setText(QString::number(lstSSH_USBKeys.count()));
            ui->lblFile->setText(szFirstFile);
            nCurrentFile = 0;
            fRes = true;
        }
    }
    return fRes;
}

void ManageSSH::on_tblUsb_itemClicked(QTableWidgetItem *item)
{
    QTableWidgetItem *fileCell;
    int nRow = item->row();

    ui->lblFile->setText("");
    nCurrentFile = -1;
    if (nRow >= 0 && nRow < ui->tblUsb->rowCount())  {
        // retrieve file name from hidden column 0
        fileCell = ui->tblUsb->item(nRow, 0);
        if (fileCell)  {
            ui->lblFile->setText(fileCell->text());
            nCurrentFile = nRow;
        }
    }
}

void ManageSSH::on_tblTPac_itemClicked(QTableWidgetItem *item)
{
    QTableWidgetItem *rowCell;
    int nRow = item->row();

    ui->lblKey->setText("");
    nCurrentKey = -1;
    if (nRow >= 0 && nRow < ui->tblTPac->rowCount())  {
        // retrieve file name from hidden column 0
        rowCell = ui->tblTPac->item(nRow, 1);
        if (rowCell)  {
            QString szText = rowCell->text();
            if (szText != QString(SSH_KEY_SMILY))  {
                nCurrentKey = nRow;
                ui->lblKey->setText(QString("Key:%1") .arg(nRow + 1, 3, 10));
            }
        }
    }
}

void ManageSSH::lockTableRow(QTableWidget *table, int nRow2Lock)
// Lock a table Row
{
    if (nRow2Lock >= 0 && nRow2Lock < table->rowCount())  {
        for (int nCol = 0; nCol < table->columnCount(); nCol++)  {
            table->item(nRow2Lock, nCol)->setFlags(Qt::NoItemFlags);
        }
    }
}

void ManageSSH::on_cmdAdd_clicked()
// Add to TPAC Key List a Key imported from file
{
    QTableWidgetItem    *tItem;

    if (nCurrentFile >= 0 && nCurrentFile < ui->tblUsb->rowCount() && nCurrentFile < lstSSH_USBKeys.count())  {
        QString szNewKey = lstSSH_USBKeys.at(nCurrentFile);
        QStringList lstItems = szNewKey.split(" ", QString::SkipEmptyParts);
        if (lstItems.count() > SSH_KEY_COMMENT)  {
            lstSSH_TPacKeys.append(szNewKey);
            int nNewRowIndex = ui->tblTPac->rowCount();
            ui->tblTPac->insertRow(nNewRowIndex);
            // Add Type Info
            tItem = new QTableWidgetItem(lstItems.at(SSH_KEY_TYPE));
            ui->tblTPac->setItem(nNewRowIndex, 0, tItem);
            // Add Comment Info
            tItem = new QTableWidgetItem(lstItems.at(SSH_KEY_COMMENT));
            ui->tblTPac->setItem(nNewRowIndex, 1, tItem);
            // Remove Current File from List
            lstSSH_USBKeys.removeAt(nCurrentFile);
            ui->tblUsb->removeRow(nCurrentFile);
            if (lstSSH_USBKeys.count() > 0)  {
                nCurrentFile = 0;
            }
            else  {
                ui->cmdAdd->setEnabled(false);
            }
            ui->tblUsb->selectRow(nCurrentFile);
            // Update Interface
            ui->lblNumTP->setText(QString::number(lstSSH_TPacKeys.count()));
            ui->lblNumUSB->setText(QString::number(lstSSH_USBKeys.count()));
            ui->tblTPac->update();
            ui->tblUsb->update();
        }
    }
}

void ManageSSH::on_cmdRemove_clicked()
// Remove a key from TPAC Key List
{
    QTableWidgetItem    *tItem;

    if (nCurrentKey >= 0 && nCurrentKey  < ui->tblTPac->rowCount() && nCurrentKey < lstSSH_USBKeys.count())  {
        // Check if sMily Key
        tItem = ui->tblTPac->item(nCurrentKey, 2);
        if (tItem->text() != QString(SSH_KEY_SMILY))  {
            // Remove Current Key from List
            lstSSH_TPacKeys.removeAt(nCurrentKey);
            ui->tblTPac->removeRow(nCurrentKey);
            nCurrentKey = -1;
            ui->tblTPac->selectRow(nCurrentKey);
            // Update Interface
            ui->lblKey->setText("");
            ui->lblNumTP->setText(QString::number(lstSSH_TPacKeys.count()));
            ui->tblTPac->update();
        }
    }

}
