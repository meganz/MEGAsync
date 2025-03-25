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
    mSyncs->setSyncOrigin(origin);
}

void SyncsComponent::setRemoteFolder(const QString& remoteFolder)
{
    mSyncs->setRemoteFolder(remoteFolder);
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

void SyncsComponent::chooseRemoteFolderButtonClicked()
{
    mSyncs->clearRemoteError();
}

void SyncsComponent::chooseLocalFolderButtonClicked()
{
    mSyncs->clearLocalError();
}

void SyncsComponent::setSyncCandidateLocalFolder(const QString& path)
{
    mLocalFolderSyncCandidate = path;
}

void SyncsComponent::setSyncCandidateRemoteFolder(const QString& path)
{
    mRemoteFolderSyncCandidate = path;
}

void SyncsComponent::syncButtonClicked()
{
    mSyncs->addSync(mLocalFolderSyncCandidate, mRemoteFolderSyncCandidate);
}
