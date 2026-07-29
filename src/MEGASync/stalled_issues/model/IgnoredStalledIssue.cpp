#include "IgnoredStalledIssue.h"

#include "MegaApplication.h"
#include "MegaIgnoreManager.h"

#include <QDir>

#include <algorithm>

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

QString IgnoredStalledIssue::ignoreRulePattern(const IgnoredPath& ignoredPath,
                                               const mega::MegaSync& sync)
{
    QDir dir(ignoredPath.pathSide == IgnoredPath::IgnorePathSide::REMOTE ?
                 QString::fromUtf8(sync.getLastKnownMegaFolder()) :
                 QDir::toNativeSeparators(QString::fromUtf8(sync.getLocalFolder())));
    return dir.relativeFilePath(ignoredPath.path);
}

bool IgnoredStalledIssue::canBeIgnored() const
{
    // Symlinks are ignored with a fixed "-s:*" rule that carries no path, so they are always
    // ignorable. Every other case writes one rule per path, and a name that cannot be put on a
    // single line cannot be written as a rule at all.
    if (isSymLink())
    {
        return true;
    }

    if (mCanBeIgnored.has_value())
    {
        return mCanBeIgnored.value();
    }

    if (mIgnoredPaths.isEmpty())
    {
        return false;
    }

    if (syncIds().isEmpty())
    {
        return true;
    }

    std::shared_ptr<mega::MegaSync> sync(
        MegaSyncApp->getMegaApi()->getSyncByBackupId(firstSyncId()));
    if (!sync)
    {
        return true;
    }

    // Only the part below the sync root is written, so that is what has to be writable: a control
    // character in the root itself, or in a cloud folder above it, says nothing about this item.
    mCanBeIgnored = std::all_of(mIgnoredPaths.cbegin(),
                                mIgnoredPaths.cend(),
                                [&sync](const IgnoredPath& ignoredPath)
                                {
                                    return MegaIgnoreRule::isPatternWritable(
                                        ignoreRulePattern(ignoredPath, *sync));
                                });
    return mCanBeIgnored.value();
}

bool IgnoredStalledIssue::isAutoSolvable() const
{
    //Always autosolvable, we don´t need to check if smart mode is active
    return !isSolved() && !syncIds().isEmpty() && canBeIgnored() &&
           (mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedSymlink ||
            mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedHardLink ||
            mLinkType == mega::MegaSyncStall::SyncPathProblem::DetectedSpecialFile);
}

void IgnoredStalledIssue::fillIssue(const mega::MegaSyncStall* stall)
{
    mCanBeIgnored.reset();

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
    // if it is not any of these types, let the user expand it
    return !(isSymLink() || isSpecialLink() || isHardLink());
}

bool IgnoredStalledIssue::checkForExternalChanges(QObject*)
{
    return false;
}

//Only for Symbolic, hard and special links
StalledIssue::ResolutionState IgnoredStalledIssue::autoSolveIssue()
{
    setAutoResolutionApplied(true);
    StalledIssue::ResolutionState result(StalledIssue::ResolutionState::FAILED);

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
                auto ruleRejected(false);

                if(isSymLink())
                {
                    ignoreManager.addIgnoreSymLinksRule();
                    mSymLinksIgnoredInSyncs.insert(syncId, true);
                }
                else
                {
                    foreach(auto ignoredPath, mIgnoredPaths)
                    {
                        // The writer refuses a pattern it cannot put on a single line. Without
                        // this check the issue would be reported as solved while nothing was
                        // written.
                        if (!ignoreManager.addNameRule(MegaIgnoreNameRule::Class::EXCLUDE,
                                                       ignoreRulePattern(ignoredPath, *sync),
                                                       MegaIgnoreNameRule::Target::NONE,
                                                       MegaIgnoreNameRule::Type::p))
                        {
                            ruleRejected = true;
                        }
                    }
                }

                auto changesApplied(ignoreManager.applyChanges());
                if (!ruleRejected &&
                    changesApplied < MegaIgnoreManager::ApplyChangesError::NO_WRITE_PERMISSION)
                {
                    result = StalledIssue::ResolutionState::SOLVED;
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
            result = StalledIssue::ResolutionState::SOLVED;
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
    return canBeIgnored();
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
