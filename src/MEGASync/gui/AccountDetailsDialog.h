#ifndef ACCOUNTDETAILSDIALOG_H
#define ACCOUNTDETAILSDIALOG_H

#include "Utilities.h"
#include "control/Preferences.h"
#include "HighDpiResize.h"

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

private:
    Ui::AccountDetailsDialog* mUi;
    HighDpiResize mHighDpiResize;
};

#endif // ACCOUNTDETAILSDIALOG_H
