#include "MoveOrRenameCannotOccurIssue.h"

#include <syncs/control/SyncInfo.h>
#include <syncs/control/SyncController.h>
#include <syncs/control/SyncSettings.h>

#include <MegaApplication.h>
#include <Utilities.h>

#include <QDir>

QMap<mega::MegaHandle, MoveOrRenameIssueChosenSide> MoveOrRenameCannotOccurIssue::mChosenSideBySyncId = QMap<mega::MegaHandle, MoveOrRenameIssueChosenSide>();

StalledIssueVariant MoveOrRenameCannotOccurFactory::createIssue(
    const mega::MegaSyncStall* stall)
{
    auto newIssue = std::make_shared<MoveOrRenameCannotOccurIssue>(stall);
    if(!newIssue->syncIds().isEmpty())
    {
        auto newIssueSyncId(newIssue->syncIds().first());
        auto previousIssue = mIssueBySyncId.value(newIssueSyncId);

        if (previousIssue.isValid())
        {
            auto moveOrRenameIssue = previousIssue.convert<MoveOrRenameCannotOccurIssue>();
            if (moveOrRenameIssue)
            {
                stall->detectedCloudSide() ? moveOrRenameIssue->fillCloudSide(stall)
                                           : moveOrRenameIssue->fillLocalSide(stall);
            }

            return StalledIssueVariant();
        }
        else
        {
            StalledIssueVariant variant(newIssue, stall);
            stall->detectedCloudSide() ? newIssue->fillCloudSide(stall)
                                       : newIssue->fillLocalSide(stall);
            mIssueBySyncId.insert(newIssue->syncIds().first(), variant);
            return variant;
        }
    }

    return StalledIssueVariant();
}

void MoveOrRenameCannotOccurFactory::setSyncIdIssuesBeingSolved(mega::MegaHandle syncId, MoveOrRenameIssueChosenSide side)
{
    mSyncIssuesBeingSolved.syncId = syncId;
    mSyncIssuesBeingSolved.chosenSide = side;
}

//////////////////////////////////////

MoveOrRenameCannotOccurIssue::MoveOrRenameCannotOccurIssue(const mega::MegaSyncStall* stall)
    : QObject(nullptr)
    , StalledIssue(stall)
    , mSolvingStarted(false)
    , mSyncController(new SyncController())
    , mChosenSide(MoveOrRenameIssueChosenSide::NONE)
{
    connect(SyncInfo::instance(), &SyncInfo::syncStateChanged,
            this, &MoveOrRenameCannotOccurIssue::onSyncPausedEnds);

    fillBasicInfo(stall);
}

//We don´t fill the issue as usual
void MoveOrRenameCannotOccurIssue::fillIssue(const mega::MegaSyncStall*)
{

}

bool MoveOrRenameCannotOccurIssue::isAutoSolvable() const
{
    //If it is unsolved but the chosen side is set, it is because user has started solving these issues in this sync id
    if(!isSolved() && !(getSyncIdChosenSide() == MoveOrRenameIssueChosenSide::NONE))
    {
        return true;
    }

    if (consultLocalData() && consultCloudData())
    {
        std::unique_ptr<mega::MegaNode> previousNode(
            MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getPathHandle()));
        std::unique_ptr<mega::MegaNode> currentNode(
            MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getMovePathHandle()));
        if (previousNode && currentNode)
        {
            return !isSolved();
        }
        else
        {
            QFileInfo previousPath(consultLocalData()->getPath().path);
            QFileInfo currentPath(consultLocalData()->getMovePath().path);
            if (previousPath.exists() && currentPath.exists())
            {
                return !isSolved();
            }
        }
    }

    return false;
}

bool MoveOrRenameCannotOccurIssue::refreshListAfterSolving() const
{
    return false;
}

void MoveOrRenameCannotOccurIssue::solveIssue(MoveOrRenameIssueChosenSide side)
{
    if(!syncIds().isEmpty())
    {
        auto syncId(syncIds().first());
        mChosenSideBySyncId.insert(syncId, side);
        mChosenSide = side;
        auto syncSettings = SyncInfo::instance()->getSyncSettingByTag(syncId);
        if(syncSettings)
        {
            mSolvingStarted = true;

            //We pause the sync and when it is really paused, we continue solving the issue
            mSyncController->setSyncToPause(syncSettings);
        }
    }
}

//Now the sync is paused, so we continue solving the issue
void MoveOrRenameCannotOccurIssue::onSyncPausedEnds(std::shared_ptr<SyncSettings> syncSettings)
{
    if(!mSolvingStarted)
    {
        return;
    }
    const auto syncState = syncSettings->getRunState();
    if(syncState == mega::MegaSync::RUNSTATE_PAUSED || syncState == mega::MegaSync::RUNSTATE_SUSPENDED)
    {
        bool created(false);

        //We perform the undo in the opposite side
        if(getChosenSide() == MoveOrRenameIssueChosenSide::REMOTE)
        {
            QFileInfo previousPath(consultLocalData()->getFilePath());
            QFileInfo previousDirectory(previousPath.absolutePath());

            created = previousDirectory.exists();

            if(!created)
            {
                created = QDir().mkpath(previousDirectory.absoluteFilePath());
            }

            if(created)
            {
                QFile file(consultLocalData()->getNativeMoveFilePath());
                created = file.rename(previousPath.absoluteFilePath());
            }
        }
        else
        {
            std::unique_ptr<mega::MegaNode> nodeToMove(MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getMovePathHandle()));
            if(nodeToMove)
            {
                std::unique_ptr<mega::MegaNode> newParent(MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getPathHandle()));
                if(!newParent)
                {
                    PathCreator dirCreator;
                    dirCreator.mkDir(QString(), consultCloudData()->getNativeFilePath());
                }
                MegaSyncApp->getMegaApi()->moveNode(nodeToMove.get(), newParent.get());
                created = true;
            }
        }

        if(created)
        {
            disconnect(SyncInfo::instance(), &SyncInfo::syncStateChanged,
                this, &MoveOrRenameCannotOccurIssue::onSyncPausedEnds);
            setIsSolved(false);
        }

        emit issueSolved(created);

        mSyncController->setSyncToRun(syncSettings);
    }

    mSolvingStarted = false;
}

bool MoveOrRenameCannotOccurIssue::autoSolveIssue()
{
    auto chosenSide(getSyncIdChosenSide());
    if(isAutoSolvable() && !(chosenSide == MoveOrRenameIssueChosenSide::NONE))
    {
        solveIssue(chosenSide);
        return true;
    }

    return false;
}

bool MoveOrRenameCannotOccurIssue::checkForExternalChanges()
{
    if(!isSolved())
    {
        std::unique_ptr<mega::MegaNode> previousNode(
            MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getPathHandle()));
        std::unique_ptr<mega::MegaNode> currentNode(
            MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getMovePathHandle()));
        if (!previousNode || !currentNode)
        {
            setIsSolved(true);
        }
        else
        {
            QFileInfo previousPath(consultLocalData()->getPath().path);
            QFileInfo currentPath(consultLocalData()->getMovePath().path);
            if (!previousPath.absoluteDir().exists() || !currentPath.exists())
            {
                setIsSolved(true);
            }
        }
    }

    return isPotentiallySolved();
}

MoveOrRenameIssueChosenSide MoveOrRenameCannotOccurIssue::getChosenSide() const
{
    return mChosenSide;
}

MoveOrRenameIssueChosenSide MoveOrRenameCannotOccurIssue::getSyncIdChosenSide() const
{
    auto syncId(syncIds().first());
    return mChosenSideBySyncId.value(syncId, MoveOrRenameIssueChosenSide::NONE);
}

bool MoveOrRenameCannotOccurIssue::needsAutomaticRefresh(
    const std::shared_ptr<const MoveOrRenameCannotOccurIssue> issue)
{
    auto syncId(issue->syncIds().first());
    return mChosenSideBySyncId.contains(syncId);
}

void MoveOrRenameCannotOccurIssue::automaticRefreshFinished()
{
    mChosenSideBySyncId.clear();
}

void MoveOrRenameCannotOccurIssue::fillCloudSide(const mega::MegaSyncStall* stall)
{
    auto cloudSourcePath = QString::fromUtf8(stall->path(true, 0));
    auto cloudTargetPath = QString::fromUtf8(stall->path(true, 1));

    if(!cloudSourcePath.isEmpty())
    {
        initCloudIssue();
        getCloudData()->mPath.path = cloudSourcePath;
        QFileInfo previousPath(cloudSourcePath);
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(previousPath.absolutePath().toStdString().c_str()));
        if(node)
        {
            getCloudData()->mPathHandle = node->getHandle();
        }

        setIsFile(cloudSourcePath, false);
    }

    if(!cloudTargetPath.isEmpty())
    {
        initCloudIssue();
        getCloudData()->mMovePath.path = cloudTargetPath;

        QFileInfo targetPath(cloudTargetPath);
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(targetPath.absoluteFilePath().toStdString().c_str()));
        if(node)
        {
            getCloudData()->mMovePathHandle = node->getHandle();
        }

        setIsFile(cloudTargetPath, false);
    }
}

void MoveOrRenameCannotOccurIssue::fillLocalSide(const mega::MegaSyncStall* stall)
{
    auto localSourcePath = QString::fromUtf8(stall->path(false, 0));
    auto localTargetPath = QString::fromUtf8(stall->path(false, 1));

    if(!localSourcePath.isEmpty())
    {
        initLocalIssue();
        getLocalData()->mPath.path = localSourcePath;

        setIsFile(localSourcePath, true);
    }

    if(!localTargetPath.isEmpty())
    {
        initLocalIssue();
        getLocalData()->mMovePath.path = localTargetPath;

        setIsFile(localTargetPath, true);
    }
}
