#include "BackupCandidatesComponent.h"

#include "AddExclusionRule.h"
#include "BackupCandidatesController.h"
#include "BackupCandidatesModel.h"
#include "DialogOpener.h"
#include "MegaApplication.h"

static bool qmlRegistrationDone = false;

BackupCandidatesComponent::BackupCandidatesComponent(QObject* parent):
    QMLComponent(parent),
    mComesFromSettings(false),
    mBackupCandidatesController(std::make_shared<BackupCandidatesController>()),
    mBackupsProxyModel(new BackupCandidatesProxyModel(mBackupCandidatesController))
{
    registerQmlModules();

    mBackupCandidatesController->initWithDefaultDirectories();
    // Just in case it is used from the Onboarding Dialog
    QmlManager::instance()->setRootContextProperty(this);

    connect(mBackupCandidatesController.get(),
            &BackupCandidatesController::backupsCreationFinished,
            this,
            &BackupCandidatesComponent::onBackupsCreationFinished);
}

BackupCandidatesComponent::~BackupCandidatesComponent()
{
    mBackupsProxyModel->deleteLater();
}

QUrl BackupCandidatesComponent::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/backups/BackupsDialog.qml"));
}

QString BackupCandidatesComponent::contextName()
{
    return QString::fromUtf8("backupsComponentAccess");
}

void BackupCandidatesComponent::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterType<BackupCandidatesComponent>("BackupsComponent", 1, 0, "BackupsComponent");
        qmlRegisterType<BackupCandidatesProxyModel>("BackupCandidatesProxyModel",
                                                    1,
                                                    0,
                                                    "BackupCandidatesProxyModel");
        qmlRegisterType<BackupCandidates>("BackupCandidates", 1, 0, "BackupCandidates");
        qmlRegistrationDone = true;
    }
}

void BackupCandidatesComponent::openDeviceCentre() const
{
    MegaSyncApp->openDeviceCentre();
}

bool BackupCandidatesComponent::getComesFromSettings() const
{
    return mComesFromSettings;
}

BackupCandidates* BackupCandidatesComponent::getData()
{
    return mBackupCandidatesController->getBackupCandidates().get();
}

void BackupCandidatesComponent::onBackupsCreationFinished(bool success)
{
    emit backupsCreationFinished(success);
    if (success)
    {
        mBackupsProxyModel->setSelectedFilterEnabled(false);
    }
}

void BackupCandidatesComponent::setComesFromSettings(bool value)
{
    mComesFromSettings = value;
}

void BackupCandidatesComponent::openExclusionsDialog() const
{
    if (auto dialog = DialogOpener::findDialog<QmlDialogWrapper<BackupCandidatesComponent>>())
    {
        auto folderPaths = mBackupCandidatesController->getSelectedCandidates();

        QWidget* parentWidget = static_cast<QWidget*>(dialog->getDialog().data());
        QPointer<QmlDialogWrapper<AddExclusionRule>> exclusions =
            new QmlDialogWrapper<AddExclusionRule>(parentWidget, folderPaths);
        DialogOpener::showDialog(exclusions);
    }
}

void BackupCandidatesComponent::confirmFoldersMoveToSelect()
{
    mBackupsProxyModel->setSelectedFilterEnabled(false);
}

void BackupCandidatesComponent::selectFolderMoveToConfirm()
{
    mBackupCandidatesController->calculateFolderSizes();
    mBackupsProxyModel->setSelectedFilterEnabled(true);
    mBackupCandidatesController->refreshBackupCandidatesErrors();
}

void BackupCandidatesComponent::insertFolder(const QString& path)
{
    auto row = mBackupCandidatesController->insert(path);
    emit insertFolderAdded(row);
}

int BackupCandidatesComponent::rename(const QString& folder, const QString& newName)
{
    return mBackupCandidatesController->rename(folder, newName);
}

void BackupCandidatesComponent::remove(const QString& folder)
{
    mBackupCandidatesController->remove(folder);
}

void BackupCandidatesComponent::change(const QString& folder, const QString& newFolder)
{
    mBackupCandidatesController->change(folder, newFolder);
}

void BackupCandidatesComponent::selectAllFolders(Qt::CheckState state, bool fromModel)
{
    mBackupCandidatesController->setCheckAllState(state, fromModel);
}

void BackupCandidatesComponent::createBackups(int syncOrigin)
{
    mBackupCandidatesController->createBackups(syncOrigin);
}
