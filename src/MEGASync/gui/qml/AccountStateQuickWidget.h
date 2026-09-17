#ifndef ACCOUNTSTATEQUICKWIDGET_H
#define ACCOUNTSTATEQUICKWIDGET_H

#include "MegaQuickWidget.h"

class AccountState;

// Legacy Settings dialog host of AccountStateSurface.qml. The state lives in
// AccountState (SNC-6795); this widget only creates the QML view and exposes
// the state to it as `accountStateAccess`.
class AccountStateQuickWidget: public MegaQuickWidget
{
    Q_OBJECT

public:
    explicit AccountStateQuickWidget(QWidget* parent = nullptr);

    AccountState* accountState() const;

private:
    AccountState* mAccountState;
};

#endif // ACCOUNTSTATEQUICKWIDGET_H
