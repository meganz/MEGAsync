#include "Backups.h"

#include "BackupsQmlDialog.h"
#include "BackupsModel.h"
#include "AddExclusionRule.h"

#include "DialogOpener.h"
#include "MegaApplication.h"

static bool qmlRegistrationDone = false;

Backups::Backups(QObject *parent)
    : QMLComponent(parent)
    , mComesFromSettings(false)
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

bool Backups::getComesFromSettings() const
{
    return mComesFromSettings;
}

void Backups::setComesFromSettings(bool value)
{
    mComesFromSettings = value;
}

void Backups::openExclusionsDialog(const QStringList& folderPaths) const
{
    if(auto dialog = DialogOpener::findDialog<QmlDialogWrapper<Backups>>())
    {
        QWidget* parentWidget = static_cast<QWidget*>(dialog->getDialog().data());
        QPointer<QmlDialogWrapper<AddExclusionRule>> exclusions = new QmlDialogWrapper<AddExclusionRule>(parentWidget, folderPaths);
        DialogOpener::showDialog(exclusions);
    }
}
