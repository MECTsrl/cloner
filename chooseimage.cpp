#include "chooseimage.h"
#include "ui_chooseimage.h"

#include "publics.h"

#include <QDir>

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
    retentiveMode = RETENTIVE_IGNORE;
    ui->optIgnore->setChecked(true);
    if (ui->lstImmagini->count())  {
        ui->cmdOk->setEnabled(true);
        ui->lstImmagini->setFocus();
        ui->lstImmagini->setCurrentRow(0);
    }
    else {
        ui->cmdOk->setEnabled(false);
    }
}

ChooseImage::~ChooseImage()
{
    delete ui;
}

void ChooseImage::timerEvent(QTimerEvent *event)
{
    // Aggiornamento orologio
    QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->lblDateTime->setText(szDateTime);
}


QString ChooseImage::getSelectedImage(int &nRetentiveMode)
{
    nRetentiveMode = retentiveMode;
    return selectedImage;
}

void ChooseImage::on_cmdCancel_clicked()
{
    this->reject();
}

void ChooseImage::on_cmdOk_clicked()
{


    if (! selectedImage.isEmpty())  {
        if (QMessageBox::question(this, "Confirm Restore", QString("Confirm Restore from:\n\n[%1]\n\nCloner Image?").arg(selectedImage),
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
    return lstDir.count();
}

void ChooseImage::on_lstImmagini_currentRowChanged(int currentRow)
{
    if (currentRow >= 0 && currentRow < ui->lstImmagini->count())  {
        selectedImage = ui->lstImmagini->currentItem()->text();
    }
}

void ChooseImage::on_optIgnore_clicked(bool checked)
{
    if (checked)  {
        retentiveMode = RETENTIVE_IGNORE;
    }
}

void ChooseImage::on_optReset_clicked(bool checked)
{
    if (checked)  {
        retentiveMode = RETENTIVE_RESET;
    }
}

void ChooseImage::on_optRestore_clicked(bool checked)
{
    if (checked)  {
        retentiveMode = RETENTIVE_RESTORE;
    }
}
