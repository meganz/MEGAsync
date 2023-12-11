#include "IgnoredStalledIssue.h"

#include <StalledIssuesUtilities.h>
#include <MegaApplication.h>
#include <syncs/control/MegaIgnoreManager.h>
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>
#include <StalledIssuesModel.h>

QMap<mega::MegaHandle, bool> IgnoredStalledIssue::mSymLinksIgnoredInSyncs = QMap<mega::MegaHandle, bool>();

IgnoredStalledIssue::IgnoredStalledIssue(const mega::MegaSyncStall *stallIssue)
    : StalledIssue(stallIssue)
{

}

void IgnoredStalledIssue::clearIgnoredSyncs()
{
    mSymLinksIgnoredInSyncs.clear();
}

bool IgnoredStalledIssue::isSolvable() const
{
    return !isSolved() &&
           !syncIds().isEmpty() &&
           isSpecialLink();
}

bool IgnoredStalledIssue::isSymLink() const
{
    return getReason() == mega::MegaSyncStall::FileIssue &&
           consultLocalData() &&
           consultLocalData()->getPath().pathProblem == mega::MegaSyncStall::SyncPathProblem::DetectedSymlink;
}

bool IgnoredStalledIssue::isSpecialLink() const
{
    return getReason() == mega::MegaSyncStall::FileIssue &&
           consultLocalData() &&
           (consultLocalData()->getPath().pathProblem == mega::MegaSyncStall::SyncPathProblem::DetectedSymlink ||
            consultLocalData()->getPath().pathProblem == mega::MegaSyncStall::SyncPathProblem::DetectedHardLink ||
            consultLocalData()->getPath().pathProblem == mega::MegaSyncStall::SyncPathProblem::DetectedSpecialFile);
}

bool IgnoredStalledIssue::autoSolveIssue()
{
    if(!syncIds().isEmpty())
    {
        auto syncId(syncIds().first());
        //We could do it without this static list
        //as the MegaIgnoreManager checks if the rule already exists
        //but with the list we save the megaignore parser, so it is more efficient
        if(!mSymLinksIgnoredInSyncs.contains(syncId) ||
           !isSymLink())
        {
            std::shared_ptr<mega::MegaSync>sync(MegaSyncApp->getMegaApi()->getSyncByBackupId(syncId));
            if(sync)
            {
                auto folderPath(QDir::toNativeSeparators(QString::fromUtf8(sync->getLocalFolder())));

                MegaIgnoreManager ignoreManager(folderPath, false);
                if(isSymLink())
                {
                    ignoreManager.addIgnoreSymLinksRule();
                    mSymLinksIgnoredInSyncs.insert(syncId, true);
                }
                else
                {
                    QDir dir(folderPath);
                    foreach(auto ignoredPath, mIgnoredPaths)
                    {
                        ignoreManager.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE, dir.relativeFilePath(ignoredPath));
                    }
                }

                auto changesApplied(ignoreManager.applyChanges());
                if(changesApplied < MegaIgnoreManager::ApplyChangesError::NO_WRITE_PERMISSION)
                {
                    setIsSolved(false);
                }
                else
                {
                    Utilities::queueFunctionInAppThread([]()
                    {
                        auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

                        QMegaMessageBox::MessageBoxInfo msgInfo;
                        msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
                        msgInfo.title = MegaSyncApp->getMEGAString();
                        msgInfo.textFormat = Qt::RichText;
                        msgInfo.buttons = QMessageBox::Ok;
                        msgInfo.text = MegaSyncApp->getStalledIssuesModel()->tr("We could not update the megaignore file. Please, check if it has write permissions.");
                        QMegaMessageBox::warning(msgInfo);
                    });

                    if(isSymLink())
                    {
                        mSymLinksIgnoredInSyncs.remove(syncId);
                    }
                }
            }
        }
        //Only done for sym links
        else if(mSymLinksIgnoredInSyncs.value(syncId) == true)
        {
            setIsSolved(false);
        }
    }

    return isSolved();
}
