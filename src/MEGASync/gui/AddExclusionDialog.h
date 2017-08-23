#ifndef ADDEXCLUSIONDIALOG_H
#define ADDEXCLUSIONDIALOG_H

#include <QDialog>

namespace Ui {
class AddExclusionDialog;
}

class AddExclusionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddExclusionDialog(QWidget *parent = 0);
    ~AddExclusionDialog();

private:
    Ui::AddExclusionDialog *ui;
};

#endif // ADDEXCLUSIONDIALOG_H
