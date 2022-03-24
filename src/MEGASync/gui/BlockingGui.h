#ifndef BLOCKINGUI_H
#define BLOCKINGUI_H

#include <QStackedWidget>

#include "CancelConfirmWidget.h"
#include "ScanningWidget.h"

class BlockingUi : public QObject
{
    Q_OBJECT

public:
    BlockingUi(QStackedWidget* _container);
    ~BlockingUi();

    void show();
    void hide();
    bool isActive();

signals:
    void cancelTransfers();

private slots:
    void onCancelClicked();
    void onCancelConfirmed();
    void onCancelDismissed();

private:
    QStackedWidget *mContainer = nullptr;
    ScanningWidget* mBlockingWidget = nullptr;
    CancelConfirmWidget* mConfirmWidget = nullptr;
    QWidget* mLastSelectedWidget = nullptr;

    static const char* getControlStyles();
};

#endif // BLOCKINGUI_H
