#ifndef TRANSFERS_ACCOUNT_INFO_WIDGET_H
#define TRANSFERS_ACCOUNT_INFO_WIDGET_H

#include "Utilities.h"

#include <QWidget>

namespace Ui
{
class TransfersAccountInfoWidget;
}

class TransfersAccountInfoWidget: public QWidget, public IStorageObserver, public IAccountObserver
{
    Q_OBJECT

public:
    explicit TransfersAccountInfoWidget(QWidget* parent = nullptr);
    ~TransfersAccountInfoWidget();

    void updateStorageElements() override;
    void updateAccountElements() override;

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void on_bUpgrade_clicked();

private:
    Ui::TransfersAccountInfoWidget* mUi;

    void updateStorageText();
    void updateStorageBar();
    void updateProgressBarStateUntilFull(int percentage);
    void refreshProgressBar();
    void checkUpgradeButtonVisibility();
    void updateUpgradeButtonText();
};

#endif // TRANSFERS_ACCOUNT_INFO_WIDGET_H
