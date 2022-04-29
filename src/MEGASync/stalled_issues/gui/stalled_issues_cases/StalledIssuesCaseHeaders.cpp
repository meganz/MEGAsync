#include "StalledIssuesCaseHeaders.h"

#include <Utilities.h>
#include <Preferences.h>
#include <MegaApplication.h>

#include <QDebug>

#ifdef _WIN32
    #include "minwindef.h"
#elif Q_OS_MACOS
    #include "sys/syslimits.h"
#elif Q_OS_LINUX
    #include "limits.h"
#endif

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

    ui->actionButton->show();
    ui->actionButton->setText(tr("Ignore"));
}

void SpecialFilesNotSupportedHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Cannot access <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("filesystem error preventing file access: Special files not supported"));
}

void SpecialFilesNotSupportedHeader::on_actionButton_clicked()
{
    ignoreFile();
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
{
    ui->actionButton->show();
    ui->actionButton->setText(tr("Ignore"));
}

void SyncItemExceedsSupoortedTreeDepthHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Unable to sync <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("Target is too deep on your folder structure. Please move it to a location that is less\nthan 64 folders deep."));
}

void SyncItemExceedsSupoortedTreeDepthHeader::on_actionButton_clicked()
{
    ignoreFile();
}

//MoveTargetNameTooLongHeader
MoveTargetNameTooLongHeader::MoveTargetNameTooLongHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void MoveTargetNameTooLongHeader::refreshCaseUi()
{
    auto maxCharacters(0);

#ifdef Q_OS_MACX
    maxCharacters = NAME_MAX;
#elif defined(_WIN32)
    maxCharacters = MAX_PATH;
#elif defined (Q_OS_LINUX)
    maxCharacters = NAME_MAX;
#endif

    ui->errorTitleText->setText(tr("Unable to sync <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("File name too long. Your Operating System only supports file\nnames up to %1 characters.").arg(QString::number(maxCharacters)));
}

//Matched Against Unidentified Item
MatchedAgainstUnidentifiedItemHeader::MatchedAgainstUnidentifiedItemHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{
    ui->actionButton->show();
    ui->actionButton->setText(tr("Ignore"));
}

void MatchedAgainstUnidentifiedItemHeader::refreshCaseUi()
{
    ui->errorTitleText->setText(tr("Cannot access <b>%1</b>").arg(getData().getFileName()));
    ui->errorDescriptionText->setText(tr("cannot identify item"));
}

void MatchedAgainstUnidentifiedItemHeader::on_actionButton_clicked()
{
    ignoreFile();
}


