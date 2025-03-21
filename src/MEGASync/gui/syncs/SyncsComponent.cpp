#include "SyncsComponent.h"

#include "AddExclusionRule.h"
#include "ChooseFolder.h"
#include "DialogOpener.h"
#include "Syncs.h"
#include "SyncsData.h"
#include "SyncsQmlDialog.h"

static bool qmlRegistrationDone = false;

SyncsComponent::SyncsComponent(QObject* parent):
    QMLComponent(parent),
    mRemoteFolder(QString()),
    mSyncOrigin(SyncInfo::SyncOrigin::MAIN_APP_ORIGIN),
    mSyncs(std::make_unique<Syncs>())
{
    registerQmlModules();

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsComponentAccess"),
                                                   this);
    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsDataAccess"),
                                                   mSyncs->getSyncsData());
}

QUrl SyncsComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/syncs/SyncsDialog.qml"));
}

void SyncsComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("SyncsComponents", 1, 0);
        qmlRegisterType<SyncsQmlDialog>("SyncsComponents", 1, 0, "SyncsQmlDialog");
        qmlRegisterType<ChooseRemoteFolder>("SyncsComponents", 1, 0, "ChooseRemoteFolder");
        qmlRegisterType<ChooseLocalFolder>("SyncsComponents", 1, 0, "ChooseLocalFolder");

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

void SyncsComponent::addSync(SyncInfo::SyncOrigin origin,
                             const QString& local,
                             const QString& remote)
{
    mSyncs->addSync(origin, local, remote);
}

bool SyncsComponent::checkLocalSync(const QString& path)
{
    return mSyncs->checkLocalSync(path);
}

bool SyncsComponent::checkRemoteSync(const QString& path)
{
    return mSyncs->checkRemoteSync(path);
}

void SyncsComponent::clearRemoteError()
{
    mSyncs->clearRemoteError();
}

void SyncsComponent::clearLocalError()
{
    mSyncs->clearLocalError();
}

QString SyncsComponent::getInitialLocalFolder()
{
    ChooseLocalFolder localFolderChooser;
    auto syncsData = mSyncs->getSyncsData();

    QString defaultFolder = localFolderChooser.getDefaultFolder(syncsData->getDefaultMegaFolder());

    if (mSyncOrigin != SyncInfo::ONBOARDING_ORIGIN && !mRemoteFolder.isEmpty())
    {
        defaultFolder.clear();
    }

    if (!checkLocalSync(defaultFolder))
    {
        defaultFolder.clear();
        clearLocalError();
    }

    return defaultFolder;
}

QString SyncsComponent::getInitialRemoteFolder()
{
    auto syncsData = mSyncs->getSyncsData();

    QString defaultFolder = syncsData->getDefaultMegaPath();

    if (mSyncOrigin != SyncInfo::ONBOARDING_ORIGIN && !mRemoteFolder.isEmpty())
    {
        defaultFolder = mRemoteFolder;
    }

    if (!checkRemoteSync(defaultFolder))
    {
        defaultFolder.clear();
        clearRemoteError();
    }

    return defaultFolder;
}

void SyncsComponent::chooseRemoteFolderButtonClicked()
{
    clearRemoteError();
}

void SyncsComponent::chooseLocalFolderButtonClicked()
{
    clearLocalError();
}

bool SyncsComponent::getComesFromOnboarding() const
{
    return mSyncOrigin == SyncInfo::ONBOARDING_ORIGIN;
}
