#include "BackupsComponent.h"

#include "AddExclusionRule.h"
#include "BackupCandidatesController.h"
#include "BackupCandidatesModel.h"
#include "DialogOpener.h"
#include "MegaApplication.h"

static bool qmlRegistrationDone = false;

BackupsComponent::BackupsComponent(QObject* parent):
    QMLComponent(parent),
    mComesFromSettings(false),
    mBackupCandidatesController(std::make_shared<BackupCandidatesController>()),
    mBackupsProxyModel(new BackupCandidatesProxyModel(mBackupCandidatesController))
{
    registerQmlModules();

    mBackupCandidatesController->init();
    // Just in case it is used from the Onboarding Dialog
    QmlManager::instance()->setRootContextProperty(this);

    connect(mBackupCandidatesController.get(),
            &BackupCandidatesController::backupsCreationFinished,
            this,
            &BackupsComponent::onBackupsCreationFinished);
}

BackupsComponent::~BackupsComponent()
{
    mBackupsProxyModel->deleteLater();
}

QUrl BackupsComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/backups/BackupsDialog.qml"));
}

QString BackupsComponent::contextName()
{
    return QString::fromUtf8("backupsComponentAccess");
}

void BackupsComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("BackupsComponent", 1, 0);
        qmlRegisterType<BackupCandidatesProxyModel>("BackupCandidatesProxyModel",
                                                    1,
                                                    0,
                                                    "BackupCandidatesProxyModel");
        qmlRegisterType<BackupCandidates>("BackupCandidates", 1, 0, "BackupCandidates");
    }
}

void BackupsComponent::openDeviceCentre() const
{
    MegaSyncApp->openDeviceCentre();
}

bool BackupsComponent::getComesFromSettings() const
{
    return mComesFromSettings;
}

void BackupsComponent::onBackupsCreationFinished(bool success)
{
    emit backupsCreationFinished(success);
    if (success)
    {
        mBackupsProxyModel->setSelectedFilterEnabled(false);
    }
}

void BackupsComponent::setComesFromSettings(bool value)
{
    mComesFromSettings = value;
}

void BackupsComponent::openExclusionsDialog() const
{
    if (auto dialog = DialogOpener::findDialog<QmlDialogWrapper<BackupsComponent>>())
    {
        auto folderPaths = mBackupCandidatesController->getSelectedCandidates();

        QWidget* parentWidget = static_cast<QWidget*>(dialog->getDialog().data());
        QPointer<QmlDialogWrapper<AddExclusionRule>> exclusions =
            new QmlDialogWrapper<AddExclusionRule>(parentWidget, folderPaths);
        DialogOpener::showDialog(exclusions);
    }
}

void BackupsComponent::confirmFoldersMoveToSelect()
{
    mBackupsProxyModel->setSelectedFilterEnabled(false);
}

void BackupsComponent::selectFolderMoveToConfirm()
{
    mBackupCandidatesController->calculateFolderSizes();
    mBackupsProxyModel->setSelectedFilterEnabled(true);
    mBackupCandidatesController->check();
}

void BackupsComponent::insertFolder(const QString& path)
{
    auto row = mBackupCandidatesController->insert(path);
    emit insertFolderAdded(row);
}

int BackupsComponent::rename(const QString& folder, const QString& newName)
{
    return mBackupCandidatesController->rename(folder, newName);
}

void BackupsComponent::remove(const QString& folder)
{
    mBackupCandidatesController->remove(folder);
}

void BackupsComponent::change(const QString& folder, const QString& newFolder)
{
    mBackupCandidatesController->change(folder, newFolder);
}

void BackupsComponent::selectAllFolders(Qt::CheckState state, bool fromModel)
{
    mBackupCandidatesController->setCheckAllState(state, fromModel);
}

void BackupsComponent::createBackups(int syncOrigin)
{
    mBackupCandidatesController->createBackups(syncOrigin);
}
