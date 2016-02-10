#ifndef DOWNLOADFROMMEGADIALOG_H
#define DOWNLOADFROMMEGADIALOG_H

#include <QDialog>

namespace Ui {
class DownloadFromMegaDialog;
}

class DownloadFromMegaDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DownloadFromMegaDialog(QString defaultPath = QString::fromUtf8(""), QWidget *parent = 0);
    ~DownloadFromMegaDialog();
    bool isDefaultDownloadOption();
    void setDefaultDownloadOption(bool);
    QString getPath();

private slots:
    void on_bChange_clicked();
    void on_bOK_clicked();

protected:
    void changeEvent(QEvent * event);

private:
    Ui::DownloadFromMegaDialog *ui;
};

#endif // DOWNLOADFROMMEGADIALOG_H
