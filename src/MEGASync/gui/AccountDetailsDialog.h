#ifndef ACCOUNTDETAILSDIALOG_H
#define ACCOUNTDETAILSDIALOG_H

#include <QDialog>

#include "megaapi.h"
#include "Preferences.h"
#include "HighDpiResize.h"
#include "Utilities.h"

namespace Ui {
class AccountDetailsDialog;
}

class AccountDetailsDialog : public QDialog, public IObserver
{
    Q_OBJECT

public:
    explicit AccountDetailsDialog(mega::MegaApi *megaApi, QWidget *parent = 0);
    ~AccountDetailsDialog();
    void refresh(Preferences *details);
    void update();

private:
    Ui::AccountDetailsDialog *ui;
    mega::MegaApi *megaApi;
    HighDpiResize highDpiResize;

private:
    void usageDataAvailable(bool isAvailable);

};

#endif // ACCOUNTDETAILSDIALOG_H
