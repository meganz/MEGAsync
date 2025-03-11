#include "SyncsComponent.h"

#include "AddExclusionRule.h"
#include "ChooseFolder.h"
#include "DialogOpener.h"
#include "Syncs.h"
#include "SyncsQmlDialog.h"

static bool qmlRegistrationDone = false;

SyncsComponent::SyncsComponent(QObject* parent):
    QMLComponent(parent),
    mRemoteFolder(QString()),
    mSyncOrigin(SyncInfo::SyncOrigin::MAIN_APP_ORIGIN)
{
    registerQmlModules();
}

QUrl SyncsComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/syncs/SyncsDialog.qml"));
}

void SyncsComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("Syncs", 1, 0);
        qmlRegisterType<SyncsQmlDialog>("SyncsQmlDialog", 1, 0, "SyncsQmlDialog");
        qmlRegisterType<Syncs>("Syncs", 1, 0, "Syncs");
        qmlRegisterType<ChooseRemoteFolder>("ChooseRemoteFolder", 1, 0, "ChooseRemoteFolder");
        qmlRegisterUncreatableType<Syncs>(
            "Syncs",
            1,
            0,
            "SyncStatusCode",
            QString::fromUtf8("Cannot register Syncs::SyncStatusCode in QML"));
        qmlRegistrationDone = true;
    }
}

void SyncsComponent::openSyncsTabInPreferences() const
{
    MegaSyncApp->openSettings(SettingsDialog::SYNCS_TAB);
}

void SyncsComponent::setSyncOrigin(SyncInfo::SyncOrigin origin)
{
    if (mSyncOrigin != origin)
    {
        mSyncOrigin = origin;
    }
}

SyncInfo::SyncOrigin SyncsComponent::getSyncOrigin() const
{
    return mSyncOrigin;
}

void SyncsComponent::setRemoteFolder(const QString& remoteFolder)
{
    mRemoteFolder = remoteFolder;
    emit remoteFolderChanged();
}

QString SyncsComponent::getRemoteFolder() const
{
    return mRemoteFolder;
}

bool SyncsComponent::getComesFromSettings() const
{
    return mComesFromSettings;
}

void SyncsComponent::setComesFromSettings(bool value)
{
    mComesFromSettings = value;
}

void SyncsComponent::openExclusionsDialog(const QString& folder) const
{
    if (auto dialog = DialogOpener::findDialog<QmlDialogWrapper<SyncsComponent>>())
    {
        QWidget* parentWidget = static_cast<QWidget*>(dialog->getDialog().data());
        QPointer<QmlDialogWrapper<AddExclusionRule>> exclusions =
            new QmlDialogWrapper<AddExclusionRule>(parentWidget, QStringList() << folder);
        DialogOpener::showDialog(exclusions);
    }
}
