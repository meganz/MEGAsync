#include "SyncsComponent.h"

#include "AddExclusionRule.h"
#include "DialogOpener.h"
#include "Syncs.h"
#include "SyncsData.h"
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

    QmlManager::instance()->setRootContextProperty(QString::fromLatin1("syncsCandidatesModel"),
                                                   mSyncs->getSyncsCandidadtesModel());
}

void SyncsComponent::confirmSyncCandidateButtonClicked()
{
    mSyncs->confirmSyncCandidates();
}

void SyncsComponent::exclusionsButtonClicked()
{
    openExclusionsDialog(mSyncs->getSyncsData()->getLocalFolderCandidate());
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
    mSyncs->setRemoteFolderCandidate(remoteFolder);
}

void SyncsComponent::setLocalFolder(const QString& localFolder)
{
    mSyncs->setLocalFolderCandidate(localFolder);
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

void SyncsComponent::addSyncCandidadeButtonClicked(const QString& localFolder,
                                                   const QString& megaFolder)
{
    mSyncs->addSyncCandidate(localFolder, megaFolder);
}

void SyncsComponent::editSyncCandidadeButtonClicked(const QString& localFolder,
                                                    const QString& megaFolder,
                                                    const QString& originalLocalFolder,
                                                    const QString& originalMegaFolder)
{
    mSyncs->editSyncCandidate(localFolder, megaFolder, originalLocalFolder, originalMegaFolder);
}

void SyncsComponent::removeSyncCandidadeButtonClicked(const QString& localFolder,
                                                      const QString& megaFolder)
{
    mSyncs->removeSyncCandidate(localFolder, megaFolder);
}

void SyncsComponent::closeDialogButtonClicked()
{
    mSyncs->clearRemoteError();
    mSyncs->clearLocalError();
}
