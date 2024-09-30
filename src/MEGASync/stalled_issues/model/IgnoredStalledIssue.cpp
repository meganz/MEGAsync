#include "IgnoredStalledIssue.h"

#include <StalledIssuesUtilities.h>
#include <MegaApplication.h>
#include "MegaIgnoreManager.h"
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>
#include <StalledIssuesModel.h>

QMap<mega::MegaHandle, bool> IgnoredStalledIssue::mSymLinksIgnoredInSyncs = QMap<mega::MegaHandle, bool>();

IgnoredStalledIssue::IgnoredStalledIssue(const mega::MegaSyncStall *stallIssue)
    : StalledIssue(stallIssue) ,
    mLinkType(mega::MegaSyncStall::SyncPathProblem::NoProblem)
{

}

void IgnoredStalledIssue::clearIgnoredSyncs()
{
    mSymLinksIgnoredInSyncs.clear();
}

mega::MegaSyncStall::SyncPathProblem IgnoredStalledIssue::linkType() const
{
    return mLinkType;
}

bool IgnoredStalledIssue::isAutoSolvable() const
{
    //Always autosolvable, we don´t need to check if smart mode is active
    return !isSolved() && !syncIds().isEmpty() &&
           (mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedSymlink ||
            mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedHardLink ||
            mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedSpecialFile);
}

void IgnoredStalledIssue::fillIssue(const mega::MegaSyncStall* stall)
{
    StalledIssue::fillIssue(stall);
    if(getReason() == mega::MegaSyncStall::FileIssue && consultLocalData())
    {
        mLinkType = consultLocalData()->getPath().pathProblem;
    }

    //FILL LOCAL IGNORED PATHS
    {
        auto fillLocalIgnoredPath = [this](const QString& path)
        {
            QFileInfo info(path);
            if(info.exists())
            {
                mIgnoredPaths.append({path,
                    IgnoredPath::IgnorePathSide::LOCAL,
                    info.isFile() ? MegaIgnoreNameRule::Target::f : MegaIgnoreNameRule::Target::d});
            }
        };

        if(stall->couldSuggestIgnoreThisPath(false, 0))
        {
            fillLocalIgnoredPath(consultLocalData()->getNativeFilePath());
        }

        if(stall->couldSuggestIgnoreThisPath(false, 1))
        {
            fillLocalIgnoredPath(consultLocalData()->getNativeMoveFilePath());
        }
    }

    //FILL MEGA IGNORED PATHS
    {
        auto fillCloudIgnoredPath = [this](const mega::MegaHandle& handle, const QString& path)
        {
            std::shared_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
            if(node)
            {
                mIgnoredPaths.append({path,
                    IgnoredPath::IgnorePathSide::REMOTE,
                    node->isFile() ? MegaIgnoreNameRule::Target::f : MegaIgnoreNameRule::Target::d});
            }
        };

        if(stall->couldSuggestIgnoreThisPath(true, 0))
        {
            fillCloudIgnoredPath(
                consultCloudData()->getPathHandle(), consultCloudData()->getNativeFilePath());
        }

        if(stall->couldSuggestIgnoreThisPath(true, 1))
        {
            fillCloudIgnoredPath(consultCloudData()->getMovePathHandle(),
                consultCloudData()->getNativeMoveFilePath());
        }
    }
}

bool IgnoredStalledIssue::isSymLink() const
{
    return mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedSymlink;
}

bool IgnoredStalledIssue::isSpecialLink() const
{
    return mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedSpecialFile;
}

bool IgnoredStalledIssue::isHardLink() const
{
    return mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedHardLink;
}

bool IgnoredStalledIssue::isExpandable() const
{
    return false;
}

bool IgnoredStalledIssue::checkForExternalChanges()
{
    return false;
}

//Only for Symbolic, hard and special links
StalledIssue::AutoSolveIssueResult IgnoredStalledIssue::autoSolveIssue()
{
    setAutoResolutionApplied(true);
    StalledIssue::AutoSolveIssueResult result(StalledIssue::AutoSolveIssueResult::FAILED);

    if(!syncIds().isEmpty())
    {
        auto syncId(firstSyncId());
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
                    QDir dir;
                    foreach(auto ignoredPath, mIgnoredPaths)
                    {
                        if(ignoredPath.pathSide == IgnoredPath::IgnorePathSide::REMOTE)
                        {
                            dir.setPath(QString::fromUtf8(sync->getLastKnownMegaFolder()));
                        }
                        else
                        {
                            dir.setPath(folderPath);
                        }

                        ignoreManager.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE,
                            dir.relativeFilePath(ignoredPath.path), ignoredPath.target);
                    }
                }

                auto changesApplied(ignoreManager.applyChanges());
                if(changesApplied < MegaIgnoreManager::ApplyChangesError::NO_WRITE_PERMISSION)
                {
                    result = StalledIssue::AutoSolveIssueResult::SOLVED;
                }
                else
                {
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
            result = StalledIssue::AutoSolveIssueResult::SOLVED;
        }
    }

    return result;
}

CloudNodeIsBlockedIssue::CloudNodeIsBlockedIssue(const mega::MegaSyncStall* stallIssue)
    : IgnoredStalledIssue(stallIssue)
{

}

bool CloudNodeIsBlockedIssue::isAutoSolvable() const
{
    return true;
}

void CloudNodeIsBlockedIssue::fillIssue(const mega::MegaSyncStall* stall)
{
    IgnoredStalledIssue::fillIssue(stall);
    mIgnoredPaths.append({consultCloudData()->getFilePath(), IgnoredPath::IgnorePathSide::REMOTE, MegaIgnoreNameRule::Target::f});
}

bool CloudNodeIsBlockedIssue::showDirectoryInHyperlink() const
{
    return true;
}

bool CloudNodeIsBlockedIssue::isExpandable() const
{
    return true;
}
