#include "SyncsComponent.h"

#include "qml/ChooseFolder.h"
#include "onboarding/Syncs.h"

#include "MegaApplication.h"

static bool qmlRegistrationDone = false;

SyncsComponent::SyncsComponent(QObject *parent)
    : QMLComponent(parent)
    , mRemoteFolder(QString())
{
    registerQmlModules();
    //connect(SyncInfo::instance(), &SyncInfo::syncRemoved, this, &SyncsComponent::onSyncRemoved);
}

QUrl SyncsComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/syncs/SyncsDialog.qml"));
}

QString SyncsComponent::contextName()
{
    return QString::fromUtf8("syncsComponentAccess");
}

void SyncsComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("Syncs", 1, 0);
        qmlRegisterType<Syncs>("Syncs", 1, 0, "Syncs");
        qmlRegisterType<ChooseRemoteFolder>("ChooseRemoteFolder", 1, 0, "ChooseRemoteFolder");
        qmlRegisterUncreatableType<Syncs>("Syncs", 1, 0, "SyncStatusCode",
                                          QString::fromUtf8("Cannot register Syncs::SyncStatusCode in QML"));
        qmlRegistrationDone = true;
    }
}

void SyncsComponent::openSyncsTabInPreferences() const
{
    MegaSyncApp->openSettings(SettingsDialog::SYNCS_TAB);
}

bool SyncsComponent::getComesFromSettings() const
{
    return mComesFromSettings;
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

void SyncsComponent::setComesFromSettings(bool value)
{
    mComesFromSettings = value;
}

/*
void SyncsComponent::onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings)
{
    Q_UNUSED(syncSettings);
    auto syncInfo = SyncInfo::instance();
    if(syncInfo->getNumSyncedFolders(mega::MegaSync::SyncType::TYPE_TWOWAY) <= 0)
    {
        mSyncStatus = NONE;
    }
    else
    {
        mSyncStatus = FULL;
    }
}
*/
