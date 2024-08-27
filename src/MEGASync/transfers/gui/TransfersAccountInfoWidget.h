#ifndef TRANSFERS_ACCOUNT_INFO_WIDGET_H
#define TRANSFERS_ACCOUNT_INFO_WIDGET_H

#include "Utilities.h"

#include <QWidget>

namespace Ui
{
class TransfersAccountInfoWidget;
}

class TransfersAccountInfoWidget: public QWidget, public IStorageObserver
{
    Q_OBJECT

public:
    explicit TransfersAccountInfoWidget(QWidget* parent = nullptr);
    ~TransfersAccountInfoWidget();

    void updateStorageElements() override;

protected:
    void changeEvent(QEvent* event) override;

private:
    Ui::TransfersAccountInfoWidget* mUi;

    void updateStorageText();
    void updateStorageBar();
    void updateProgressBarStateUntilFull(int percentage);
    void refreshProgressBar();
};

#endif // TRANSFERS_ACCOUNT_INFO_WIDGET_H
