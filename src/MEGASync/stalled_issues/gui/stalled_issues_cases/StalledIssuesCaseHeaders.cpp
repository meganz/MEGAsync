#include "StalledIssuesCaseHeaders.h"

#include <Utilities.h>
#include <Preferences.h>
#include <MegaApplication.h>

LocalAndRemotePreviouslyUnsyncedDifferHeader::LocalAndRemotePreviouslyUnsyncedDifferHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{

}

void LocalAndRemotePreviouslyUnsyncedDifferHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Can´t sync <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("This file has conflicting copies"));
}

//Local and remote previously synced differ
LocalAndRemoteChangedSinceLastSyncedStateHeader::LocalAndRemoteChangedSinceLastSyncedStateHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{

}

void LocalAndRemoteChangedSinceLastSyncedStateHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Can´t sync <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("This file has changed since it it was last synced."));
}

//Special Files not supported
SpecialFilesNotSupportedHeader::SpecialFilesNotSupportedHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{
    connect(ui->actionButton, &QPushButton::clicked, this, &SpecialFilesNotSupportedHeader::on_actionButton_clicked);
}

void SpecialFilesNotSupportedHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Cannot access <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("filesystem error preventing file access: Special files not supported"));

    ui->actionButton->show();
    ui->actionButton->setText(tr("Ignore"));
}

void SpecialFilesNotSupportedHeader::on_actionButton_clicked()
{
    auto data = getData().getStalledIssueData();
    if(data)
    {
        Preferences::instance()->setExcludedSyncPaths(QStringList() << data->mIndexPath.path);
        Preferences::instance()->setCrashed(true);

        QStringList exclusionPaths = Preferences::instance()->getExcludedSyncPaths();
        std::vector<std::string> vExclusionPaths;
        for (int i = 0; i < exclusionPaths.size(); i++)
        {
            vExclusionPaths.push_back(exclusionPaths[i].toUtf8().constData());
        }
        MegaSyncApp->getMegaApi()->setLegacyExcludedPaths(&vExclusionPaths);
    }
}

//Local folder not scannable
LocalFolderNotScannableHeader::LocalFolderNotScannableHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void LocalFolderNotScannableHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Can´t move <b>%1</b> to the selected location").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("Cannot reach the destination folder"));
}

//Folder contains locked files
FolderContainsLockedFilesHeader::FolderContainsLockedFilesHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void FolderContainsLockedFilesHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Unable to scan <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("Some contents of this folder are locked. Please see if any of its files are opened by\n"
                                         "another program. Close the program and try again."));
}

//Can fingerprint file yet
CantFingerprintFileYetHeader::CantFingerprintFileYetHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void CantFingerprintFileYetHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Unable to sync <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("File is opened by another program. Close the process and try again."));
}

//Local folder not scannable
MoveOrRenameFailedHeader::MoveOrRenameFailedHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void MoveOrRenameFailedHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Cannot move or rename <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("filesystem error preventing file access"));
}

//Could not moved to local debris
CouldNotMoveToLocalDebrisFolderHeader::CouldNotMoveToLocalDebrisFolderHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void CouldNotMoveToLocalDebrisFolderHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Cannot update <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("The sync tried to move this file to the debris folder, but the operation failed"));
}

//Create folder failed
CreateFolderFailedHeader::CreateFolderFailedHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void CreateFolderFailedHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Cannot create <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("filesystem error preventing folder access"));
}

//Local folder not scannable
DeleteWaitingOnMovesHeader::DeleteWaitingOnMovesHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void DeleteWaitingOnMovesHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("waiting to move <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("waiting for other processes to complete"));
}

//ApplyMoveNeedsOtherSideParentFolderToExist
ApplyMoveNeedsOtherSideParentFolderToExistHeader::ApplyMoveNeedsOtherSideParentFolderToExistHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void ApplyMoveNeedsOtherSideParentFolderToExistHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Can´t move <b>%1</b> to the selected location").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("Cannot reach the destination folder %1").arg(getData().isCloud() ? tr("on the cloud") : tr("on your device")));
}

//Unable to load ignore file
UnableToLoadIgnoreFileHeader::UnableToLoadIgnoreFileHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void UnableToLoadIgnoreFileHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Unable to load <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("cannot read ignore file"));
}

//SyncItemExceedsSupoortedTreeDepth
SyncItemExceedsSupoortedTreeDepthHeader::SyncItemExceedsSupoortedTreeDepthHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void SyncItemExceedsSupoortedTreeDepthHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Unable to sync <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("Target is too deep on your folder structure. Please move it to a location that is less\nthan 64 folders deep."));
}

