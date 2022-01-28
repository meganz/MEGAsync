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

signals:
    void cancelTransfers();

private slots:
    void onCancelClicked();
    void onCancelConfirmed();
    void onCancelDismissed();

private:
    QStackedWidget *container = nullptr;
    ScanningWidget* blockingWidget = nullptr;
    CancelConfirmWidget* confirmWidget = nullptr;
    QWidget* lastSelectedWidget = nullptr;
};

#endif // BLOCKINGUI_H
