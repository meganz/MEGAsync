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
    QString textValue();

private slots:
    void on_bOk_clicked();
    void on_bChoose_clicked();

protected:
    void changeEvent(QEvent * event);

private:
    Ui::AddExclusionDialog *ui;
};

#endif // ADDEXCLUSIONDIALOG_H
