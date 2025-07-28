#ifndef MEGAPROGRESSCUSTOMDIALOG_H
#define MEGAPROGRESSCUSTOMDIALOG_H

#include <QDialog>

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
    bool event(QEvent* event) override;

private:
    Ui::MegaProgressCustomDialog *ui;

};

#endif // MEGAPROGRESSCUSTOMDIALOG_H
