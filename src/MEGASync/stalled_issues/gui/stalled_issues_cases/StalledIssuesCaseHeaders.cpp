#include "StalledIssuesCaseHeaders.h"
#include <QDialogButtonBox>

#include <Utilities.h>
#include "control/Preferences/Preferences.h"
#include <MegaApplication.h>

#include <StalledIssuesModel.h>
#include <StalledIssue.h>
#include <NameConflictStalledIssue.h>
#include <MoveOrRenameCannotOccurIssue.h>
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

//Issue checker
bool HeaderCaseIssueChecker::checkIssue(StalledIssueHeader *header, bool isSingleSelection)
{
    if(isSingleSelection && MegaSyncApp->getStalledIssuesModel()->checkForExternalChanges(header->getCurrentIndex()))
    {
        auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.textFormat = Qt::RichText;
        msgInfo.buttons = QMessageBox::Ok;
        QMap<QMessageBox::StandardButton, QString> buttonsText;
        buttonsText.insert(QMessageBox::Ok, tr("Refresh"));
        msgInfo.buttonsText = buttonsText;
        msgInfo.text = tr("The issue may have been solved externally.\nPlease, refresh the list.");
        msgInfo.finishFunc = [](QPointer<QMessageBox>){
            MegaSyncApp->getStalledIssuesModel()->updateStalledIssues();
        };
        QMegaMessageBox::warning(msgInfo);

        header->updateSizeHint();
        return true;
    }

    return false;
}

//Local folder not scannable
DefaultHeader::DefaultHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DefaultHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Error detected with <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Reason not found."));
}

//Detected Hard Link
SymLinkHeader::SymLinkHeader(StalledIssueHeader *header)
    : StalledIssueHeaderCase(header)
{}

void SymLinkHeader::onMultipleActionButtonOptionSelected(StalledIssueHeader* header, int index)
{
    auto isSymLinkChecker = [](const std::shared_ptr<const StalledIssue> issue){
        return issue->isSymLink();
    };

    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();
    auto selection = dialog->getDialog()->getSelection(isSymLinkChecker);

    if(HeaderCaseIssueChecker::checkIssue(header, selection.size() == 1))
    {
        return;
    }

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::No, tr("Cancel"));

    auto allSimilarIssues = MegaSyncApp->getStalledIssuesModel()->getIssues(isSymLinkChecker);

    if(index == IgnoreType::IgnoreAll)
    {
         textsByButton.insert(QMessageBox::Ok, tr("Ok"));
    }
    else
    {
        if(selection.size() <= 1)
        {
            if(allSimilarIssues.size() != selection.size())
            {
                msgInfo.buttons |= QMessageBox::Yes;
                textsByButton.insert(QMessageBox::Yes, tr("Apply to all similar symlinks (%1)").arg(allSimilarIssues.size()));
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
    }

    msgInfo.buttonsText = textsByButton;

    if(index == IgnoreType::IgnoreAll)
    {
        msgInfo.text = tr("Are you sure you want to ignore all symlinks in all syncs?");
        msgInfo.informativeText = tr("This action will ignore all present and future symlinks in all your syncs.");
    }
    else
    {
        msgInfo.text = tr("Are you sure you want to ignore this symlink?");
        msgInfo.informativeText = tr("This action will ignore this symlink and it will not be synced.");
    }

    msgInfo.finishFunc = [this, index, selection, allSimilarIssues](QMessageBox* msgBox)
    {
        if(msgBox->result() == QDialogButtonBox::Ok)
        {
            if(index == IgnoreType::IgnoreAll)
            {
                MegaSyncApp->getStalledIssuesModel()->ignoreSymLinks();
            }
            else
            {
                MegaSyncApp->getStalledIssuesModel()->ignoreItems(selection, true);
            }
        }
        else if(msgBox->result() == QDialogButtonBox::Yes)
        {
            MegaSyncApp->getStalledIssuesModel()->ignoreItems(allSimilarIssues, true);
        }
    };

    QMegaMessageBox::warning(msgInfo);

}

void SymLinkHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Detected sym link: <b>%1</b>").arg(header->getData().consultData()->consultLocalData()->getNativeFilePath()));
    header->setTitleDescriptionText(QString());
    header->setIsExpandable(false);
}

void SymLinkHeader::refreshCaseActions(StalledIssueHeader *header)
{
    if(!header->getData().consultData()->isSolved())
    {
        QList<StalledIssueHeader::ActionInfo> actions;
        actions << StalledIssueHeader::ActionInfo(tr("Ignore symlink"), IgnoreType::IgnoreThis);
        actions << StalledIssueHeader::ActionInfo(tr("Ignore all symlinks in all syncs"), IgnoreType::IgnoreAll);

        header->showActions(tr("Ignore"), actions);
    }
}

//Cloud Fingerprint missing
CloudFingerprintMissingHeader::CloudFingerprintMissingHeader(StalledIssueHeader *header)
    : StalledIssueHeaderCase(header)
{}

void CloudFingerprintMissingHeader::onMultipleActionButtonOptionSelected(StalledIssueHeader* header, int index)
{
    auto fingerprintMissingChecker = [](const std::shared_ptr<const StalledIssue> issue){
        return issue->missingFingerprint();
    };

    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();
    auto selection = dialog->getDialog()->getSelection(fingerprintMissingChecker);

    if(HeaderCaseIssueChecker::checkIssue(header, selection.size() == 1))
    {
        return;
    }

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::No, tr("Cancel"));

    auto allSimilarIssues = MegaSyncApp->getStalledIssuesModel()->getIssues(fingerprintMissingChecker);

    auto pluralNumber(1);

    if(selection.size() <= 1)
    {
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
        pluralNumber = selection.size();
        textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issues (%1)").arg(selection.size()));
    }

    msgInfo.text = tr("Are you sure you want to solve the issue?", "", pluralNumber);
    msgInfo.informativeText = tr("This action will download the file to a temp location, fix the issue and finally remove it.", "", pluralNumber);
    if(MegaSyncApp->getTransfersModel()->areAllPaused())
    {
        msgInfo.informativeText.append(tr("<br><b>Please, resume your transfers to fix the issue</b></br>", "", pluralNumber));
    }

    msgInfo.buttonsText = textsByButton;

    msgInfo.finishFunc = [this, index, selection, allSimilarIssues](QMessageBox* msgBox)
    {
        if(msgBox->result() == QDialogButtonBox::Ok)
        {
            MegaSyncApp->getStalledIssuesModel()->fixFingerprint(selection);
        }
        else if(msgBox->result() == QDialogButtonBox::Yes)
        {
            MegaSyncApp->getStalledIssuesModel()->fixFingerprint(allSimilarIssues);
        }
    };

    QMegaMessageBox::warning(msgInfo);

}

void CloudFingerprintMissingHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t download <b>%1</b> to the selected location").arg(header->getData().consultData()->consultCloudData()->getNativeFilePath()));
    header->setTitleDescriptionText(tr("File fingerprint missing"));
    header->setIsExpandable(false);
}

void CloudFingerprintMissingHeader::refreshCaseActions(StalledIssueHeader *header)
{
    if(!header->getData().consultData()->isSolved())
    {
        header->showAction(StalledIssueHeader::ActionInfo(tr("Solve"), 0));
    }
}

//Cloud node undecrypted
CloudNodeUndecryptedHeader::CloudNodeUndecryptedHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void CloudNodeUndecryptedHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Clode node undecrypted <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Decryption process could not be completed. Reload your account on <a href=\"http://mega.io/\">MEGA</a> or contact <a href=mailto:support@mega.nz>Support</a>."));
}

//Local folder not scannable
FileIssueHeader::FileIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void FileIssueHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t sync <b>%1</b>").arg(header->displayFileName()));
    if(header->getData().consultData()->filesCount() > 0)
    {
        header->setTitleDescriptionText(tr("A single file had an issue that needs a user decision to solve"));
    }
    else if(header->getData().consultData()->foldersCount() > 0)
    {
        header->setTitleDescriptionText(tr("A single folder had an issue that needs a user decision to solve."));
    }
}

//Local folder not scannable
MoveOrRenameCannotOccurHeader::MoveOrRenameCannotOccurHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void MoveOrRenameCannotOccurHeader::refreshCaseTitles(StalledIssueHeader* header)
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

void MoveOrRenameCannotOccurHeader::refreshCaseActions(StalledIssueHeader *header)
{
    if(header->getData().consultData()->isSolvable())
    {
        header->showAction(StalledIssueHeader::ActionInfo(tr("Solve"), 0));
    }
}

void MoveOrRenameCannotOccurHeader::onMultipleActionButtonOptionSelected(StalledIssueHeader *header, int)
{    
    if(HeaderCaseIssueChecker::checkIssue(header, true))
    {
        return;
    }

    MegaSyncApp->getStalledIssuesModel()->fixMoveOrRenameCannotOccur(header->getCurrentIndex());
}

//Delete or Move Waiting onScanning
DeleteOrMoveWaitingOnScanningHeader::DeleteOrMoveWaitingOnScanningHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DeleteOrMoveWaitingOnScanningHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t find <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Waiting to finish scan to see if the file was moved or deleted."));
}

//Local folder not scannable
DeleteWaitingOnMovesHeader::DeleteWaitingOnMovesHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DeleteWaitingOnMovesHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Waiting to move <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Waiting for other processes to complete."));
}

//Upsync needs target folder
UploadIssueHeader::UploadIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void UploadIssueHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t upload <b>%1</b> to the selected location").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Cannot reach the destination folder."));
}

//Downsync needs target folder
DownloadIssueHeader::DownloadIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void DownloadIssueHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t download <b>%1</b> to the selected location").arg(header->displayFileName(true)));
    header->setTitleDescriptionText(tr("A failure occurred either downloading the file, or moving the downloaded temporary file to its final name and location."));
}

//Create folder failed
CannotCreateFolderHeader::CannotCreateFolderHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void CannotCreateFolderHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Cannot create <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Filesystem error preventing folder access."));
}

//Create folder failed
CannotPerformDeletionHeader::CannotPerformDeletionHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void CannotPerformDeletionHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Cannot perform deletion <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Filesystem error preventing folder access."));
}

//SyncItemExceedsSupoortedTreeDepth
SyncItemExceedsSupoortedTreeDepthHeader::SyncItemExceedsSupoortedTreeDepthHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void SyncItemExceedsSupoortedTreeDepthHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Unable to sync <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Target is too deep on your folder structure.\nPlease move it to a location that is less than 64 folders deep."));
}

////MoveTargetNameTooLongHeader
//CreateFolderNameTooLongHeader::CreateFolderNameTooLongHeader(StalledIssueHeader* header)
//    : StalledIssueHeaderCase(header)
//{}

//void CreateFolderNameTooLongHeader::refreshCaseTitles(StalledIssueHeader* header)
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

void FolderMatchedAgainstFileHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t sync <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Cannot sync folders against files."));
}

LocalAndRemotePreviouslyUnsyncedDifferHeader::LocalAndRemotePreviouslyUnsyncedDifferHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void LocalAndRemotePreviouslyUnsyncedDifferHeader::refreshCaseActions(StalledIssueHeader *header)
{
    if(header->getData().consultData()->isSolved())
    {
        header->showSolvedMessage();
    }
}

void LocalAndRemotePreviouslyUnsyncedDifferHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t sync <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("This file has conflicting copies"));
}

//Local and remote previously synced differ
LocalAndRemoteChangedSinceLastSyncedStateHeader::LocalAndRemoteChangedSinceLastSyncedStateHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void LocalAndRemoteChangedSinceLastSyncedStateHeader::refreshCaseActions(StalledIssueHeader *header)
{
    if(header->getData().consultData()->isSolved())
    {
        header->showSolvedMessage();
    }
}

void LocalAndRemoteChangedSinceLastSyncedStateHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t sync <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("This file has been changed both in MEGA and locally since it it was last synced."));
}

//Name Conflicts
NameConflictsHeader::NameConflictsHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void NameConflictsHeader::refreshCaseActions(StalledIssueHeader *header)
{
    if(auto nameConflict = header->getData().convert<NameConflictedStalledIssue>())
    {
        if(!nameConflict->isSolved())
        {
            QList<StalledIssueHeader::ActionInfo> actions;

            if(header->getData().consultData()->filesCount() > 0)
            {
                if(nameConflict->areAllDuplicatedNodes())
                {
                    actions << StalledIssueHeader::ActionInfo(tr("Remove duplicates"), NameConflictedStalledIssue::RemoveDuplicated);
                }
                else if(nameConflict->hasDuplicatedNodes())
                {
                    NameConflictedStalledIssue::ActionsSelected selection(NameConflictedStalledIssue::RemoveDuplicated | NameConflictedStalledIssue::Rename);
                    QString actionMessage;
                    if(header->getData().consultData()->foldersCount() > 1)
                    {
                        selection |= NameConflictedStalledIssue::MergeFolders;
                        actionMessage = tr("Remove duplicates, merge folders and rename the rest");
                    }
                    else
                    {
                        actionMessage = tr("Remove duplicates and rename the rest");
                    }
                    actions << StalledIssueHeader::ActionInfo(actionMessage, selection);
                }
                else if(header->getData().consultData()->foldersCount() > 1)
                {
                     actions << StalledIssueHeader::ActionInfo(tr("Merge folders and rename the rest"), NameConflictedStalledIssue::Rename | NameConflictedStalledIssue::MergeFolders);
                }

                actions << StalledIssueHeader::ActionInfo(tr("Rename all items"), NameConflictedStalledIssue::Rename);
            }
            else if(header->getData().consultData()->foldersCount() > 1)
            {
                actions << StalledIssueHeader::ActionInfo(tr("Merge folders"), NameConflictedStalledIssue::MergeFolders);
            }

            header->showActions(tr("Solve options"), actions);
        }
    }
}

void NameConflictsHeader::refreshCaseTitles(StalledIssueHeader* header)
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

        if(header->getData().consultData()->filesCount() > 0 && header->getData().consultData()->foldersCount() > 0)
        {
            header->setTitleDescriptionText(tr("These items contain multiple names on one side, that would all become the same single name on the other side."
                                               "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
        }
        else if(header->getData().consultData()->filesCount() > 0)
        {
            header->setTitleDescriptionText(tr("These files contain multiple names on one side, that would all become the same single name on the other side."
                                               "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
        }
        else if(header->getData().consultData()->foldersCount() > 0)
        {
            header->setTitleDescriptionText(tr("These folders contain multiple names on one side, that would all become the same single name on the other side."
                                               "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
        }
    }
}

void NameConflictsHeader::onMultipleActionButtonOptionSelected(StalledIssueHeader* header, int index)
{
    if(auto nameConflict = header->getData().convert<NameConflictedStalledIssue>())
    {
        auto solutionCanBeApplied = [index](const std::shared_ptr<const StalledIssue> issue) -> bool{
            auto result = issue->getReason() == mega::MegaSyncStall::NamesWouldClashWhenSynced;
            if(result)
            {
                if(auto nameIssue = StalledIssue::convert<NameConflictedStalledIssue>(issue))
                {
                    if(index == NameConflictedStalledIssue::RemoveDuplicated)
                    {
                        result = nameIssue->filesCount() > 0 && nameIssue->areAllDuplicatedNodes();
                    }

                    if(result && (index & NameConflictedStalledIssue::RemoveDuplicated &&
                                  index & NameConflictedStalledIssue::Rename))
                    {
                        result = nameIssue->filesCount() > 0 &&
                                 nameIssue->hasDuplicatedNodes() &&
                                 !nameIssue->areAllDuplicatedNodes();
                    }

                    if(result && (index & NameConflictedStalledIssue::MergeFolders))
                    {
                        result = nameIssue->foldersCount() > 0;
                    }
                }
            }
            return result;
        };

        auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();
        auto selection = dialog->getDialog()->getSelection(solutionCanBeApplied);

        if(HeaderCaseIssueChecker::checkIssue(header, selection.size() == 1))
        {
            return;
        }

        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.textFormat = Qt::RichText;
        msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::No, tr("Cancel"));

        auto allSimilarIssues = MegaSyncApp->getStalledIssuesModel()->getIssues(solutionCanBeApplied);

        if(selection.size() <= 1)
        {
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

        if(index == NameConflictedStalledIssue::Rename)
        {
            msgInfo.informativeText = tr("This action will rename the conflicted items (adding a suffix like (1)).");
        }
        else if(index == NameConflictedStalledIssue::MergeFolders)
        {
            msgInfo.informativeText = tr("This action will merge all folders into a single one. We will skip duplicated files\nand rename the files with the same name but different content (adding a suffix like (1))");
        }
        else
        {
            if(index == NameConflictedStalledIssue::RemoveDuplicated)
            {
               msgInfo.informativeText = tr("This action will delete the duplicate files.");
            }
            else if(!(index & NameConflictedStalledIssue::MergeFolders))
            {
                msgInfo.informativeText = tr("This action will delete the duplicate files and rename the remaining items in case of name conflict (adding a suffix like (1)).");
            }
            else
            {
                 msgInfo.informativeText = tr("This action will delete the duplicate files, merge all folders into a single one and rename the remaining items in case of name conflict (adding a suffix like (1)).");
            }
        }

        msgInfo.finishFunc = [this, index, selection, allSimilarIssues, header, nameConflict](QMessageBox* msgBox)
        {
            if(msgBox->result() == QDialogButtonBox::Ok)
            {
                MegaSyncApp->getStalledIssuesModel()->semiAutoSolveNameConflictIssues(selection, index);
            }
            else if(msgBox->result() == QDialogButtonBox::Yes)
            {
                MegaSyncApp->getStalledIssuesModel()->semiAutoSolveNameConflictIssues(allSimilarIssues, index);
            }
        };

        QMegaMessageBox::warning(msgInfo);
    }
}
