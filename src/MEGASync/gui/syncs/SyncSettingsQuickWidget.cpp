#include "SyncSettingsQuickWidget.h"

#include "QmlManager.h"
#include "SyncSettingsActions.h"
#include "SyncSettingsModel.h"

#include <QQmlEngine>

SyncSettingsQuickWidget::SyncSettingsQuickWidget(QWidget* parent):
    SettingsQuickWidgetBase(parent)
{
    auto* actions = new SyncSettingsActions(this, this);
    setActions(actions);

    qmlRegisterType<SyncSettingsModel>("SyncSettingsModel", 1, 0, "SyncSettingsModel");

    QmlManager::instance()->setRootContextProperty(QStringLiteral("syncSettingsModel"),
                                                   actions->model());
    // Distinct context-property name: the Syncs and Backups widgets share one QML
    // engine root context, so they must not use the same name. The shared QML
    // components receive the actions object as an explicit "settingsAccess"
    // property instead.
    QmlManager::instance()->setRootContextProperty(QStringLiteral("syncSettingsAccess"), actions);

    setSource(QString::fromUtf8("qrc:/settings/SyncSettings.qml"));
}
