#include "MoveOrRenameCannotOccurIssue.h"

#include "MegaApiSynchronizedRequest.h"
#include "MegaApplication.h"
#include "MEGAPathCreator.h"
#include "MergeMEGAFolders.h"
#include "StalledIssuesUtilities.h"
#include "StatsEventHandler.h"
#include "SyncController.h"
#include "SyncInfo.h"
#include "SyncSettings.h"
#include "Utilities.h"

#include <QDir>

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
    StalledIssuesList issuesToFix =
        remoteSideWasChosen() ? mDetectedLocalSideIssuesToFix : mDetectedCloudSideIssuesToFix;

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

            stopSolvingIssue = solveIssueByPathProblem(issueToFix);

            if (!stopSolvingIssue)
            {
                remoteSideWasChosen() ? solveRemoteGenericIssues(issueToFix) :
                                        solveLocalGenericIssues(issueToFix);
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
        case mega::MegaSyncStall::SyncPathProblem::ParentFolderDoesNotExist:
        {
            return solveParentFolderDoesNotExist(issue);
        }
        // The solve logic for SourceWasMovedElsewhere is the same as a normal issue
        case mega::MegaSyncStall::SyncPathProblem::SourceWasMovedElsewhere:
        {
            if (wasSourceDeleted(issue))
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

    if (localSideWasChosen())
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
        if (Utilities::removeLocalFile(issue->getLocalData()->getMoveFilePath(),
                                                    firstSyncId()))
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

    if (localSideWasChosen())
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
                    Utilities::removeLocalFile(localFile.filePath(), firstSyncId());

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

bool MoveOrRenameCannotOccurIssue::solveParentFolderDoesNotExist(StalledIssueSPtr issue)
{
    auto issueIsSolved(false);

    if (remoteSideWasChosen())
    {
        if (!issue->detectedCloudSide())
        {
            issueIsSolved = solveRemoteGenericIssues(issue);
        }
        else
        {
            auto localData(issue->consultLocalData());

            QFileInfo currentPath(localData->getNativeMoveFilePath());
            QFileInfo currentPathDirectory(currentPath.absolutePath());

            // If already exists, fixed
            issueIsSolved = currentPathDirectory.exists() ||
                            QDir().mkpath(currentPathDirectory.absoluteFilePath());

            if (issueIsSolved)
            {
                mUndoSuccessful--;
            }
        }
    }
    else
    {
        if (issue->detectedCloudSide())
        {
            issueIsSolved = solveLocalGenericIssues(issue);
        }
        else
        {
            auto cloudData(issue->consultCloudData());

            QFileInfo currentPath(cloudData->getNativeMoveFilePath());

            std::unique_ptr<mega::MegaNode> node(
                MegaSyncApp->getMegaApi()->getNodeByPath(currentPath.path().toStdString().c_str()));

            if (!node)
            {
                // Try to restore removed node by path
                // If not possible, mark it as failed
                issueIsSolved = Utilities::restoreNode(currentPath.fileName().toStdString().c_str(),
                                                       currentPath.path().toStdString().c_str(),
                                                       MegaSyncApp->getMegaApi(),
                                                       nullptr);
            }
            else
            {
                // Already exists, fixed
                issueIsSolved = true;
            }

            if (issueIsSolved)
            {
                mUndoSuccessful--;
            }
        }
    }

    return issueIsSolved;
}

bool MoveOrRenameCannotOccurIssue::solveLocalGenericIssues(StalledIssueSPtr issue)
{
    auto currentUndoSucessful(mUndoSuccessful);

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

        std::shared_ptr<mega::MegaNode> targetNode(MegaSyncApp->getMegaApi()->getNodeByPath(
            targetPath.absoluteFilePath().toUtf8().constData()));

        QByteArray byteArray = targetPath.fileName().toUtf8();
        const char* fileName = byteArray.constData();

        auto resultLambda = [this, targetNode](mega::MegaRequest*, mega::MegaError* e) {
            if (e->getErrorCode() == mega::MegaError::API_OK)
            {
                if (targetNode && targetNode->isFolder())
                {
                    // Don´t handle error, if it fails, we will have a name conflict, but nothing
                    // important
                    MergeMEGAFolders::merge(
                        targetNode.get(),
                        nullptr,
                        MergeMEGAFolders::ActionForDuplicates::IgnoreAndMoveToBin);
                }

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

    return currentUndoSucessful != mUndoSuccessful;
}

bool MoveOrRenameCannotOccurIssue::wasSourceDeleted(StalledIssueSPtr issue) const
{
    auto node(issue->consultCloudData()->getNode());
    return (!node || node->getParentHandle() == MegaSyncApp->getRubbishNode()->getHandle());
}

bool MoveOrRenameCannotOccurIssue::areInTheSameDirectory(StalledIssueSPtr issue)
{
    if (issue->detectedCloudSide())
    {
        QFileInfo sourcePath(issue->consultLocalData()->getNativeFilePath());
        QFileInfo targetPath(issue->consultLocalData()->getNativeMoveFilePath());

        return sourcePath.absolutePath() == targetPath.absolutePath();
    }
    else
    {
        QFileInfo sourcePath(issue->consultCloudData()->getNativeFilePath());
        QFileInfo targetPath(issue->consultCloudData()->getNativeMoveFilePath());

        return sourcePath.absolutePath() == targetPath.absolutePath();
    }
}

bool MoveOrRenameCannotOccurIssue::remoteSideWasChosen() const
{
    return getChosenSide() == MoveOrRenameIssueChosenSide::REMOTE;
}

bool MoveOrRenameCannotOccurIssue::localSideWasChosen() const
{
    return getChosenSide() == MoveOrRenameIssueChosenSide::LOCAL;
}

ChoosableSides
    MoveOrRenameCannotOccurIssue::calculateChoosableSidesByPathProblem(StalledIssueSPtr issue)
{
    auto pathProblem(issue->getPathProblem());

    auto defaultOption = [](StalledIssueSPtr issue) -> ChoosableSides {
        return issue->detectedCloudSide() ? MoveOrRenameIssueChosenSide::LOCAL :
                                            MoveOrRenameIssueChosenSide::REMOTE;
    };

    switch (pathProblem)
    {
        case mega::MegaSyncStall::SyncPathProblem::ParentFolderDoesNotExist:
        {
            // We offer both sides if we can recreate the parent folder structure
            if (!areInTheSameDirectory(issue))
            {
                return MoveOrRenameIssueChosenSide::REMOTE | MoveOrRenameIssueChosenSide::LOCAL;
            }

            return defaultOption(issue);
        }
        case mega::MegaSyncStall::SyncPathProblem::SourceWasMovedElsewhere:
        {
            if (wasSourceDeleted(issue))
            {
                return MoveOrRenameIssueChosenSide::REMOTE | MoveOrRenameIssueChosenSide::LOCAL;
            }

            return defaultOption(issue);
        }
        default:
        {
            return defaultOption(issue);
        }
    }
}

bool MoveOrRenameCannotOccurIssue::solveRemoteGenericIssues(StalledIssueSPtr issue)
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
        if (previousPath.exists())
        {
            Utilities::removeLocalFile(localData->getFilePath(), firstSyncId());
        }

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

    return result;
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
    if (syncIds.isEmpty())
    {
        return nullptr;
    }

    auto syncId(*syncIds.begin());
    auto syncSetting(SyncInfo::instance()->getSyncSettingByTag(syncId));
    if (!syncSetting)
    {
        return nullptr;
    }

    if (syncSetting->getType() == mega::MegaSync::SyncType::TYPE_TWOWAY)
    {
        if (mIssueBySyncId.contains(syncId))
        {
            auto previousIssue(mIssueBySyncId.value(syncId));
            previousIssue->fillIssue(stall);
        }
        else
        {
            std::shared_ptr<MoveOrRenameCannotOccurIssue> moveIssue(nullptr);

            if (solver)
            {
                auto moveOrRenameSolver(dynamic_cast<MoveOrRenameMultiStepIssueSolver*>(solver));
                if (moveOrRenameSolver)
                {
                    moveIssue = moveOrRenameSolver->getIssue();
                    moveIssue->increaseCombinedNumberOfIssues();
                }
            }
            else if (syncId != mega::INVALID_HANDLE)
            {
                moveIssue = std::make_shared<MoveOrRenameCannotOccurIssue>(stall);
            }

            if (moveIssue)
            {
                mIssueBySyncId.insert(syncId, moveIssue);
                return moveIssue;
            }
        }
    }
    else
    {
        mBackupSyncsDetected.insert(syncId);
    }

    // We don´t want to add it to the model, just update it
    return nullptr;
}

void MoveOrRenameCannotOccurFactory::clear()
{
    mIssueBySyncId.clear();
}

void MoveOrRenameCannotOccurFactory::finish()
{
    // If backups are detected, disabled and enabled backups
    foreach(auto& backupId, mBackupSyncsDetected)
    {
        auto backupSettings = SyncInfo::instance()->getSyncSettingByTag(backupId);
        if (backupSettings)
        {
            SyncController::instance().resetSync(
                backupSettings,
                mega::MegaSync::SyncRunningState::RUNSTATE_DISABLED);
        }
    }

    mBackupSyncsDetected.clear();
}

QSet<mega::MegaHandle> MoveOrRenameCannotOccurFactory::backupSyncsDetected() const
{
    return mBackupSyncsDetected;
}
