#include "managessh.h"
#include "ui_managessh.h"

#include "publics.h"

#include <QDir>
#include <QFile>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QTextStream>

ManageSSH::ManageSSH(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManageSSH)
{
    ui->setupUi(this);
    ui->frameTitle->setVisible(false);
    ui->lblModel->setText(szModel);
    ui->lblKey->setText("");
    ui->lblFile->setText("");
    nCurrentFile = -1;
    nCurrentKey = -1;
    nSmilyKey = -1;
    nChanges = 0;
    szSmilyKey.clear();
    // Clear Tables
    clearTable(ui->tblTPac);
    clearTable(ui->tblUsb);
    // Load SSH Key on TPac
    if (loadSSHKeys())  {
        ui->tblTPac->setEnabled(true);
    }
    ui->cmdRemove->setEnabled(ui->tblTPac->rowCount() > 0);
    // Load SSH Key files on USB
    if (loadSSHFiles())  {
        ui->tblUsb->setEnabled(true);
    }
    else {
        ui->tblUsb->setEnabled(false);
    }
    ui->cmdAdd->setEnabled(ui->tblUsb->rowCount() > 0);
    startTimer(REFRESH_MS);
}

ManageSSH::~ManageSSH()
{
    delete ui;
}

void ManageSSH::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    // Aggiornamento orologio
    QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->lblDateTime->setText(szDateTime);
}

void ManageSSH::on_cmdCancel_clicked()
{
    if (nChanges)  {
        if (QMessageBox::question(this, this->windowTitle(),
                QString("There are unsaved changes\nQuit anyway?"),
                QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok) != QMessageBox::Ok)  {
            // do nothing
            return;
        }
    }
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
                        nSmilyKey = nRows;
                        szSmilyKey = sshKey;
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
                nCurrentKey = nFirstSelectable;
                updateTPacInfo(nCurrentKey);
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
                        ui->tblUsb->insertRow(nRows);
                        // Add Type Info
                        tItem = new QTableWidgetItem(lstItems.at(SSH_KEY_TYPE));
                        ui->tblUsb->setItem(nRows, 0, tItem);
                        // Add Comment Info
                        tItem = new QTableWidgetItem(lstItems.at(SSH_KEY_COMMENT));
                        ui->tblUsb->setItem(nRows, 1, tItem);
                        // Add file name Info
                        tItem = new QTableWidgetItem(lstFilesUSB.at(nFile));
                        ui->tblUsb->setItem(nRows, 2, tItem);
                        // Lock Key already present on TPac
                        if (lstSSH_TPacKeys.indexOf(sshKey) >= 0)  {
                            lockTableRow(ui->tblUsb, nRows);
                        }
                        nRows++;
                    }
                }
                sshFile.close();
            }
        }
        // Almost one key loaded, adjust Column headers
        if (lstSSH_USBKeys.count() > 0)  {
            QStringList lstCols;
            lstCols.append("Type");
            lstCols.append("Comment");
            lstCols.append("File");
            ui->tblUsb->setColumnHidden(2, true);
            ui->tblUsb->setHorizontalHeaderLabels(lstCols);
            ui->tblUsb->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
            ui->tblUsb->horizontalHeader()->setStretchLastSection(true);
            ui->tblUsb->verticalHeader()->hide();
            // Search first usable key
            nRows = -1;
            for (int nItem = 0; nItem < ui->tblUsb->rowCount(); nItem++)  {
                QString     sshKey = lstSSH_USBKeys.at(nItem);
                // check Key presence on TPac
                if (lstSSH_TPacKeys.indexOf(sshKey) < 0)  {
                    nRows = nItem;
                    break;
                }
            }
            ui->tblUsb->selectRow(nRows);
            updateUSBInfo(nRows);
            nCurrentFile = nRows;
            fRes = true;
        }
    }
    return fRes;
}

void ManageSSH::updateUSBInfo(int nRow)
// Update selected file Info
{
    QTableWidgetItem *fileCell;
    // retrieve file name from hidden column 0
    ui->lblFile->setText("");
    fileCell = ui->tblUsb->item(nRow, 2);
    if (fileCell)  {
        ui->lblFile->setText(fileCell->text());
    }
    ui->lblNumUSB->setText(QString::number(lstSSH_USBKeys.count()));
}

void ManageSSH::updateTPacInfo(int nRow)
// Update selected certificate Info
{
    ui->lblKey->setText("");
    if (nRow >= 0 && nRow < ui->tblTPac->rowCount())  {
        ui->lblKey->setText(QString("Key:%1") .arg(nRow + 1, 3, 10));
    }
    ui->lblNumTP->setText(QString::number(lstSSH_TPacKeys.count()));
}

void ManageSSH::on_tblUsb_itemClicked(QTableWidgetItem *item)
{
    int nRow = item->row();

    nCurrentFile = -1;
    if (nRow >= 0 && nRow < ui->tblUsb->rowCount())  {
        QString sshKey = lstSSH_USBKeys.at(nRow);
        // check key presence on TPac
        if (lstSSH_TPacKeys.indexOf(sshKey) < 0)  {
            updateUSBInfo(nRow);
            nCurrentFile = nRow;
        }
    }
}

void ManageSSH::on_tblTPac_itemClicked(QTableWidgetItem *item)
{
    int nRow = item->row();

    if (nRow >= 0 && nRow < ui->tblTPac->rowCount() && nRow != nSmilyKey)  {
        updateTPacInfo(nRow);
        nCurrentKey = nRow;
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
        // check Key existence
        if (lstSSH_TPacKeys.indexOf(szNewKey) < 0)  {
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
                // Get Name of file added
                tItem = ui->tblUsb->item(nCurrentFile, 2);
                QString szFileName = tItem->text();
                // Log Action
                fprintf(stderr, "Added SSH Key [%s] from USB file: [%s]\n",
                            lstItems.at(SSH_KEY_COMMENT).toLatin1().data(), szFileName.toLatin1().data());
                nChanges++;
                // Remove Current File from USB List and USB Table
                lstSSH_USBKeys.removeAt(nCurrentFile);
                ui->tblUsb->removeRow(nCurrentFile);
                if (lstSSH_USBKeys.count() > 0)  {
                    if (nCurrentFile >= ui->tblUsb->rowCount())  {
                        nCurrentFile = ui->tblUsb->rowCount() -1;
                    }
                    updateUSBInfo(nCurrentFile);
                }
                else  {
                    ui->lblFile->setText("");
                    ui->cmdAdd->setEnabled(false);
                    ui->tblUsb->setEnabled(false);
                }
                ui->tblUsb->selectRow(nCurrentFile);
                ui->lblNumUSB->setText(QString::number(lstSSH_USBKeys.count()));
                // Update TPAC Table info
                ui->lblNumTP->setText(QString::number(lstSSH_TPacKeys.count()));
                ui->tblTPac->selectRow(nNewRowIndex);
                updateTPacInfo(nNewRowIndex);
                // Repaint
                ui->tblTPac->update();
                ui->tblUsb->update();
            }
        }
    }
}

void ManageSSH::on_cmdRemove_clicked()
// Remove a key from TPAC Key List
{
    if (nCurrentKey >= 0 && nCurrentKey  < ui->tblTPac->rowCount() && nCurrentKey < lstSSH_TPacKeys.count())  {
        // Check if sMily Key
        QString sshKey = lstSSH_TPacKeys.at(nCurrentKey);
        if (sshKey != szSmilyKey)  {
            // Remove Current Key from List
            lstSSH_TPacKeys.removeAt(nCurrentKey);
            ui->tblTPac->removeRow(nCurrentKey);
            // Log Action
            QStringList lstItems = sshKey.split(" ", QString::SkipEmptyParts);
            fprintf(stderr, "Removed SSH Key [%s] from TPac\n",
                        lstItems.at(SSH_KEY_COMMENT).toLatin1().data());
            nChanges++;
            // Update sMily Key Position
            if (not szSmilyKey.isEmpty())  {
                nSmilyKey = lstSSH_TPacKeys.indexOf(szSmilyKey);
            }
            else  {
                nSmilyKey = -1;
            }
            if (lstSSH_TPacKeys.count() > 0)  {
                // Go on last key
                if (nCurrentKey >= ui->tblTPac->rowCount() || nCurrentKey == nSmilyKey)  {
                    nCurrentKey = ui->tblTPac->rowCount() -1;
                }
                updateTPacInfo(nCurrentKey);
            }
            else  {
                ui->cmdRemove->setEnabled(false);
                ui->tblTPac->setEnabled(false);
            }
            ui->tblTPac->update();
        }
    }
}

bool ManageSSH::saveSSHKeys()
// Save authorized_keys file on TPAC
{
    bool fRes = false;

    // Mount Root File System as RW
    system(MOUNT_ROOTFS_RW);
    // Open SSH authorized_keys file
    QFile sshFile(SSH_KEY_FILE);
    if (sshFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QTextStream txtSSH(&sshFile);
        for (int nItem = 0; nItem < lstSSH_TPacKeys.count(); nItem++)  {
            txtSSH << lstSSH_TPacKeys.at(nItem) << endl;
        }
        txtSSH.flush();
        sshFile.close();
        system("sync");
        fRes = true;
    }
    // Mount Root File System as RO
    system(MOUNT_ROOTFS_RO);
    return fRes;
}


void ManageSSH::on_cmdOk_clicked()
{
    if (nChanges)  {
        if (QMessageBox::question(this, this->windowTitle(),
                QString("Save Changed SSH Keys on Panel ?"),
                QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Ok) != QMessageBox::Ok)  {
            // do nothing
            return;
        }
        else {
            if (saveSSHKeys())  {
                QMessageBox::information(this, this->windowTitle(), QString("SSH Keys file [%1] Updated\nWritten [%2] Keys") .arg(SSH_KEY_FILE) .arg(lstSSH_TPacKeys.count()), QMessageBox::Ok);
            }
            else  {
                QMessageBox::critical(this, this->windowTitle(), QString("Error updating SSH Keys file [%1]") .arg(SSH_KEY_FILE), QMessageBox::Ok);
            }
        }
    }
    this->accept();
}
