#include "MoveOrRenameCannotOccurIssue.h"

#include <syncs/control/SyncInfo.h>
#include <syncs/control/SyncController.h>
#include <syncs/control/SyncSettings.h>
#include <StalledIssuesUtilities.h>

#include <MegaApplication.h>
#include <Utilities.h>

#include <QDir>

QMap<mega::MegaHandle, MoveOrRenameIssueChosenSide> MoveOrRenameCannotOccurIssue::mChosenSideBySyncId = QMap<mega::MegaHandle, MoveOrRenameIssueChosenSide>();

std::shared_ptr<StalledIssue> MoveOrRenameCannotOccurFactory::createIssue(MultiStepIssueSolverBase* solver, const mega::MegaSyncStall* stall)
{
    auto updateIssue = [](const mega::MegaSyncStall* stall, std::shared_ptr<MoveOrRenameCannotOccurIssue> issue){
        stall->detectedCloudSide() ? issue->fillCloudSide(stall)
                                   : issue->fillLocalSide(stall);
    };

    auto syncIds(StalledIssuesBySyncFilter::getSyncIdsByStall(stall));
    if(!syncIds.isEmpty())
    {
        auto syncId(*syncIds.begin());

        if(mIssueBySyncId.contains(syncId))
        {
            auto previousIssue(mIssueBySyncId.value(syncId));
            updateIssue(stall, previousIssue);
        }
        else
        {
            std::shared_ptr<MoveOrRenameCannotOccurIssue> moveIssue(nullptr);

            if(solver)
            {
                auto moveOrRenameSolver(dynamic_cast<MoveOrRenameMultiStepIssueSolver*>(solver));
                if(moveOrRenameSolver)
                {
                    moveIssue = moveOrRenameSolver->getIssue();
                    moveIssue->increaseCombinedNumberOfIssues();
                }
            }
            else if(syncId != mega::INVALID_HANDLE)
            {
                moveIssue =std::make_shared<MoveOrRenameCannotOccurIssue>(stall);
            }

            if(moveIssue)
            {
                updateIssue(stall, moveIssue);
                mIssueBySyncId.insert(syncId, moveIssue);
                return moveIssue;
            }
        }
    }

    //We don´t want to add it to the model, just update it
    return nullptr;
}

void MoveOrRenameCannotOccurFactory::clear()
{
    mIssueBySyncId.clear();
}

//////////////////////////////////////

MoveOrRenameCannotOccurIssue::MoveOrRenameCannotOccurIssue(const mega::MegaSyncStall* stall)
    : StalledIssue(stall)
    , mSolvingStarted(false)
    , mSyncController(new SyncController())
    , mChosenSide(MoveOrRenameIssueChosenSide::NONE)
    , mCombinedNumberOfIssues(1)
{
}

bool MoveOrRenameCannotOccurIssue::isValid() const
{
    return consultCloudData() && consultLocalData();
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
    return true;
}

void MoveOrRenameCannotOccurIssue::solveIssue(MoveOrRenameIssueChosenSide side)
{
    if(!syncIds().isEmpty())
    {
        auto syncId(firstSyncId());
        mChosenSideBySyncId.insert(syncId, side);
        mChosenSide = side;
        auto syncSettings = SyncInfo::instance()->getSyncSettingByTag(syncId);
        if(syncSettings)
        {
            connect(SyncInfo::instance(), &SyncInfo::syncStateChanged,
                this, &MoveOrRenameCannotOccurIssue::onSyncPausedEnds);

            mSolvingStarted = true;

            //We pause the sync and when it is really paused, we continue solving the issue
            //This step is needed as the SDK acts differently if the sync is not paused
            mSyncController->setSyncToPause(syncSettings);

            startAsyncIssueSolving();
        }
    }
}

//Now the sync is paused, so we continue solving the issue
void MoveOrRenameCannotOccurIssue::onSyncPausedEnds(std::shared_ptr<SyncSettings> syncSettings)
{
    if (!syncIds().isEmpty())
    {
        auto syncId(firstSyncId());

        if (syncSettings->backupId() != syncId)
        {
            return;
        }

        if (!mSolvingStarted)
        {
            return;
        }

        const auto syncState = syncSettings->getRunState();
        if (syncState == mega::MegaSync::RUNSTATE_PAUSED ||
            syncState == mega::MegaSync::RUNSTATE_SUSPENDED)
        {
            bool created(false);

            //We perform the undo in the opposite side
            if (getChosenSide() == MoveOrRenameIssueChosenSide::REMOTE)
            {
                QFileInfo previousPath(consultLocalData()->getFilePath());
                QFileInfo previousDirectory(previousPath.absolutePath());

                created = previousDirectory.exists();

                if (!created)
                {
                    created = QDir().mkpath(previousDirectory.absoluteFilePath());
                }

                if (created)
                {
                    QFile file(consultLocalData()->getNativeMoveFilePath());
                    created = file.rename(previousPath.absoluteFilePath());
                }
            }
            else
            {
                std::unique_ptr<mega::MegaNode> nodeToMove(
                    MegaSyncApp->getMegaApi()->getNodeByHandle(
                        consultCloudData()->getMovePathHandle()));
                if (nodeToMove)
                {
                    std::unique_ptr<mega::MegaNode> newParent(
                        MegaSyncApp->getMegaApi()->getNodeByHandle(
                            consultCloudData()->getPathHandle()));
                    if (!newParent)
                    {
                        PathCreator dirCreator;
                        dirCreator.mkDir(QString(), consultCloudData()->getNativeFilePath());
                    }
                    MegaSyncApp->getMegaApi()->moveNode(nodeToMove.get(), newParent.get());
                    created = true;
                }
            }

            mSyncController->setSyncToRun(syncSettings);
        }

        disconnect(SyncInfo::instance(),
            &SyncInfo::syncStateChanged,
            this,
            &MoveOrRenameCannotOccurIssue::onSyncPausedEnds);

        mSolvingStarted = false;
    }
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
        if (previousNode || !currentNode)
        {
            setIsSolved(StalledIssue::SolveType::POTENTIALLY_SOLVED);
        }
        else
        {
            QFileInfo previousPath(consultLocalData()->getPath().path);
            QFileInfo currentPath(consultLocalData()->getMovePath().path);
            if (previousPath.exists() || !currentPath.exists())
            {
                setIsSolved(StalledIssue::SolveType::POTENTIALLY_SOLVED);
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
    auto syncId(firstSyncId());
    return mChosenSideBySyncId.value(syncId, MoveOrRenameIssueChosenSide::NONE);
}

QString MoveOrRenameCannotOccurIssue::syncName() const
{
    std::unique_ptr<mega::MegaSync> sync(MegaSyncApp->getMegaApi()->getSyncByBackupId(firstSyncId()));
    if(sync)
    {
        return QString::fromUtf8(sync->getName());
    }

    return QString();
}

void MoveOrRenameCannotOccurIssue::increaseCombinedNumberOfIssues()
{
    mCombinedNumberOfIssues++;
}

int MoveOrRenameCannotOccurIssue::combinedNumberOfIssues() const
{
    return mCombinedNumberOfIssues;
}

bool MoveOrRenameCannotOccurIssue::findIssue(
    const std::shared_ptr<const MoveOrRenameCannotOccurIssue> issue)
{
    auto syncId(issue->firstSyncId());
    return mChosenSideBySyncId.contains(syncId);
}

void MoveOrRenameCannotOccurIssue::finishAsyncIssueSolving()
{
    mChosenSideBySyncId.clear();
    StalledIssue::finishAsyncIssueSolving();
}

void MoveOrRenameCannotOccurIssue::fillCloudSide(const mega::MegaSyncStall* stall)
{
    auto cloudSourcePath = QString::fromUtf8(stall->path(true, 0));
    auto cloudTargetPath = QString::fromUtf8(stall->path(true, 1));

    if(!cloudSourcePath.isEmpty())
    {
        initCloudIssue();
        getCloudData()->mPath.path = cloudSourcePath;
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(cloudSourcePath.toStdString().c_str()));
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

        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(cloudTargetPath.toStdString().c_str()));
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
