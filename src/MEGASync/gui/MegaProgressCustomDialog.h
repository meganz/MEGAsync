#ifndef MEGAPROGRESSCUSTOMDIALOG_H
#define MEGAPROGRESSCUSTOMDIALOG_H

#include <QDialog>
#include "HighDpiResize.h"

namespace Ui {
class MegaProgressCustomDialog;
}

class MegaProgressCustomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MegaProgressCustomDialog(QWidget *parent = 0, int minimum = 0, int maximum = 0);
    ~MegaProgressCustomDialog();

protected:
    void reject();
    void closeEvent(QCloseEvent * event);
    void changeEvent(QEvent * event);


private:
    Ui::MegaProgressCustomDialog *ui;
    HighDpiResize highDpiResize;

};

#endif // MEGAPROGRESSCUSTOMDIALOG_H
