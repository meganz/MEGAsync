#ifndef ACCOUNTDETAILSDIALOG_H
#define ACCOUNTDETAILSDIALOG_H

#include "Utilities.h"
#include "control/Preferences/Preferences.h"

#include <QDialog>

namespace Ui {
class AccountDetailsDialog;
}

class AccountDetailsDialog : public QDialog, public IStorageObserver
{
    Q_OBJECT

public:
    explicit AccountDetailsDialog(QWidget* parent = 0);
    ~AccountDetailsDialog();
    void refresh();
    void updateStorageElements();

protected:
    void changeEvent(QEvent *event) override;

private:
    Ui::AccountDetailsDialog* mUi;
};

#endif // ACCOUNTDETAILSDIALOG_H
