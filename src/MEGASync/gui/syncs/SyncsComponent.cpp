#include "SyncsComponent.h"

#include "AddExclusionRule.h"
#include "DialogOpener.h"
#include "SyncsCandidates.h"
#include "SyncsData.h"
#include "SyncsQmlDialog.h"

static bool qmlRegistrationDone = false;

SyncsComponent::SyncsComponent(QObject* parent):
    QMLComponent(parent),
    mSyncsCandidates(std::make_unique<SyncsCandidates>())
{
    registerQmlModules();

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsComponentAccess"),
                                                   this);

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsDataAccess"),
                                                   mSyncsCandidates->getSyncsData());

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsCandidatesModel"),
                                                   mSyncsCandidates->getSyncsCandidadtesModel());
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

void SyncsComponent::exclusionsButtonClicked()
{
    openExclusionsDialog(mSyncsCandidates->getSyncsData()->getLocalFolderCandidate());
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
    mSyncsCandidates->setRemoteFolderCandidate(remoteFolder);
}

void SyncsComponent::setLocalFolder(const QString& localFolder)
{
    mSyncsCandidates->setLocalFolderCandidate(localFolder);
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
