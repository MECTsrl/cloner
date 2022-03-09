#include "managevpn.h"
#include "ui_managevpn.h"

#include "publics.h"

#include <QListWidgetItem>
#include <QFileInfo>

ManageVPN::ManageVPN(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ManageVPN)
{
    QString szFileName;
    QString szPath;

    ui->setupUi(this);
    ui->lblModel->setText(szModel);
    startTimer(REFRESH_MS);
    userAction = OVPN_NO_ACTION;
    // Left Panel
    if (QFile::exists(szVPNOriginalFile))  {
        QFileInfo fi(szVPNOriginalFile);
        szFileName = fi.fileName();
        szPath = QString("%1 [%2]") .arg(ui->lblTPac->text()) .arg(fi.path());
        ui->lblTPac->setText(szPath);
        QListWidgetItem *newItem = new QListWidgetItem;
        newItem->setText(szFileName);
        ui->lstTPac->insertItem(0, newItem);
        ui->lblNumTP->setText("1");
        ui->lstTPac->setEnabled(true);
        ui->lstTPac->setCurrentRow(0);
        ui->lstTPac->setFocus();
    }
    ui->cmdRemove->setEnabled(ui->lstTPac->count() > 0);
    // Right Panel
    if (QFile::exists(szVPNNewFile))  {
        QFileInfo fi(szVPNNewFile);
        szFileName = fi.fileName();
        szPath = QString("%1 [%2]") .arg(ui->lblUsb->text()) .arg(fi.path());
        ui->lblUsb->setText(szPath);
        QListWidgetItem *newItem = new QListWidgetItem;
        newItem->setText(szFileName);
        ui->lstUsb->insertItem(0, newItem);
        ui->lblNumUSB->setText("1");
        ui->lstUsb->setEnabled(true);
        ui->lstUsb->setCurrentRow(0);
        ui->lstUsb->setFocus();
    }
    ui->cmdAdd->setEnabled(ui->lstUsb->count() > 0);
}

ManageVPN::~ManageVPN()
{
    delete ui;
}

void ManageVPN::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    // Aggiornamento orologio
    QString szDateTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    ui->lblDateTime->setText(szDateTime);
}

void ManageVPN::on_cmdCancel_clicked()
{
    this->reject();
}

void ManageVPN::on_cmdOk_clicked()
{
    this->accept();
}

int     ManageVPN::getSelectedAction()
// Return user selected Action
{
    return userAction;
}


void ManageVPN::on_cmdRemove_clicked()
{
    userAction |= OVPN_CERT_REMOVE;
    if (ui->lstTPac->count() > 0)  {
        ui->lstTPac->clear();
        ui->cmdRemove->setEnabled(false);
    }
}

void ManageVPN::on_cmdAdd_clicked()
{
    userAction |= OVPN_CERT_ADD;
    if (ui->lstUsb->count() > 0)  {
        QString szText = ui->lstUsb->currentItem()->text();
        // Add to TPAC list if needed
        if (ui->lstTPac->count() == 0)  {
            ui->lstTPac->addItem(szText);
        }
        ui->lstUsb->clear();
        ui->cmdAdd->setEnabled(false);
    }
}
