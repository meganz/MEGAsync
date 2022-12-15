#include "StalledIssuesCaseHeaders.h"

#include <Utilities.h>
#include <Preferences.h>
#include <MegaApplication.h>

#include <StalledIssuesModel.h>
#include <StalledIssue.h>

#ifdef _WIN32
    #include "minwindef.h"
#elif defined Q_OS_MACOS
    #include "sys/syslimits.h"
#elif defined Q_OS_LINUX
    #include "limits.h"
#endif

StalledIssueHeaderCase::StalledIssueHeaderCase(StalledIssueHeader *header)
    :mStalledIssueHeader(header), QObject(header)
{
    connect(mStalledIssueHeader, &StalledIssueHeader::refreshCaseUi, this, &StalledIssueHeaderCase::onRefreshCaseUi);
}

QPointer<StalledIssueHeader> StalledIssueHeaderCase::getStalledIssueHeader()
{
    return mStalledIssueHeader;
}


//Local folder not scannable
DefaultHeader::DefaultHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DefaultHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Error detected with"));
    mStalledIssueHeader->addFileName();
    mStalledIssueHeader->setTitleDescriptionText(tr("Reason not found."));
}

//Local folder not scannable
FileIssueHeader::FileIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void FileIssueHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Can´t sync"));
    mStalledIssueHeader->addFileName();
    if(mStalledIssueHeader->getData().consultData()->hasFiles() > 0)
    {
        mStalledIssueHeader->setTitleDescriptionText(tr("A single file had an issue that needs a user decision to solve"));
    }
    else if(mStalledIssueHeader->getData().consultData()->hasFolders() > 0)
    {
        mStalledIssueHeader->setTitleDescriptionText(tr("A single folder had an issue that needs a user decision to solve."));
    }
}

//Local folder not scannable
MoveOrRenameCannotOccurHeader::MoveOrRenameCannotOccurHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void MoveOrRenameCannotOccurHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Cannot move or rename"));
    mStalledIssueHeader->addFileName();
    if (mStalledIssueHeader->getData().consultData()->mDetectedMEGASide)
    {
        mStalledIssueHeader->setTitleDescriptionText(tr("A move or rename was detected in MEGA, but could not be replicated in the local filesystem."));
    }
    else
    {
        mStalledIssueHeader->setTitleDescriptionText(tr("A move or rename was detected in the local filesystem, but could not be replicated in MEGA."));
    }
}

//Delete or Move Waiting onScanning
DeleteOrMoveWaitingOnScanningHeader::DeleteOrMoveWaitingOnScanningHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DeleteOrMoveWaitingOnScanningHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Can´t find"));
    mStalledIssueHeader->addFileName();
    mStalledIssueHeader->setTitleDescriptionText(tr("Waiting to finish scan to see if the file was moved or deleted."));
}

//Local folder not scannable
DeleteWaitingOnMovesHeader::DeleteWaitingOnMovesHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DeleteWaitingOnMovesHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Waiting to move"));
    mStalledIssueHeader->addFileName();
    mStalledIssueHeader->setTitleDescriptionText(tr("Waiting for other processes to complete."));
}

//Upsync needs target folder
UploadIssueHeader::UploadIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void UploadIssueHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Can´t upload"));
    mStalledIssueHeader->addFileName();
    mStalledIssueHeader->setRightTitleText(tr("to the selected location"));
    mStalledIssueHeader->setTitleDescriptionText(tr("Cannot reach the destination folder."));
}

//Downsync needs target folder
DownloadIssueHeader::DownloadIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void DownloadIssueHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Can´t download"));
    mStalledIssueHeader->addFileName();
    mStalledIssueHeader->setRightTitleText(tr("to the selected location"));
    mStalledIssueHeader->setTitleDescriptionText(tr("Cannot reach the destination folder."));
}

//Create folder failed
CannotCreateFolderHeader::CannotCreateFolderHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void CannotCreateFolderHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Cannot create"));
    mStalledIssueHeader->addFileName();
    mStalledIssueHeader->setTitleDescriptionText(tr("Filesystem error preventing folder access."));
}

//Create folder failed
CannotPerformDeletionHeader::CannotPerformDeletionHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void CannotPerformDeletionHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Cannot perform deletion"));
    mStalledIssueHeader->addFileName();
    mStalledIssueHeader->setTitleDescriptionText(tr("Filesystem error preventing folder access."));
}

//SyncItemExceedsSupoortedTreeDepth
SyncItemExceedsSupoortedTreeDepthHeader::SyncItemExceedsSupoortedTreeDepthHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void SyncItemExceedsSupoortedTreeDepthHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Unable to sync"));
    mStalledIssueHeader->addFileName();
    mStalledIssueHeader->setTitleDescriptionText(tr("Target is too deep on your folder structure.\nPlease move it to a location that is less than 64 folders deep."));
}

////MoveTargetNameTooLongHeader
//CreateFolderNameTooLongHeader::CreateFolderNameTooLongHeader(StalledIssueHeader* header)
//    : StalledIssueHeaderCase(header)
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
FolderMatchedAgainstFileHeader::FolderMatchedAgainstFileHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void FolderMatchedAgainstFileHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Cannot sync"));
    mStalledIssueHeader->addFileName();
    mStalledIssueHeader->setTitleDescriptionText(tr("Cannot sync folders against files."));
}

LocalAndRemotePreviouslyUnsyncedDifferHeader::LocalAndRemotePreviouslyUnsyncedDifferHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void LocalAndRemotePreviouslyUnsyncedDifferHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Can´t sync"));
    mStalledIssueHeader->addFileName();
    mStalledIssueHeader->setTitleDescriptionText(tr("This file has conflicting copies"));
}

//Local and remote previously synced differ
LocalAndRemoteChangedSinceLastSyncedStateHeader::LocalAndRemoteChangedSinceLastSyncedStateHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void LocalAndRemoteChangedSinceLastSyncedStateHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Can´t sync"));
    mStalledIssueHeader->addFileName();
    mStalledIssueHeader->setTitleDescriptionText(tr("This file has been changed both in MEGA and locally since it it was last synced."));
}

//Name Conflicts
NameConflictsHeader::NameConflictsHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void NameConflictsHeader::onRefreshCaseUi()
{
    mStalledIssueHeader->setLeftTitleText(tr("Name Conflicts:"));

    if(mStalledIssueHeader->getData().consultData()->hasFiles() > 0 && mStalledIssueHeader->getData().consultData()->hasFolders() > 0)
    {
        mStalledIssueHeader->setTitleDescriptionText(tr("These files and folders contain multiple names on one side, that would all become the same single name on the other side of the sync."
                                   "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
    }
    else if(mStalledIssueHeader->getData().consultData()->hasFiles() > 0)
    {
        mStalledIssueHeader->setTitleDescriptionText(tr("These files contain multiple names on one side, that would all become the same single name on the other side of the sync."
                                   "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
    }
    else if(mStalledIssueHeader->getData().consultData()->hasFolders() > 0)
    {
        mStalledIssueHeader->setTitleDescriptionText(tr("These folders contain multiple names on one side, that would all become the same single name on the other side of the sync."
                                   "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
    }
}
