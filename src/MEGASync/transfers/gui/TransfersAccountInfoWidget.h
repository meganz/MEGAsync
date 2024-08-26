#ifndef TRANSFERS_ACCOUNT_INFO_WIDGET_H
#define TRANSFERS_ACCOUNT_INFO_WIDGET_H

#include "Utilities.h"

#include <QWidget>

namespace Ui
{
class TransfersAccountInfoWidget;
}

class TransfersAccountInfoWidget: public QWidget, public IAccountObserver
{
    Q_OBJECT

public:
    explicit TransfersAccountInfoWidget(QWidget* parent = nullptr);
    ~TransfersAccountInfoWidget();

    void updateAccountElements() override;

protected:
    void changeEvent(QEvent* event) override;

private:
    Ui::TransfersAccountInfoWidget* mUi;
};

#endif // TRANSFERS_ACCOUNT_INFO_WIDGET_H
