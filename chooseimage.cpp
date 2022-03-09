#include "chooseimage.h"
#include "ui_chooseimage.h"

#include "publics.h"

#include <QListWidgetItem>

ChooseImage::ChooseImage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseImage)
{
    ui->setupUi(this);
    ui->lblAction->setText("Restore from Cloner Image");
    ui->lblModel->setText(szModel);
    startTimer(REFRESH_MS);
    selectedImage.clear();
    fillImagesList();
    // Default values: Full Restore
    ui->optRetRestore->setChecked(true);
    ui->optIniRestore->setChecked(true);
    ui->optLogRestore->setChecked(true);
    // Check images
    if (ui->lstImmagini->count())  {
        ui->cmdOk->setEnabled(true);
        ui->lstImmagini->setFocus();
        ui->lstImmagini->setCurrentRow(0);
    }
    else {
        ui->lstImmagini->setEnabled(false);
        ui->cmdOk->setEnabled(false);
    }
}

ChooseImage::~ChooseImage()
{
    delete ui;
}

void ChooseImage::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    // Aggiornamento orologio
    QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->lblDateTime->setText(szDateTime);
}


QString ChooseImage::getSelectedImage(int &nRetentiveMode, int &nHmiMode, int &nLogMode)
{
    // Retentives Vars
    if (ui->optRetIgnore->isChecked())  {
        nRetentiveMode = RESTORE_IGNORE;
    }
    else if (ui->optRetReset->isChecked())  {
        nRetentiveMode = RESTORE_RESET;

    }
    else if (ui->optRetRestore->isChecked())  {
        nRetentiveMode = RESTORE_RESTORE;
    }
    // HMI.ini
    if (ui->optIniIgnore->isChecked())  {
        nHmiMode = RESTORE_IGNORE;
    }
    else if (ui->optIniReset->isChecked())  {
        nHmiMode = RESTORE_RESET;

    }
    else if (ui->optIniRestore->isChecked())  {
        nHmiMode = RESTORE_RESTORE;
    }
    // Logs
    if (ui->optLogIgnore->isChecked())  {
        nLogMode = RESTORE_IGNORE;
    }
    else if (ui->optLogReset->isChecked())  {
        nLogMode = RESTORE_RESET;

    }
    else if (ui->optLogRestore->isChecked())  {
        nLogMode = RESTORE_RESTORE;
    }
    return selectedImage;
}

void ChooseImage::on_cmdCancel_clicked()
{
    this->reject();
}

void ChooseImage::on_cmdOk_clicked()
{


    if (! selectedImage.isEmpty())  {
        if (QMessageBox::question(this, "Confirm Restore", QString("Confirm Restore from:\n\n[%1]\nto Model: [%2]\nVersion: [%3] ?")
                            .arg(selectedImage) .arg(szModel) .arg(szTargetVersion),
                        QMessageBox::Ok|QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Ok)  {
            this->accept();
        }
    }
}

int ChooseImage::fillImagesList()
{
    QDir        imagesDir(CLONED_IMAGES_DIR);
    QStringList lstDir = imagesDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);

    if (lstDir.count())  {
        ui->lstImmagini->clear();
        for (int nItem = 0; nItem < lstDir.count(); nItem++)  {
            // Check localfs.tar existence
            QString localFsTar = QString("%1%2/%3") .arg(CLONED_IMAGES_DIR) .arg(lstDir.at(nItem)) .arg(LOCAL_FS_TAR);
            if (QFile::exists(localFsTar))  {
                QListWidgetItem *newItem = new QListWidgetItem;
                newItem->setText(lstDir.at(nItem));
                ui->lstImmagini->insertItem(nItem, newItem);
            }
            else {
                fprintf(stderr, "ChooseImage: Local tar file NOT found %s\n", localFsTar.toLatin1().data());
            }
        }
    }
    return (ui->lstImmagini->count());
}

void ChooseImage::on_lstImmagini_currentRowChanged(int currentRow)
{
    if (currentRow >= 0 && currentRow < ui->lstImmagini->count())  {
        selectedImage = ui->lstImmagini->currentItem()->text();
    }
}

