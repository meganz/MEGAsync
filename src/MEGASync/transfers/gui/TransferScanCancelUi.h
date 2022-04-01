#ifndef TransferScanCancelUi_H
#define TransferScanCancelUi_H

#include <QStackedWidget>

#include "CancelConfirmWidget.h"
#include "ScanningWidget.h"

class TransferScanCancelUi : public QObject
{
    Q_OBJECT

public:
    TransferScanCancelUi(QStackedWidget* _container);
    ~TransferScanCancelUi() = default;

    void show();
    void hide();
    bool isActive();

signals:
    void cancelTransfers();

private slots:
    void onCancelClicked();
    void onCancelDismissed();

private:
    QStackedWidget *mContainer = nullptr;
    ScanningWidget* mBlockingWidget = nullptr;
    CancelConfirmWidget* mConfirmWidget = nullptr;
    QWidget* mLastSelectedWidget = nullptr;

    static const char* getControlStyles();
};

#endif // BLOCKINGUI_H
