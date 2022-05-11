#include "StalledIssuesCaseHeaders.h"

#include <Utilities.h>
#include <Preferences.h>
#include <MegaApplication.h>

#include <QDebug>

#ifdef _WIN32
    #include "minwindef.h"
#elif defined Q_OS_MACOS
    #include "sys/syslimits.h"
#elif defined Q_OS_LINUX
    #include "limits.h"
#endif

//Local folder not scannable
DefaultHeader::DefaultHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void DefaultHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Error detected with"));
    addFileName();
    setTitleDescriptionText(tr("Reason not found."));
}

//Local folder not scannable
FileIssueHeader::FileIssueHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void FileIssueHeader::refreshCaseUi()
{
    setLeftTitleText(tr("File Issue text"));
    addFileName();
    setTitleDescriptionText(tr("Example text."));
}

//Local folder not scannable
MoveOrRenameCannotOccurHeader::MoveOrRenameCannotOccurHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void MoveOrRenameCannotOccurHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Cannot move or rename"));
    addFileName();
    setTitleDescriptionText(tr("Filesystem error preventing file acces."));
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

//Upsync needs target folder
UploadIssueHeader::UploadIssueHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{
}

void UploadIssueHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Can´t upload"));
    addFileName();
    setRightTitleText(tr("to the selected location"));
    setTitleDescriptionText(tr("Cannot reach the destination folder."));
}

//Downsync needs target folder
DownloadIssueHeader::DownloadIssueHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{
}

void DownloadIssueHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Can´t download"));
    addFileName();
    setRightTitleText(tr("to the selected location"));
    setTitleDescriptionText(tr("Cannot reach the destination folder."));
}

//Create folder failed
CannotCreateFolderHeader::CannotCreateFolderHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void CannotCreateFolderHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Cannot create"));
    addFileName();
    setTitleDescriptionText(tr("Filesystem error preventing folder access."));
}

//Create folder failed
CannotPerformDeletionHeader::CannotPerformDeletionHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void CannotPerformDeletionHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Cannot perform deletion"));
    addFileName();
    setTitleDescriptionText(tr("Filesystem error preventing folder access."));
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

////MoveTargetNameTooLongHeader
//CreateFolderNameTooLongHeader::CreateFolderNameTooLongHeader(QWidget *parent)
//    : StalledIssueHeader(parent)
//{}

//void CreateFolderNameTooLongHeader::refreshCaseUi()
//{
//    auto maxCharacters(0);

//#ifdef Q_OS_MACX
//    maxCharacters = NAME_MAX;
//#elif defined(_WIN32)
//    maxCharacters = MAX_PATH;
//#elif defined (Q_OS_LINUX)
//    maxCharacters = NAME_MAX;
//#endif

//    setLeftTitleText(tr("Unable to sync"));
//    addFileName();
//    setTitleDescriptionText(tr("Folder name too long. Your Operating System only supports folder"
//                               "\nnames up to %1 characters.").arg(QString::number(maxCharacters)));
//}

//SymlinksNotSupported
FolderMatchedAgainstFileHeader::FolderMatchedAgainstFileHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{
}

void FolderMatchedAgainstFileHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Cannot sync"));
    addFileName();
    setTitleDescriptionText(tr("Cannot sync folders against files."));
}

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
