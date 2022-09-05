#include "StalledIssuesCaseHeaders.h"

#include <Utilities.h>
#include <Preferences.h>
#include <MegaApplication.h>

#include <StalledIssuesModel.h>
#include <StalledIssue.h>

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
    setLeftTitleText(tr("Can´t sync"));
    addFileName();
    if(getData().consultData()->hasFiles() > 0)
    {
        setTitleDescriptionText(tr("A single file had an issue that needs a user decision to solve"));
    }
    else if(getData().consultData()->hasFolders() > 0)
    {
        setTitleDescriptionText(tr("A single folder had an issue that needs a user decision to solve."));
    }
}

//Local folder not scannable
MoveOrRenameCannotOccurHeader::MoveOrRenameCannotOccurHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{}

void MoveOrRenameCannotOccurHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Cannot move or rename"));
    addFileName();
    if (getData().consultData()->mDetectedMEGASide)
    {
        setTitleDescriptionText(tr("A move or rename was detected in MEGA, but could not be replicated in the local filesystem."));
    }
    else
    {
        setTitleDescriptionText(tr("A move or rename was detected in the local filesystem, but could not be replicated in MEGA."));
    }
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
}

void SyncItemExceedsSupoortedTreeDepthHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Unable to sync"));
    addFileName();
    setTitleDescriptionText(tr("Target is too deep on your folder structure.\nPlease move it to a location that is less than 64 folders deep."));
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

//Name Conflicts
NameConflictsHeader::NameConflictsHeader(QWidget *parent)
    : StalledIssueHeader(parent)
{

}

void NameConflictsHeader::refreshCaseUi()
{
    setLeftTitleText(tr("Name Conflicts:"));

    if(getData().consultData()->hasFiles() > 0 && getData().consultData()->hasFolders() > 0)
    {
        setTitleDescriptionText(tr("These files and folders contain multiple names on one side, that would all become the same single name on the other side of the sync."
                                   "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
    }
    else if(getData().consultData()->hasFiles() > 0)
    {
        setTitleDescriptionText(tr("These files contain multiple names on one side, that would all become the same single name on the other side of the sync."
                                   "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
    }
    else if(getData().consultData()->hasFolders() > 0)
    {
        setTitleDescriptionText(tr("These folders contain multiple names on one side, that would all become the same single name on the other side of the sync."
                                   "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
    }
}

