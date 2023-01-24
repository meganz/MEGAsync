#ifndef ACCOUNTDETAILSDIALOG_H
#define ACCOUNTDETAILSDIALOG_H

#include "Utilities.h"
#include "control/Preferences.h"
#include "HighDpiResize.h"

#include <QDialog>
#include <QQuickView>

class AccountDetailsDialog : public QDialog, public IStorageObserver
{
    Q_OBJECT

public:
    explicit AccountDetailsDialog(QWidget* parent = 0);
    ~AccountDetailsDialog();
    void refresh();
    void updateStorageElements();
private slots:
    void cppSlot();

private:
    HighDpiResize mHighDpiResize;
    QQuickView* mUi;
};

#endif // ACCOUNTDETAILSDIALOG_H
