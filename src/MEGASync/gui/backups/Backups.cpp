#include "Backups.h"

#include "AddExclusionRule.h"
#include "BackupCandidatesController.h"
#include "DialogOpener.h"
#include "MegaApplication.h"

static bool qmlRegistrationDone = false;

Backups::Backups(QObject* parent):
    QMLComponent(parent),
    mComesFromSettings(false),
    mBackupCandidatesController(std::make_shared<BackupCandidatesController>()),
    mBackupsModel(std::make_shared<BackupsModel>(mBackupCandidatesController)),
    mBackupsProxyModel(std::make_shared<BackupsProxyModel>(mBackupsModel))
{
    registerQmlModules();
    mBackupCandidatesController->init();

    connect(mBackupCandidatesController.get(),
            &BackupCandidatesController::backupsCreationFinished,
            this,
            &Backups::backupsCreationFinished);
}

QUrl Backups::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/backups/BackupsDialog.qml"));
}

QString Backups::contextName()
{
    return QString::fromUtf8("backupsAccess");
}

void Backups::registerQmlModules()
{
    if (!qmlRegistrationDone)
    {
        qmlRegisterModule("Backups", 1, 0);
        qmlRegisterType<BackupsProxyModel>("BackupsProxyModel", 1, 0, "BackupsProxyModel");
        qmlRegisterType<BackupCandidates>("BackupCandidates", 1, 0, "BackupCandidates");
    }
}

void Backups::openDeviceCentre() const
{
    MegaSyncApp->openDeviceCentre();
}

bool Backups::getComesFromSettings() const
{
    return mComesFromSettings;
}

std::shared_ptr<BackupCandidates> Backups::getBackupCandidates() const
{
    return mBackupCandidatesController->getBackupCandidates();
}

int Backups::getGlobalError() const
{
    return mBackupCandidatesController->getBackupCandidates()->getGlobalError();
}

void Backups::setComesFromSettings(bool value)
{
    mComesFromSettings = value;
}

void Backups::openExclusionsDialog() const
{
    if(auto dialog = DialogOpener::findDialog<QmlDialogWrapper<Backups>>())
    {
        auto folderPaths = mBackupCandidatesController->getSelectedCandidates();

        QWidget* parentWidget = static_cast<QWidget*>(dialog->getDialog().data());
        QPointer<QmlDialogWrapper<AddExclusionRule>> exclusions = new QmlDialogWrapper<AddExclusionRule>(parentWidget, folderPaths);
        DialogOpener::showDialog(exclusions);
    }
}

void Backups::confirmFoldersMoveToSelect()
{
    mBackupsProxyModel->setSelectedFilterEnabled(false);
}

void Backups::selectFolderMoveToConfirm()
{
    mBackupCandidatesController->calculateFolderSizes();
    mBackupsProxyModel->setSelectedFilterEnabled(true);
    mBackupCandidatesController->check();
}

void Backups::insertFolder(const QString& path)
{
    mBackupCandidatesController->insert(path);
}

int Backups::rename(const QString& folder, const QString& newName)
{
    return mBackupCandidatesController->rename(folder, newName);
}

void Backups::remove(const QString& folder)
{
    mBackupCandidatesController->remove(folder);
}

void Backups::change(const QString& folder, const QString& newFolder)
{
    mBackupCandidatesController->change(folder, newFolder);
}

void Backups::selectAllFolders(Qt::CheckState state, bool fromModel)
{
    mBackupCandidatesController->setCheckAllState(state, fromModel);
}

void Backups::createBackups(int syncOrigin)
{
    mBackupCandidatesController->createBackups(syncOrigin);
}
