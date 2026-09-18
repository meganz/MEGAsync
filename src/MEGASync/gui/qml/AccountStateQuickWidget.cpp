#include "AccountStateQuickWidget.h"

#include "AccountState.h"

#include <QQmlContext>

AccountStateQuickWidget::AccountStateQuickWidget(QWidget* parent):
    MegaQuickWidget(parent),
    mAccountState(new AccountState(this))
{
    rootContext()->setContextProperty(QString::fromLatin1("accountStateAccess"), mAccountState);
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setSource(QString::fromUtf8("qrc:/settings/AccountStateSurface.qml"));
}

AccountState* AccountStateQuickWidget::accountState() const
{
    return mAccountState;
}
