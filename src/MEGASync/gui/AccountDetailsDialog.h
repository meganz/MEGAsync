#ifndef ACCOUNTDETAILSDIALOG_H
#define ACCOUNTDETAILSDIALOG_H

#include "Utilities.h"
#include "control/Preferences.h"
#include "HighDpiResize.h"
#include "QMLDialogWrapper.h"

#include <QDialog>
#include <QQuickView>
#include <QQmlComponent>

class AccountDetailsDialog : public QMLComponentWrapper, public IStorageObserver
{
    Q_OBJECT

public:
    AccountDetailsDialog(QObject* parent = 0);
    ~AccountDetailsDialog();
    void refresh();
    void updateStorageElements() override;

//private slots:
//    void cppSlot();
    QUrl getQmlUrl() override;
    QString contextName() override;
public slots:
    void cppSlot1public();

signals:
    void testSignal(int value);

private:
//    HighDpiResize mHighDpiResize;
//    QQuickView* mUi;

//    Ui::AccountDetailsDialog* mUi;
};

#endif // ACCOUNTDETAILSDIALOG_H
