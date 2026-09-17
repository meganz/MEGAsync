#include "BackupSettingsQuickWidget.h"

#include "BackupSettingsActions.h"
#include "BackupSettingsModel.h"
#include "QmlManager.h"

#include <QQmlEngine>

BackupSettingsQuickWidget::BackupSettingsQuickWidget(QWidget* parent):
    SettingsQuickWidgetBase(parent)
{
    auto* actions = new BackupSettingsActions(this, this);
    setActions(actions);

    qmlRegisterType<BackupSettingsModel>("BackupSettingsModel", 1, 0, "BackupSettingsModel");

    QmlManager::instance()->setRootContextProperty(QStringLiteral("backupSettingsModel"),
                                                   actions->model());
    QmlManager::instance()->setRootContextProperty(QStringLiteral("backupSettingsAccess"), actions);

    setSource(QString::fromUtf8("qrc:/settings/BackupSettings.qml"));
}
