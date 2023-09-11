#ifndef IGNORESEDITINGDIALOG_H
#define IGNORESEDITINGDIALOG_H

#include <QDialog>

namespace Ui {
class IgnoresEditingDialog;
}

class IgnoresEditingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IgnoresEditingDialog(QWidget *parent = nullptr);
    ~IgnoresEditingDialog();

private:
    Ui::IgnoresEditingDialog *ui;
};

#endif // IGNORESEDITINGDIALOG_H
