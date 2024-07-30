#include "MoveOrRenameCannotOccurIssue.h"

#include "SyncInfo.h"
#include "SyncController.h"
#include "SyncSettings.h"
#include <StalledIssuesUtilities.h>

#include <StatsEventHandler.h>

#include <MegaApplication.h>
#include <Utilities.h>
#include <MEGAPathCreator.h>

#include <QDir>

//////////////////////////////////////

const int MAX_RETRIES = 5;

MoveOrRenameCannotOccurIssue::MoveOrRenameCannotOccurIssue(const mega::MegaSyncStall* stall)
    : StalledIssue(stall)
    , mega::MegaRequestListener()
    , mSolvingStarted(false)
    , mUndoSuccessful(false)
    , mChosenSide(MoveOrRenameIssueChosenSide::NONE)
    , mCombinedNumberOfIssues(1)
    , mSolveAttempts(0)
    , mListener(std::make_shared<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
{
}

bool MoveOrRenameCannotOccurIssue::isValid() const
{
    return consultCloudData() || consultLocalData();
}

//We don´t fill the issue as usual
void MoveOrRenameCannotOccurIssue::fillIssue(const mega::MegaSyncStall* stall)
{
    if(stall->detectedCloudSide() && !consultCloudData())
    {
        fillCloudSide(stall);
    }
    else if(!stall->detectedCloudSide() && !consultLocalData())
    {
        fillLocalSide(stall);
    }
}

bool MoveOrRenameCannotOccurIssue::isAutoSolvable() const
{
    //If it is unsolved but the chosen side is set, it is because user has started solving these issues in this sync id
    if(!isSolved() && !(getSyncIdChosenSide() == MoveOrRenameIssueChosenSide::NONE))
    {
        return !solveAttemptsAchieved();
    }

    return false;
}

void MoveOrRenameCannotOccurIssue::solveIssue(MoveOrRenameIssueChosenSide side)
{
    if(!syncIds().isEmpty())
    {
        auto syncId(firstSyncId());
        mChosenSideBySyncId.insert(syncId, side);
        mChosenSide = side;
        mUndoSuccessful = false;

        auto syncSettings = SyncInfo::instance()->getSyncSettingByTag(syncId);
        if(syncSettings)
        {
            connect(SyncInfo::instance(), &SyncInfo::syncStateChanged,
                this, &MoveOrRenameCannotOccurIssue::onSyncPausedEnds);

            mSolvingStarted = true;
            mSolveAttempts++;

            //We pause the sync and when it is really paused, we continue solving the issue
            //This step is needed as the SDK acts differently if the sync is not paused
            SyncController::instance().setSyncToPause(syncSettings);

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
            //We perform the undo in the opposite side
            if (getChosenSide() == MoveOrRenameIssueChosenSide::REMOTE && consultLocalData())
            {
                QFileInfo previousPath(consultLocalData()->getFilePath());
                QFileInfo previousDirectory(previousPath.absolutePath());
                QString currentPath(consultLocalData()->getNativeMoveFilePath());

                mUndoSuccessful = previousDirectory.exists();

                if (!mUndoSuccessful)
                {
                    mUndoSuccessful = QDir().mkpath(previousDirectory.absolutePath());
                }

                if (mUndoSuccessful)
                {
                    QFile file(currentPath);
                    mUndoSuccessful = file.rename(previousPath.absoluteFilePath());
                }

                if(!mUndoSuccessful)
                {
                    mFailedLocalPaths.insert(currentPath);
                }
            }
            else if(getChosenSide() == MoveOrRenameIssueChosenSide::LOCAL && consultCloudData())
            {
                std::unique_ptr<mega::MegaNode> nodeToMove(
                    MegaSyncApp->getMegaApi()->getNodeByHandle(
                        consultCloudData()->getMovePathHandle()));

                if(!nodeToMove)
                {
                    nodeToMove.reset(
                        MegaSyncApp->getMegaApi()->getNodeByPath(consultCloudData()->getMovePath().path.toStdString().c_str()));
                }

                if (nodeToMove)
                {
                    QFileInfo targetPath(consultCloudData()->getNativeFilePath());
                    std::unique_ptr<mega::MegaNode> newParent(
                        MegaSyncApp->getMegaApi()->getNodeByPath(
                            targetPath.path().toStdString().c_str()));

                    if (!newParent)
                    {
                        std::shared_ptr<mega::MegaError> error(nullptr);
                        MEGAPathCreator::mkDir(QString(), targetPath.path(), error);

                        if(error)
                        {
                            return;
                        }
                        else
                        {
                            newParent.reset(MegaSyncApp->getMegaApi()->getNodeByPath(
                                targetPath.path().toStdString().c_str()));
                        }
                    }

                    if(strcmp(nodeToMove->getName(), targetPath.fileName().toStdString().c_str()) != 0)
                    {
                        MegaSyncApp->getMegaApi()->renameNode(nodeToMove.get(), targetPath.fileName().toStdString().c_str(), mListener.get());
                    }
                    else
                    {
                        MegaSyncApp->getMegaApi()->moveNode(
                            nodeToMove.get(), newParent.get(), mListener.get());
                    }
                }
            }

            onUndoFinished(syncSettings);

            MegaSyncApp->getMegaApi()->clearStalledPath(originalStall.get());
        }
    }
}

void MoveOrRenameCannotOccurIssue::onUndoFinished(std::shared_ptr<SyncSettings> syncSettings)
{
    SyncController::instance().setSyncToRun(syncSettings);
    disconnect(SyncInfo::instance(),
        &SyncInfo::syncStateChanged,
        this,
        &MoveOrRenameCannotOccurIssue::onSyncPausedEnds);

    mSolvingStarted = false;

    if(mUndoSuccessful)
    {
        mCloudData.reset();
        mLocalData.reset();
        mSolveAttempts = 0;
    }
}

bool MoveOrRenameCannotOccurIssue::solveAttemptsAchieved() const
{
    return mSolveAttempts >= MAX_RETRIES;
}

void MoveOrRenameCannotOccurIssue::onRequestFinish(
    mega::MegaApi*, mega::MegaRequest* request, mega::MegaError* e)
{
    if(request->getType() == mega::MegaRequest::TYPE_MOVE ||
        request->getType() == mega::MegaRequest::TYPE_RENAME)
    {
        mUndoSuccessful = !e || (e->getErrorCode() == mega::MegaError::API_OK);

        auto syncId(firstSyncId());
        auto syncSettings = SyncInfo::instance()->getSyncSettingByTag(syncId);
        if(syncSettings)
        {
           onUndoFinished(syncSettings);
        }
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
    if(!isSolved() && consultCloudData())
    {
        if(consultCloudData()->getPathHandle() != mega::INVALID_HANDLE || consultCloudData()->getMovePathHandle() != mega::INVALID_HANDLE)
        {
            std::unique_ptr<mega::MegaNode> previousNode(
                MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getPathHandle()));
            std::unique_ptr<mega::MegaNode> currentNode(
                MegaSyncApp->getMegaApi()->getNodeByHandle(consultCloudData()->getMovePathHandle()));
            if (previousNode && !currentNode)
            {
                setIsSolved(StalledIssue::SolveType::POTENTIALLY_SOLVED);
            }
        }
    }

    if(!isSolved() && consultLocalData())
    {
        QFileInfo previousPath(consultLocalData()->getPath().path);
        QFileInfo currentPath(consultLocalData()->getMovePath().path);
        if (previousPath.exists() && !currentPath.exists())
        {
            setIsSolved(StalledIssue::SolveType::POTENTIALLY_SOLVED);
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
    StalledIssue::performFinishAsyncIssueSolving(!mUndoSuccessful);
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

void MoveOrRenameCannotOccurIssue::setIsSolved(SolveType type)
{
    if(type == SolveType::FAILED)
    {
        mChosenSide = MoveOrRenameIssueChosenSide::NONE;
    }
    else if(type == SolveType::SOLVED)
    {
        MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SI_MOVERENAME_CANNOT_OCCUR_SOLVED_MANUALLY);
    }

    StalledIssue::setIsSolved(type);
}

/////////////////////////
QMap<mega::MegaHandle, MoveOrRenameIssueChosenSide> MoveOrRenameCannotOccurIssue::mChosenSideBySyncId = QMap<mega::MegaHandle, MoveOrRenameIssueChosenSide>();

std::shared_ptr<StalledIssue> MoveOrRenameCannotOccurFactory::createIssue(MultiStepIssueSolverBase* solver, const mega::MegaSyncStall* stall)
{
    auto syncIds(StalledIssuesBySyncFilter::getSyncIdsByStall(stall));
    if(!syncIds.isEmpty())
    {
        auto syncId(*syncIds.begin());

        if(mIssueBySyncId.contains(syncId))
        {
            auto previousIssue(mIssueBySyncId.value(syncId));
            previousIssue->fillIssue(stall);
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
