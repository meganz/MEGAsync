#include "SettingsQuickWidgetBase.h"

#include "SyncSettingsActionsBase.h"
#include "SyncSettingsModelBase.h"

#include <QApplication>
#include <QMouseEvent>
#include <QQmlEngine>

SettingsQuickWidgetBase::SettingsQuickWidgetBase(QWidget* parent):
    MegaQuickWidget(parent)
{
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Expose the shared status enum (SyncSettingsModelBase::State) to QML under a
    // tab-neutral name so the shared SettingsListDelegate can reference it without
    // depending on either concrete model type.
    qmlRegisterUncreatableType<SyncSettingsModelBase>(
        "SettingsModel",
        1,
        0,
        "SettingsModel",
        QStringLiteral("SettingsModel is an abstract base exposed only for its enums"));
}

SyncSettingsActionsBase* SettingsQuickWidgetBase::actions() const
{
    return mActions;
}

void SettingsQuickWidgetBase::setActions(SyncSettingsActionsBase* actions)
{
    Q_ASSERT(!mActions);
    Q_ASSERT(actions && actions->parent() == this);
    mActions = actions;
    connect(mActions,
            &SyncSettingsActionsBase::contextMenuOpenChanged,
            this,
            &SettingsQuickWidgetBase::onContextMenuOpenChanged);
}

void SettingsQuickWidgetBase::onContextMenuOpenChanged(bool open)
{
    if (open)
        qApp->installEventFilter(this);
    else
        qApp->removeEventFilter(this);
}

bool SettingsQuickWidgetBase::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress && mActions)
    {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (!rect().contains(mapFromGlobal(mouseEvent->globalPos())))
        {
            mActions->requestCloseContextMenu();
        }
    }
    return MegaQuickWidget::eventFilter(watched, event);
}
