#include "SyncsComponent.h"

#include "AddExclusionRule.h"
#include "DialogOpener.h"
#include "Syncs.h"
#include "SyncsQmlDialog.h"

static bool qmlRegistrationDone = false;

SyncsComponent::SyncsComponent(QObject* parent):
    QMLComponent(parent),
    mSyncs(std::make_unique<Syncs>())
{
    registerQmlModules();

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsComponentAccess"),
                                                   this);

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsDataAccess"),
                                                   mSyncs->getSyncsData());

    connect(&mRemoteFolderChooser,
            &ChooseRemoteFolder::folderChosen,
            this,
            &SyncsComponent::onRemoteFolderChosen);

    connect(&mLocalFolderChooser,
            &ChooseLocalFolder::folderChosen,
            this,
            &SyncsComponent::onLocalFolderChosen);
}

void SyncsComponent::closingOnboardingDialog()
{
    if (mEnteredOnSyncCreation)
    {
        mEnteredOnSyncCreation = false;

        auto model = SyncInfo::instance();
        if (model != nullptr && !model->hasSyncs())
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::USER_ABORTS_ONBOARDING_SYNC_CREATION);
        }
    }
}

void SyncsComponent::enteredOnSync()
{
    mEnteredOnSyncCreation = true;
}

void SyncsComponent::exclusionsButtonClicked(const QString& currentPath)
{
    openExclusionsDialog(currentPath);
}

void SyncsComponent::chooseRemoteFolderButtonClicked()
{
    mRemoteFolderChooser.openFolderSelector();
}

void SyncsComponent::chooseLocalFolderButtonClicked(const QString& currentPath)
{
    mLocalFolderChooser.openFolderSelector();
}

void SyncsComponent::updateDefaultFolders()
{
    mSyncs->updateDefaultFolders();
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

        qmlRegistrationDone = true;
    }
}

void SyncsComponent::viewSyncsInSettingsButtonClicked()
{
    MegaSyncApp->openSettings(SettingsDialog::SYNCS_TAB);
}

void SyncsComponent::setSyncOrigin(SyncInfo::SyncOrigin origin)
{
    mSyncs->setSyncOrigin(origin);
}

void SyncsComponent::setRemoteFolder(const QString& remoteFolder)
{
    if (remoteFolder.isEmpty())
        return;

    emit remoteFolderChosen(remoteFolder);
}

void SyncsComponent::setLocalFolder(const QString& localFolder)
{
    if (localFolder.isEmpty())
        return;

    emit localFolderChosen(localFolder);
}

void SyncsComponent::onRemoteFolderChosen(QString remotePath)
{
    setRemoteFolder(remotePath);
    clearRemoteFolderErrorHint();
}

void SyncsComponent::onLocalFolderChosen(QString localPath)
{
    setLocalFolder(localPath);
    clearLocalFolderErrorHint();
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

void SyncsComponent::clearRemoteFolderErrorHint()
{
    mSyncs->clearRemoteError();
}

void SyncsComponent::clearLocalFolderErrorHint()
{
    mSyncs->clearLocalError();
}

void SyncsComponent::syncButtonClicked(const QString& localFolder, const QString& megaFolder)
{
    mSyncs->addSync(localFolder, megaFolder);
}

void SyncsComponent::closeDialogButtonClicked()
{
    mSyncs->clearRemoteError();
    mSyncs->clearLocalError();
}
