#include "SyncsComponent.h"

#include "AddExclusionRule.h"
#include "DialogOpener.h"
#include "SyncsCandidatesController.h"
#include "SyncsQmlDialog.h"

static bool qmlRegistrationDone = false;

SyncsComponent::SyncsComponent(QObject* parent):
    QMLComponent(parent),
    mSyncsCandidates(std::make_unique<SyncsCandidatesController>())
{
    registerQmlModules();

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsComponentAccess"),
                                                   this);

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsDataAccess"),
                                                   mSyncsCandidates->getSyncsData());

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsCandidatesModel"),
                                                   mSyncsCandidates->getSyncsCandidadtesModel());

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

void SyncsComponent::confirmSyncCandidateButtonClicked()
{
    mSyncsCandidates->confirmSyncCandidates();
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
    mSyncsCandidates->updateDefaultFolders();
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
    mSyncsCandidates->setSyncOrigin(origin);
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
    mSyncsCandidates->clearRemoteError();
}

void SyncsComponent::clearLocalFolderErrorHint()
{
    mSyncsCandidates->clearLocalError();
}

void SyncsComponent::syncButtonClicked(const QString& localFolder, const QString& megaFolder)
{
    mSyncsCandidates->addSync(localFolder, megaFolder);
}

void SyncsComponent::addSyncCandidadeButtonClicked(const QString& localFolder,
                                                   const QString& megaFolder)
{
    mSyncsCandidates->addSyncCandidate(localFolder, megaFolder);
}

void SyncsComponent::editSyncCandidadeButtonClicked(const QString& localFolder,
                                                    const QString& megaFolder,
                                                    const QString& originalLocalFolder,
                                                    const QString& originalMegaFolder)
{
    mSyncsCandidates->editSyncCandidate(localFolder,
                                        megaFolder,
                                        originalLocalFolder,
                                        originalMegaFolder);
}

void SyncsComponent::removeSyncCandidadeButtonClicked(const QString& localFolder,
                                                      const QString& megaFolder)
{
    mSyncsCandidates->removeSyncCandidate(localFolder, megaFolder);

    updateDefaultFolders();
}

void SyncsComponent::closeDialogButtonClicked()
{
    mSyncsCandidates->clearRemoteError();
    mSyncsCandidates->clearLocalError();
}
