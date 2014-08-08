#ifndef ACCOUNTDETAILSDIALOG_H
#define ACCOUNTDETAILSDIALOG_H

#include <QDialog>

#include "sdk/megaapi.h"

namespace Ui {
class AccountDetailsDialog;
}

class AccountDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AccountDetailsDialog(MegaApi *megaApi, QWidget *parent = 0);
    ~AccountDetailsDialog();
    void refresh();

private slots:
    void on_bRefresh_clicked();

private:
    Ui::AccountDetailsDialog *ui;
    MegaApi *megaApi;
};

#endif // ACCOUNTDETAILSDIALOG_H
