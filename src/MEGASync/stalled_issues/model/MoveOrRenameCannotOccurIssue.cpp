#include "MoveOrRenameCannotOccurIssue.h"

#include "MegaApiSynchronizedRequest.h"
#include "SyncController.h"
#include "SyncInfo.h"
#include "SyncSettings.h"

#include <MegaApplication.h>
#include <MEGAPathCreator.h>
#include <QDir>
#include <StalledIssuesUtilities.h>
#include <StatsEventHandler.h>
#include <Utilities.h>

//////////////////////////////////////

const int MAX_RETRIES = 5;

MoveOrRenameCannotOccurIssue::MoveOrRenameCannotOccurIssue(const mega::MegaSyncStall* stall):
    StalledIssue(stall),
    mega::MegaRequestListener(),
    mSolvingStarted(false),
    mChosenSide(MoveOrRenameIssueChosenSide::NONE),
    mCombinedNumberOfIssues(1),
    mUndoSuccessful(0),
    mSolveAttempts(0)
{
}

bool MoveOrRenameCannotOccurIssue::isValid() const
{
    return consultCloudData() || consultLocalData();
}

// We don´t fill the issue as usual, we keep a list of issues to fix
void MoveOrRenameCannotOccurIssue::fillIssue(const mega::MegaSyncStall* stall)
{
    if (stall)
    {
        auto issue = std::make_shared<StalledIssue>(stall);
        issue->fillIssue(stall);
        auto choosableSides = calculateChoosableSidesByPathProblem(issue);

        if (choosableSides & MoveOrRenameIssueChosenSide::LOCAL)
        {
            mDetectedCloudSideIssuesToFix.append(issue);
        }

        if (choosableSides & MoveOrRenameIssueChosenSide::REMOTE)
        {
            mDetectedLocalSideIssuesToFix.append(issue);
        }

        // We fill the main issue just to have the basic issue info (sync ids...etc)
        if (!consultCloudData() && !consultLocalData())
        {
            StalledIssue::fillIssue(stall);
        }
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
    // If we select KEEP REMOTE, we undo the local changes, so we take the local issues to fix
    StalledIssuesList issuesToFix = getChosenSide() == MoveOrRenameIssueChosenSide::REMOTE ?
                                        mDetectedLocalSideIssuesToFix :
                                        mDetectedCloudSideIssuesToFix;

    if (!mSolvingStarted)
    {
        return;
    }

    mUndoSuccessful = issuesToFix.size();

    auto syncId(firstSyncId());
    foreach(auto& issueToFix, issuesToFix)
    {
        if (syncSettings->backupId() != syncId)
        {
            return;
        }

        const auto syncState = syncSettings->getRunState();
        if (syncState == mega::MegaSync::RUNSTATE_PAUSED ||
            syncState == mega::MegaSync::RUNSTATE_SUSPENDED)
        {
            bool stopSolvingIssue(false);

            // We perform the undo in the opposite side
            if (getChosenSide() == MoveOrRenameIssueChosenSide::REMOTE)
            {
                stopSolvingIssue = solveIssueByPathProblem(issueToFix);

                if (!stopSolvingIssue)
                {
                    solveRemoteGenericIssues(issueToFix);
                }
            }
            else if (getChosenSide() == MoveOrRenameIssueChosenSide::LOCAL && consultCloudData())
            {
                stopSolvingIssue = solveIssueByPathProblem(issueToFix);

                if (!stopSolvingIssue)
                {
                    solveLocalGenericIssues(issueToFix);
                }
            }

            MegaSyncApp->getMegaApi()->clearStalledPath(issueToFix->getOriginalStall().get());
        }
    }

    onUndoFinished(syncSettings);
}

void MoveOrRenameCannotOccurIssue::onUndoFinished(std::shared_ptr<SyncSettings> syncSettings)
{
    SyncController::instance().setSyncToRun(syncSettings);
    disconnect(SyncInfo::instance(),
        &SyncInfo::syncStateChanged,
        this,
        &MoveOrRenameCannotOccurIssue::onSyncPausedEnds);

    mSolvingStarted = false;

    mDetectedCloudSideIssuesToFix.clear();
    mDetectedLocalSideIssuesToFix.clear();

    if (mUndoSuccessful == 0)
    {
        mCloudData.reset();
        mLocalData.reset();
        mSolveAttempts = 0;
    }
}

bool MoveOrRenameCannotOccurIssue::solveIssueByPathProblem(StalledIssueSPtr issue)
{
    auto pathProblem(issue->getPathProblem());

    switch (pathProblem)
    {
        case mega::MegaSyncStall::SyncPathProblem::DestinationPathInUnresolvedArea:
        {
            return solveDestinationPathInUnresolvedArea(issue);
        }
        // The solve logic for SourceWasMovedElsewhere is the same as a normal issue
        case mega::MegaSyncStall::SyncPathProblem::SourceWasMovedElsewhere:
        {
            if (isSourceWasDeleted(issue))
            {
                solveSourceWasMovedToElsewhere(issue);
            }

            [[fallthrough]];
        }
        default:
        {
            return false;
        }
    }

    return false;
}

bool MoveOrRenameCannotOccurIssue::solveSourceWasMovedToElsewhere(StalledIssueSPtr issue)
{
    auto issueIsSolved(false);

    if (getChosenSide() == MoveOrRenameIssueChosenSide::LOCAL)
    {
        std::unique_ptr<mega::MegaNode> node(
            MegaSyncApp->getMegaApi()->getNodeByHandle(issue->getCloudData()->getPathHandle()));
        if (node)
        {
            bool async(false);
            Utilities::restoreNode(node.get(),
                                   MegaSyncApp->getMegaApi(),
                                   async,
                                   [this](mega::MegaRequest*, mega::MegaError* error) {
                                       if (error &&
                                           error->getErrorCode() == mega::MegaError::API_OK)
                                       {
                                           mUndoSuccessful--;
                                       }
                                   });

            issueIsSolved = true;
        }
    }
    else
    {
        QFileInfo localFile(issue->getLocalData()->getMoveFilePath());
        if (StalledIssuesUtilities::removeLocalFile(localFile.filePath(), firstSyncId()))
        {
            mUndoSuccessful--;
        }

        issueIsSolved = true;
    }

    return issueIsSolved;
}

bool MoveOrRenameCannotOccurIssue::solveDestinationPathInUnresolvedArea(StalledIssueSPtr issue)
{
    auto issueIsSolved(false);

    if (getChosenSide() == MoveOrRenameIssueChosenSide::LOCAL)
    {
        auto cloudData(issue->consultCloudData());
        auto localData(issue->consultLocalData());

        if (cloudData && (cloudData->getFilePath() == cloudData->getMoveFilePath() &&
                          localData->getMoveFilePath().isEmpty()))
        {
            std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(
                cloudData->getFilePath().toStdString().c_str()));
            if (node)
            {
                QFileInfo localFile(localData->getFilePath());
                localFile.setFile(localFile.path(), cloudData->getFileName());
                auto removeLocal =
                    StalledIssuesUtilities::removeLocalFile(localFile.filePath(), firstSyncId());

                if (removeLocal)
                {
                    auto error = MegaApiSynchronizedRequest::runRequest(
                        &mega::MegaApi::renameNode,
                        MegaSyncApp->getMegaApi(),
                        node.get(),
                        localData->getFileName().toStdString().c_str());

                    if (error == nullptr && removeLocal)
                    {
                        mUndoSuccessful--;
                    }
                }
            }

            issueIsSolved = true;
        }
    }

    return issueIsSolved;
}

void MoveOrRenameCannotOccurIssue::solveLocalGenericIssues(StalledIssueSPtr issue)
{
    auto cloudData(issue->consultCloudData());

    std::unique_ptr<mega::MegaNode> nodeToMove(
        MegaSyncApp->getMegaApi()->getNodeByHandle(cloudData->getMovePathHandle()));

    if (!nodeToMove)
    {
        nodeToMove.reset(MegaSyncApp->getMegaApi()->getNodeByPath(
            cloudData->getMovePath().path.toUtf8().constData()));
    }

    if (nodeToMove)
    {
        QFileInfo targetPath(cloudData->getNativeFilePath());
        std::unique_ptr<mega::MegaNode> newParent(
            MegaSyncApp->getMegaApi()->getNodeByPath(targetPath.path().toUtf8().constData()));

        if (!newParent)
        {
            std::shared_ptr<mega::MegaError> error(nullptr);
            MEGAPathCreator::mkDir(QString(), targetPath.path(), error);

            if (!error)
            {
                newParent.reset(MegaSyncApp->getMegaApi()->getNodeByPath(
                    targetPath.path().toUtf8().constData()));
            }
        }

        QByteArray byteArray = targetPath.fileName().toUtf8();
        const char* fileName = byteArray.constData();

        auto resultLambda = [this](mega::MegaRequest*, mega::MegaError* e) {
            if (e->getErrorCode() == mega::MegaError::API_OK)
            {
                mUndoSuccessful--;
            }
        };

        if (strcmp(nodeToMove->getName(), fileName) != 0)
        {
            MegaApiSynchronizedRequest::runRequestWithResult(&mega::MegaApi::renameNode,
                                                             MegaSyncApp->getMegaApi(),
                                                             resultLambda,
                                                             nodeToMove.get(),
                                                             fileName);
        }
        else if (newParent)
        {
            MegaApiSynchronizedRequest::runRequestLambdaWithResult(
                [](mega::MegaNode* node,
                   mega::MegaNode* targetNode,
                   mega::MegaRequestListener* listener) {
                    MegaSyncApp->getMegaApi()->moveNode(node, targetNode, listener);
                },
                MegaSyncApp->getMegaApi(),
                resultLambda,
                nodeToMove.get(),
                newParent.get());
        }
    }
}

bool MoveOrRenameCannotOccurIssue::isSourceWasDeleted(StalledIssueSPtr issue)
{
    auto node(issue->consultCloudData()->getNode());
    return (!node || node->getParentHandle() == MegaSyncApp->getRubbishNode()->getHandle());
}

ChoosableSides
    MoveOrRenameCannotOccurIssue::calculateChoosableSidesByPathProblem(StalledIssueSPtr issue)
{
    auto pathProblem(issue->getPathProblem());

    switch (pathProblem)
    {
        // The solve logic for SourceWasMovedElsewhere is the same as a normal issue
        case mega::MegaSyncStall::SyncPathProblem::SourceWasMovedElsewhere:
        {
            if (isSourceWasDeleted(issue))
            {
                return MoveOrRenameIssueChosenSide::REMOTE | MoveOrRenameIssueChosenSide::LOCAL;
            }

            [[fallthrough]];
            // If not, do default option
        }
        default:
        {
            return issue->detectedCloudSide() ? MoveOrRenameIssueChosenSide::LOCAL :
                                                MoveOrRenameIssueChosenSide::REMOTE;
        }
    }
}

void MoveOrRenameCannotOccurIssue::solveRemoteGenericIssues(StalledIssueSPtr issue)
{
    auto localData(issue->consultLocalData());

    QFileInfo previousPath(localData->getFilePath());
    QFileInfo previousDirectory(previousPath.absolutePath());
    QString currentPath(localData->getNativeMoveFilePath());

    auto result = previousDirectory.exists();

    if (!result)
    {
        result = QDir().mkpath(previousDirectory.absolutePath());
    }

    if (result)
    {
        QFile file(currentPath);
        result = file.rename(previousPath.absoluteFilePath());
    }

    if (!result)
    {
        mFailedLocalPaths.insert(currentPath);
    }

    if (result)
    {
        mUndoSuccessful--;
    }
}

bool MoveOrRenameCannotOccurIssue::solveAttemptsAchieved() const
{
    return mSolveAttempts >= MAX_RETRIES;
}

StalledIssue::AutoSolveIssueResult MoveOrRenameCannotOccurIssue::autoSolveIssue()
{
    auto chosenSide(getSyncIdChosenSide());
    if(isAutoSolvable() && !(chosenSide == MoveOrRenameIssueChosenSide::NONE))
    {
        solveIssue(chosenSide);
        return StalledIssue::AutoSolveIssueResult::ASYNC_SOLVED;
    }

    return StalledIssue::AutoSolveIssueResult::FAILED;
}

bool MoveOrRenameCannotOccurIssue::isKeepSideAvailable(MoveOrRenameIssueChosenSide side) const
{
    auto size = side == MoveOrRenameIssueChosenSide::LOCAL ? mDetectedCloudSideIssuesToFix.size() :
                                                             mDetectedLocalSideIssuesToFix.size();
    return size != 0;
}

bool MoveOrRenameCannotOccurIssue::checkForExternalChanges()
{
    if (!isSolved())
    {
        auto checkIssue = [this](StalledIssueSPtr issue) -> bool {
            // Check if we can do a Keep local
            if (issue->detectedCloudSide())
            {
                auto cloudData(issue->consultCloudData());

                if (cloudData)
                {
                    if (!cloudData->getPath().path.isEmpty() ||
                        !cloudData->getMovePath().path.isEmpty())
                    {
                        std::unique_ptr<mega::MegaNode> previousNode(
                            MegaSyncApp->getMegaApi()->getNodeByPath(
                                cloudData->getPath().path.toStdString().c_str()));
                        std::unique_ptr<mega::MegaNode> currentNode(
                            MegaSyncApp->getMegaApi()->getNodeByPath(
                                cloudData->getMovePath().path.toStdString().c_str()));
                        if (previousNode && !currentNode)
                        {
                            setIsSolved(StalledIssue::SolveType::POTENTIALLY_SOLVED);
                        }
                    }
                }
            }
            else
            {
                auto localData(issue->consultLocalData());

                // Check if we can do a Keep remote
                if (localData)
                {
                    QFileInfo previousPath(localData->getPath().path);
                    QFileInfo currentPath(localData->getMovePath().path);
                    if (previousPath.exists() && !currentPath.exists())
                    {
                        setIsSolved(StalledIssue::SolveType::POTENTIALLY_SOLVED);
                    }
                }
            }

            return isPotentiallySolved();
        };

        foreach(auto issue, mDetectedCloudSideIssuesToFix)
        {
            if (checkIssue(issue))
            {
                return isPotentiallySolved();
            }
        }

        foreach(auto issue, mDetectedLocalSideIssuesToFix)
        {
            if (checkIssue(issue))
            {
                return isPotentiallySolved();
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
    StalledIssue::performFinishAsyncIssueSolving(mUndoSuccessful != 0);
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

StalledIssueSPtr MoveOrRenameCannotOccurFactory::createIssue(MultiStepIssueSolverBase* solver,
                                                             const mega::MegaSyncStall* stall)
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
                moveIssue = std::make_shared<MoveOrRenameCannotOccurIssue>(stall);
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
