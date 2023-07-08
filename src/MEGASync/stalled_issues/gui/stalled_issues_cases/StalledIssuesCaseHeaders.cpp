#include "StalledIssuesCaseHeaders.h"

#include <Utilities.h>
#include <Preferences.h>
#include <MegaApplication.h>

#include <StalledIssuesModel.h>
#include <StalledIssue.h>
#include <NameConflictStalledIssue.h>
#include <QMegaMessageBox.h>
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>

#ifdef _WIN32
    #include "minwindef.h"
#elif defined Q_OS_MACOS
    #include "sys/syslimits.h"
#elif defined Q_OS_LINUX
    #include "limits.h"
#endif

StalledIssueHeaderCase::StalledIssueHeaderCase(StalledIssueHeader *header)
    :QObject(header)
{
    header->setData(this);
}

//Local folder not scannable
DefaultHeader::DefaultHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DefaultHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Error detected with"));
    header->addFileName();
    header->setTitleDescriptionText(tr("Reason not found."));
}

//Local folder not scannable
FileIssueHeader::FileIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void FileIssueHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Can´t sync"));
    header->addFileName();
    if(header->getData().consultData()->hasFiles() > 0)
    {
        header->setTitleDescriptionText(tr("A single file had an issue that needs a user decision to solve"));
    }
    else if(header->getData().consultData()->hasFolders() > 0)
    {
        header->setTitleDescriptionText(tr("A single folder had an issue that needs a user decision to solve."));
    }
}

//Local folder not scannable
MoveOrRenameCannotOccurHeader::MoveOrRenameCannotOccurHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void MoveOrRenameCannotOccurHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Cannot move or rename"));
    header->addFileName();
    if (header->getData().consultData()->mDetectedMEGASide)
    {
        header->setTitleDescriptionText(tr("A move or rename was detected in MEGA, but could not be replicated in the local filesystem."));
    }
    else
    {
        header->setTitleDescriptionText(tr("A move or rename was detected in the local filesystem, but could not be replicated in MEGA."));
    }
}

//Delete or Move Waiting onScanning
DeleteOrMoveWaitingOnScanningHeader::DeleteOrMoveWaitingOnScanningHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DeleteOrMoveWaitingOnScanningHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Can´t find"));
    header->addFileName();
    header->setTitleDescriptionText(tr("Waiting to finish scan to see if the file was moved or deleted."));
}

//Local folder not scannable
DeleteWaitingOnMovesHeader::DeleteWaitingOnMovesHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DeleteWaitingOnMovesHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Waiting to move"));
    header->addFileName();
    header->setTitleDescriptionText(tr("Waiting for other processes to complete."));
}

//Upsync needs target folder
UploadIssueHeader::UploadIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void UploadIssueHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Can´t upload"));
    header->addFileName(false);
    header->setRightTitleText(tr("to the selected location"));
    header->setTitleDescriptionText(tr("Cannot reach the destination folder."));
}

//Downsync needs target folder
DownloadIssueHeader::DownloadIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void DownloadIssueHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Can´t download"));
    header->addFileName(true);
    header->setRightTitleText(tr("to the selected location"));
    header->setTitleDescriptionText(tr("A failure occurred either downloading the file, or moving the downloaded temporary file to its final name and location."));
}

//Create folder failed
CannotCreateFolderHeader::CannotCreateFolderHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void CannotCreateFolderHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Cannot create"));
    header->addFileName();
    header->setTitleDescriptionText(tr("Filesystem error preventing folder access."));
}

//Create folder failed
CannotPerformDeletionHeader::CannotPerformDeletionHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void CannotPerformDeletionHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Cannot perform deletion"));
    header->addFileName();
    header->setTitleDescriptionText(tr("Filesystem error preventing folder access."));
}

//SyncItemExceedsSupoortedTreeDepth
SyncItemExceedsSupoortedTreeDepthHeader::SyncItemExceedsSupoortedTreeDepthHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void SyncItemExceedsSupoortedTreeDepthHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Unable to sync"));
    header->addFileName();
    header->setTitleDescriptionText(tr("Target is too deep on your folder structure.\nPlease move it to a location that is less than 64 folders deep."));
}

////MoveTargetNameTooLongHeader
//CreateFolderNameTooLongHeader::CreateFolderNameTooLongHeader(StalledIssueHeader* header)
//    : StalledIssueHeaderCase(header)
//{}

//void CreateFolderNameTooLongHeader::refreshCaseUi(StalledIssueHeader* header)
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

void FolderMatchedAgainstFileHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Cannot sync"));
    header->addFileName();
    header->setTitleDescriptionText(tr("Cannot sync folders against files."));
}

LocalAndRemotePreviouslyUnsyncedDifferHeader::LocalAndRemotePreviouslyUnsyncedDifferHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void LocalAndRemotePreviouslyUnsyncedDifferHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Can´t sync"));
    header->addFileName();
    header->setTitleDescriptionText(tr("This file has conflicting copies"));
}

//Local and remote previously synced differ
LocalAndRemoteChangedSinceLastSyncedStateHeader::LocalAndRemoteChangedSinceLastSyncedStateHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void LocalAndRemoteChangedSinceLastSyncedStateHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setLeftTitleText(tr("Can´t sync"));
    header->addFileName();
    header->setTitleDescriptionText(tr("This file has been changed both in MEGA and locally since it it was last synced."));
}

//Name Conflicts
NameConflictsHeader::NameConflictsHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void NameConflictsHeader::refreshCaseUi(StalledIssueHeader* header)
{
    if(auto nameConflict = header->getData().convert<NameConflictedStalledIssue>())
    {
        auto cloudData = nameConflict->getNameConflictCloudData();
        if(cloudData.firstNameConflict())
        {
            header->addFileName(cloudData.firstNameConflict()->mConflictedName);
        }
        else
        {
            auto localConflictedNames = nameConflict->getNameConflictLocalData();
            if(!localConflictedNames.isEmpty())
            {
                header->addFileName(localConflictedNames.first()->mConflictedName);
            }
        }

        header->setLeftTitleText(tr("Name Conflicts:"));

        if(header->getData().consultData()->hasFiles() > 0 && header->getData().consultData()->hasFolders() > 0)
        {
            header->setTitleDescriptionText(tr("These items contain multiple names on one side, that would all become the same single name on the other side."
                                               "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
        }
        else if(header->getData().consultData()->hasFiles() > 0)
        {
            header->setTitleDescriptionText(tr("These files contain multiple names on one side, that would all become the same single name on the other side."
                                               "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
        }
        else if(header->getData().consultData()->hasFolders() > 0)
        {
            header->setTitleDescriptionText(tr("These folders contain multiple names on one side, that would all become the same single name on the other side."
                                               "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
        }

        if(!nameConflict->isSolved())
        {
            header->showAction(tr("Solve"));
        }
        else
        {
            header->hideAction();
        }
    }
}

void NameConflictsHeader::onActionButtonClicked(StalledIssueHeader* header)
{
    if(auto nameConflict = header->getData().convert<NameConflictedStalledIssue>())
    {
        auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.textFormat = Qt::RichText;
        msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::No, tr("Cancel"));

        auto selection = dialog->getDialog()->getSelection(QList<mega::MegaSyncStall::SyncStallReason>() << mega::MegaSyncStall::NamesWouldClashWhenSynced);

        if(selection.size() <= 1)
        {
            msgInfo.buttons |= QMessageBox::Yes;
            textsByButton.insert(QMessageBox::Yes, tr("Apply to all similar issues"));
            textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issue"));
        }
        else
        {
            textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issues (%1)").arg(selection.size()));
        }

        msgInfo.buttonsText = textsByButton;
        msgInfo.text = tr("Are you sure you want to solve the issue?");
        msgInfo.informativeText = tr("This action will delete the duplicate files and rename the rest of names.");

        msgInfo.finishFunc = [this, selection, header, nameConflict](QMessageBox* msgBox)
        {
            if(msgBox->result() == QDialogButtonBox::Ok)
            {
                MegaSyncApp->getStalledIssuesModel()->solveNameConflictIssues(selection);
            }
            else if(msgBox->result() == QDialogButtonBox::Yes)
            {
                MegaSyncApp->getStalledIssuesModel()->solveNameConflictIssues(QModelIndexList());
            }
        };

        QMegaMessageBox::warning(msgInfo);
    }
}
