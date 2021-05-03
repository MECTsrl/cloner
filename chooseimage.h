#ifndef CHOOSEIMAGE_H
#define CHOOSEIMAGE_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QListWidgetItem>
#include <QTimerEvent>

namespace Ui {
class ChooseImage;
}

class ChooseImage : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseImage(QWidget *parent = 0);
    QString     getSelectedImage(int &nRetentiveMode, int &nHmiMode, int &nLogMode);
    ~ChooseImage();

protected:
    void    timerEvent(QTimerEvent *event);

private slots:

    void on_cmdCancel_clicked();
    void on_cmdOk_clicked();
    void on_lstImmagini_currentRowChanged(int currentRow);

private:
    int fillImagesList();

    Ui::ChooseImage     *ui;
    QString             selectedImage;
    int                 retentiveMode;

};

#endif // CHOOSEIMAGE_H
