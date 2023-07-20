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
#include <LocalOrRemoteUserMustChooseStalledIssue.h>

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
    header->setText(tr("Error detected with <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Reason not found."));
}

//Local folder not scannable
FileIssueHeader::FileIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void FileIssueHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setText(tr("Can´t sync <b>%1</b>").arg(header->displayFileName()));
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
    header->setText(tr("Cannot move or rename <b>%1</b>").arg(header->displayFileName()));
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
    header->setText(tr("Can´t find <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Waiting to finish scan to see if the file was moved or deleted."));
}

//Local folder not scannable
DeleteWaitingOnMovesHeader::DeleteWaitingOnMovesHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DeleteWaitingOnMovesHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setText(tr("Waiting to move <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Waiting for other processes to complete."));
}

//Upsync needs target folder
UploadIssueHeader::UploadIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void UploadIssueHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setText(tr("Can´t upload <b>%1</b> to the selected location").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Cannot reach the destination folder."));
}

//Downsync needs target folder
DownloadIssueHeader::DownloadIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void DownloadIssueHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setText(tr("Can´t download <b>%1</b> to the selected location").arg(header->displayFileName(true)));
    header->setTitleDescriptionText(tr("A failure occurred either downloading the file, or moving the downloaded temporary file to its final name and location."));
}

//Create folder failed
CannotCreateFolderHeader::CannotCreateFolderHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void CannotCreateFolderHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setText(tr("Cannot create <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Filesystem error preventing folder access."));
}

//Create folder failed
CannotPerformDeletionHeader::CannotPerformDeletionHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void CannotPerformDeletionHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setText(tr("Cannot perform deletion <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Filesystem error preventing folder access."));
}

//SyncItemExceedsSupoortedTreeDepth
SyncItemExceedsSupoortedTreeDepthHeader::SyncItemExceedsSupoortedTreeDepthHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void SyncItemExceedsSupoortedTreeDepthHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setText(tr("Unable to sync <b>%1</b>").arg(header->displayFileName()));
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
//                               "\nnames up to <b>%1</b> characters.").arg(QString::number(maxCharacters)));
//}

//SymlinksNotSupported
FolderMatchedAgainstFileHeader::FolderMatchedAgainstFileHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void FolderMatchedAgainstFileHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setText(tr("Can´t sync <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Cannot sync folders against files."));
}

LocalAndRemotePreviouslyUnsyncedDifferHeader::LocalAndRemotePreviouslyUnsyncedDifferHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void LocalAndRemotePreviouslyUnsyncedDifferHeader::onActionButtonClicked(StalledIssueHeader *header)
{
    LocalAndRemoteActionButtonClicked::actionClicked(header);
}

void LocalAndRemotePreviouslyUnsyncedDifferHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setText(tr("Can´t sync <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("This file has conflicting copies"));

    if(auto conflict = header->getData().convert<LocalOrRemoteUserMustChooseStalledIssue>())
    {
        if(conflict->isSolvable() && !conflict->isSolved())
        {
            header->showAction(tr("Solve"));
        }
        else
        {
            header->hideAction();
        }

        if(conflict->isSolved())
        {
            header->showSolvedMessage();
        }
    }
}

//Local and remote previously synced differ
LocalAndRemoteChangedSinceLastSyncedStateHeader::LocalAndRemoteChangedSinceLastSyncedStateHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void LocalAndRemoteChangedSinceLastSyncedStateHeader::onActionButtonClicked(StalledIssueHeader *header)
{
    LocalAndRemoteActionButtonClicked::actionClicked(header);
}

void LocalAndRemoteChangedSinceLastSyncedStateHeader::refreshCaseUi(StalledIssueHeader* header)
{
    header->setText(tr("Can´t sync <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("This file has been changed both in MEGA and locally since it it was last synced."));

    if(auto conflict = header->getData().convert<LocalOrRemoteUserMustChooseStalledIssue>())
    {
        if(conflict->isSolvable() && !conflict->isSolved())
        {
            header->showAction(tr("Solve"));
        }
        else
        {
            header->hideAction();
        }

        if(conflict->isSolved())
        {
            header->showSolvedMessage();
        }
    }
}

void LocalAndRemoteActionButtonClicked::actionClicked(StalledIssueHeader *header)
{
    if(auto conflict = header->getData().convert<LocalOrRemoteUserMustChooseStalledIssue>())
    {
        auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.textFormat = Qt::RichText;
        msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::No, tr("Cancel"));

        auto reasons(QList<mega::MegaSyncStall::SyncStallReason>() << mega::MegaSyncStall::NamesWouldClashWhenSynced);
        auto selection = dialog->getDialog()->getSelection(reasons);

        if(selection.size() <= 1)
        {
            auto allSimilarIssues = MegaSyncApp->getStalledIssuesModel()->getIssuesByReason(reasons);

            if(allSimilarIssues.size() != selection.size())
            {
                msgInfo.buttons |= QMessageBox::Yes;
                textsByButton.insert(QMessageBox::Yes, tr("Apply to all similar issues (%1)").arg(allSimilarIssues.size()));
                textsByButton.insert(QMessageBox::Ok, tr("Apply only to this issue"));
            }
            else
            {
                textsByButton.insert(QMessageBox::Ok, tr("Ok"));
            }
        }
        else
        {
            textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issues (%1)").arg(selection.size()));
        }

        msgInfo.buttonsText = textsByButton;
        msgInfo.text = tr("Are you sure you want to solve the issue?");
        msgInfo.informativeText = tr("This action will choose the local side if they are duplicated");

        msgInfo.finishFunc = [selection, header, conflict](QMessageBox* msgBox)
        {
            if(msgBox->result() == QDialogButtonBox::Ok)
            {
                MegaSyncApp->getStalledIssuesModel()->solveSideConflict(selection);
            }
            else if(msgBox->result() == QDialogButtonBox::Yes)
            {
                MegaSyncApp->getStalledIssuesModel()->solveSideConflict(QModelIndexList());
            }
        };

        QMegaMessageBox::warning(msgInfo);
    }
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
        QString text(tr("Name Conflicts: <b>%1</b>"));

        auto cloudData = nameConflict->getNameConflictCloudData();
        if(cloudData.firstNameConflict())
        {
            text = text.arg(cloudData.firstNameConflict()->getConflictedName());
        }
        else
        {
            auto localConflictedNames = nameConflict->getNameConflictLocalData();
            if(!localConflictedNames.isEmpty())
            {
                text = text.arg(localConflictedNames.first()->getConflictedName());
            }
        }

        header->setText(text);

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
            header->showMultipleAction(tr("Solve options"), QStringList() << tr("Remove duplicates and rename the rest") << tr("Rename all items"));
        }
        else
        {
            header->showSolvedMessage();
            header->hideMultipleAction();
        }
    }
}


void NameConflictsHeader::onMultipleActionButtonOptionSelected(StalledIssueHeader* header, int index)
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

        auto reasons(QList<mega::MegaSyncStall::SyncStallReason>() << mega::MegaSyncStall::NamesWouldClashWhenSynced);
        auto selection = dialog->getDialog()->getSelection(reasons);

        if(selection.size() <= 1)
        {
            auto allSimilarIssues = MegaSyncApp->getStalledIssuesModel()->getIssuesByReason(reasons);

            if(allSimilarIssues.size() != selection.size())
            {
                msgInfo.buttons |= QMessageBox::Yes;
                textsByButton.insert(QMessageBox::Yes, tr("Apply to all similar issues (%1)").arg(allSimilarIssues.size()));
                textsByButton.insert(QMessageBox::Ok, tr("Apply only to this issue"));
            }
            else
            {
                textsByButton.insert(QMessageBox::Ok, tr("Ok"));
            }
        }
        else
        {
            textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issues (%1)").arg(selection.size()));
        }

        msgInfo.buttonsText = textsByButton;
        msgInfo.text = tr("Are you sure you want to solve the issue?");
        msgInfo.informativeText = index == 0 ? tr("This action will delete the duplicate files and rename the rest of names.")
                                             : tr("This action will rename the conflicted items.");

        msgInfo.finishFunc = [this, index, selection, header, nameConflict](QMessageBox* msgBox)
        {
            if(msgBox->result() == QDialogButtonBox::Ok)
            {
                MegaSyncApp->getStalledIssuesModel()->solveNameConflictIssues(selection, index);
            }
            else if(msgBox->result() == QDialogButtonBox::Yes)
            {
                MegaSyncApp->getStalledIssuesModel()->solveNameConflictIssues(QModelIndexList(), index);
            }
        };

        QMegaMessageBox::warning(msgInfo);
    }
}
