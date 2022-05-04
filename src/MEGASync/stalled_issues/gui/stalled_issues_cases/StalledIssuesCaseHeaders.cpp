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
    setLeftTitleText(tr("Can´t sync"));
    addFileName();
    setTitleDescriptionText(tr("This file has conflicting copies"));
}

//Local and remote previously synced differ
LocalAndRemoteChangedSinceLastSyncedStateHeader::LocalAndRemoteChangedSinceLastSyncedStateHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{

}

void LocalAndRemoteChangedSinceLastSyncedStateHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Can´t sync"));
    addFileName();
    setTitleDescriptionText(tr("This file has changed since it it was last synced."));
}

//Special Files not supported
SpecialFilesNotSupportedHeader::SpecialFilesNotSupportedHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{
    showAction(tr("Ignore"));
}

void SpecialFilesNotSupportedHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Cannot access"));
    addFileName();
    setTitleDescriptionText(tr("filesystem error preventing file access: Special files not supported."));
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
    setLeftTitleText(tr("Can´t move"));
    addFileName();
    setRightTitleText(tr("to the selected location"));
    setTitleDescriptionText(tr("Cannot reach the destination folder."));
}

//Folder contains locked files
FolderContainsLockedFilesHeader::FolderContainsLockedFilesHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void FolderContainsLockedFilesHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Unable to scan"));
    addFileName();
    setTitleDescriptionText(tr("Some contents of this folder are locked. Please see if any of its files are opened by\n"
                                         "another program. Close the program and try again."));
}

//Can fingerprint file yet
CantFingerprintFileYetHeader::CantFingerprintFileYetHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void CantFingerprintFileYetHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Unable to sync"));
    addFileName();
    setTitleDescriptionText(tr("File is opened by another program. Close the process and try again."));
}

//Local folder not scannable
MoveOrRenameFailedHeader::MoveOrRenameFailedHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void MoveOrRenameFailedHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Cannot move or rename"));
    addFileName();
    setTitleDescriptionText(tr("Filesystem error preventing file acces."));
}

//Could not moved to local debris
CouldNotMoveToLocalDebrisFolderHeader::CouldNotMoveToLocalDebrisFolderHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void CouldNotMoveToLocalDebrisFolderHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Cannot update"));
    addFileName();
    setTitleDescriptionText(tr("The sync tried to move this file to the debris folder, but the operation failed."));
}

//Create folder failed
CreateFolderFailedHeader::CreateFolderFailedHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void CreateFolderFailedHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Cannot create"));
    addFileName();
    setTitleDescriptionText(tr("Filesystem error preventing folder access."));
}

//Local folder not scannable
DeleteWaitingOnMovesHeader::DeleteWaitingOnMovesHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void DeleteWaitingOnMovesHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Waiting to move"));
    addFileName();
    setTitleDescriptionText(tr("Waiting for other processes to complete."));
}

//ApplyMoveNeedsOtherSideParentFolderToExist
ApplyMoveNeedsOtherSideParentFolderToExistHeader::ApplyMoveNeedsOtherSideParentFolderToExistHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void ApplyMoveNeedsOtherSideParentFolderToExistHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Can´t move"));
    addFileName();
    setRightTitleText(tr("to the selected location"));
    setTitleDescriptionText(tr("Cannot reach the destination folder %1")
                            .arg(getData().isCloud() ? tr("on the cloud") : tr("on your device")));
}

//MoveNeedsDestinationNodeProcessing
MoveNeedsDestinationNodeProcessingHeader::MoveNeedsDestinationNodeProcessingHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void MoveNeedsDestinationNodeProcessingHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Can´t move"));
    addFileName();
    setRightTitleText(tr("to the selected location"));
    setTitleDescriptionText(tr("Cannot reach the destination folder."));
}

//Unable to load ignore file
UnableToLoadIgnoreFileHeader::UnableToLoadIgnoreFileHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void UnableToLoadIgnoreFileHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Unable to load"));
    addFileName();
    setTitleDescriptionText(tr("Cannot read ignore file."));
}

//Delete or Move Waiting onScanning
DeleteOrMoveWaitingOnScanningHeader::DeleteOrMoveWaitingOnScanningHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void DeleteOrMoveWaitingOnScanningHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Can´t find"));
    addFileName();
    setTitleDescriptionText(tr("Waiting to finish scan to see if the file was moved or deleted."));
}


//SyncItemExceedsSupoortedTreeDepth
SyncItemExceedsSupoortedTreeDepthHeader::SyncItemExceedsSupoortedTreeDepthHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{
    showAction(tr("Ignore"));
}

void SyncItemExceedsSupoortedTreeDepthHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Unable to sync"));
    addFileName();
    setTitleDescriptionText(tr("Target is too deep on your folder structure. Please move it to a location that is less"
                                "\nthan 64 folders deep."));
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

    setLeftTitleText(tr("Unable to sync"));
    addFileName();
    setTitleDescriptionText(tr("File name too long. Your Operating System only supports file"
                               "\nnames up to %1 characters.").arg(QString::number(maxCharacters)));
}

//Matched Against Unidentified Item
MatchedAgainstUnidentifiedItemHeader::MatchedAgainstUnidentifiedItemHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{
    showAction(tr("Ignore"));
}

void MatchedAgainstUnidentifiedItemHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Cannot access"));
    addFileName();
    setTitleDescriptionText(tr("Cannot identify item."));
}

void MatchedAgainstUnidentifiedItemHeader::on_actionButton_clicked()
{
    ignoreFile();
}


//Moving download to target
MovingDownloadToTargetHeader::MovingDownloadToTargetHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{
}

void MovingDownloadToTargetHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Waiting to finish downloading"));
    addFileName();
    setTitleDescriptionText(tr("Moving the downloaded file to its final destination failed."));
}


