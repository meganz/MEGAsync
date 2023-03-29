#ifndef ACCOUNTDETAILSDIALOG_H
#define ACCOUNTDETAILSDIALOG_H

#include "Utilities.h"
#include "control/Preferences.h"
#include "HighDpiResize.h"
#include "qml/QmlDialogWrapper.h"

#include <QDialog>
#include <QQuickView>
#include <QQmlComponent>

class AccountDetailsDialog : public QMLComponent, public IStorageObserver
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
    QString test();
public slots:
    void cppSlot1public();

signals:
    void testSignal(int value);

protected:
    void changeEvent(QEvent *event) override;

private:
//    HighDpiResize mHighDpiResize;
//    QQuickView* mUi;

//    Ui::AccountDetailsDialog* mUi;
};

#endif // ACCOUNTDETAILSDIALOG_H
