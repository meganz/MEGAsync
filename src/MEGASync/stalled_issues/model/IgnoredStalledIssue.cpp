#include "IgnoredStalledIssue.h"

#include <StalledIssuesUtilities.h>
#include <MegaApplication.h>
#include <syncs/control/MegaIgnoreManager.h>
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>
#include <StalledIssuesModel.h>

QMap<mega::MegaHandle, bool> IgnoredStalledIssue::mIgnoredSyncs = QMap<mega::MegaHandle, bool>();

IgnoredStalledIssue::IgnoredStalledIssue(const mega::MegaSyncStall *stallIssue)
    : StalledIssue(stallIssue)
{

}

void IgnoredStalledIssue::clearIgnoredSyncs()
{
    mIgnoredSyncs.clear();
}

bool IgnoredStalledIssue::isSolvable() const
{
    return !isSolved() && !syncIds().isEmpty();
}

bool IgnoredStalledIssue::autoSolveIssue()
{
    if(!syncIds().isEmpty())
    {
        auto syncId(syncIds().first());
        //We could do it without this static list
        //as the MegaIgnoreManager checks if the rule already exists
        //but with the list we save the megaignore parser, so it is more efficient
        if(!mIgnoredSyncs.contains(syncId))
        {
            std::shared_ptr<mega::MegaSync>sync(MegaSyncApp->getMegaApi()->getSyncByBackupId(syncId));
            if(sync)
            {
                auto folderPath(QDir::toNativeSeparators(QString::fromUtf8(sync->getLocalFolder())));

                MegaIgnoreManager ignoreManager(folderPath, false);
                ignoreManager.addIgnoreSymLinksRule();
                auto changesApplied(ignoreManager.applyChanges());
                if(changesApplied < MegaIgnoreManager::ApplyChangesError::NO_WRITE_PERMISSION)
                {
                    setIsSolved(false);
                    mIgnoredSyncs.insert(syncId, true);
                }
                else
                {
                    Utilities::queueFunctionInAppThread([this]()
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
                    mIgnoredSyncs.insert(syncId, false);
                }

            }
        }
        else if(mIgnoredSyncs.value(syncId) == true)
        {
            setIsSolved(false);
        }
    }

    return isSolved();
}
