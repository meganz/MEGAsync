#include "Backups.h"

#include "BackupsQmlDialog.h"
#include "BackupsModel.h"

#include "MegaApplication.h"

static bool qmlRegistrationDone = false;

Backups::Backups(QObject *parent)
    : QMLComponent(parent)
{
    registerQmlModules();
}

QUrl Backups::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/backups/BackupsDialog.qml"));
}

QString Backups::contextName()
{
    return QString::fromUtf8("backupsAccess");
}

void Backups::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("Backups", 1, 0);
        qmlRegisterType<BackupsQmlDialog>("BackupsQmlDialog", 1, 0, "BackupsQmlDialog");
        qmlRegisterType<BackupsProxyModel>("BackupsProxyModel", 1, 0, "BackupsProxyModel");
        qmlRegisterUncreatableType<BackupsModel>("BackupsModel", 1, 0, "BackupErrorCode",
                                                 QString::fromUtf8("Cannot register BackupsModel::BackupErrorCode in QML"));
        qmlRegistrationDone = true;
    }
}

void Backups::openBackupsTabInPreferences() const
{
    MegaSyncApp->openSettings(SettingsDialog::BACKUP_TAB);
}
