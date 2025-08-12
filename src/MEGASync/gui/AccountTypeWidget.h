#ifndef ACCOUNT_TYPE_WIDGET_H
#define ACCOUNT_TYPE_WIDGET_H

#include "Utilities.h"

#include <QWidget>

namespace Ui
{
class AccountTypeWidget;
}

class AccountTypeWidget: public QWidget, public IAccountObserver
{
    Q_OBJECT

public:
    explicit AccountTypeWidget(QWidget* parent = nullptr);
    ~AccountTypeWidget();

    void updateAccountElements() override;

protected:
    bool event(QEvent* event) override;

private:
    Ui::AccountTypeWidget* mUi;

    void updateAccountText();
};

#endif // ACCOUNT_TYPE_WIDGET_H
