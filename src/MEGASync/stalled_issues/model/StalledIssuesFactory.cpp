#include "StalledIssuesFactory.h"

#include "FolderMatchedAgainstFileIssue.h"
#include "IgnoredStalledIssue.h"
#include "LocalOrRemoteUserMustChooseStalledIssue.h"
#include "MegaApplication.h"
#include "MoveOrRenameCannotOccurIssue.h"
#include "NameConflictStalledIssue.h"
#include "StatsEventHandler.h"

StalledIssuesFactory::StalledIssuesFactory()
{
    qRegisterMetaType<ReceivedStalledIssues>("ReceivedStalledIssues");
}

//////////////////////////////////
StalledIssuesCreator::StalledIssuesCreator():
    mMoveOrRenameCannotOccurFactory(std::make_shared<MoveOrRenameCannotOccurFactory>())
{
    qRegisterMetaType<IssuesCount>("IssuesCount");
    qRegisterMetaType<UpdateType>("UpdateType");
}

void StalledIssuesCreator::createIssues(const mega::MegaSyncStallMap* stallsMap,
                                        UpdateType updateType)
{
    if (stallsMap)
    {
        struct SolvableIssues
        {
            StalledIssueVariant variant;
            bool solvedSynchronously = true;
        };

        QList<SolvableIssues> solvableIssues;

        QSet<mega::MegaSyncStall::SyncStallReason> reasonsToFilter;
        QList<MultiStepIssueSolverBase*> solversWithStall;

        start();

        QHash<size_t, StalledIssueVariant> processedStalledIssues;

        auto syncIds(stallsMap->getKeys());
        for (unsigned int index = 0; index < syncIds->size(); ++index)
        {
            auto syncId(syncIds->get(index));

            auto stalls(stallsMap->get(syncId));

            for (size_t i = 0; i < stalls->size(); ++i)
            {
                auto stall = stalls->get(i);

                auto hash(stall->getHash());
                if (processedStalledIssues.contains(hash))
                {
                    auto variant(processedStalledIssues[hash]);

                    if (variant.addSyncId(syncId))
                    {
                        // Log as it is not supposed to happen
                        QString syncIdsMessage;
                        for (const auto& syncId: variant.syncIds())
                        {
                            if (!syncIdsMessage.isEmpty())
                            {
                                syncIdsMessage.append(QLatin1String(" "));
                            }
                            syncIdsMessage.append(QString::number(syncId));
                        }

                        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                                           QString::fromUtf8("Repeated stalled issue received with "
                                                             "different syncID. SyncIds: %1")
                                               .arg(syncIdsMessage)
                                               .toUtf8()
                                               .constData());
                    }
                    else
                    {
                        mega::MegaApi::log(
                            mega::MegaApi::LOG_LEVEL_WARNING,
                            QString::fromUtf8(
                                "Repeated stalled issue received with the same syncID. SyncId: %1")
                                .arg(QString::number(syncId))
                                .toUtf8()
                                .constData());
                    }

                    continue;
                }

                // Just in case this is not the first issue of a multistep issue solver
                auto multiStepIssueSolver(getMultiStepIssueSolverByStall(stall, syncId));

                if (multiStepIssueSolver)
                {
                    solversWithStall.append(multiStepIssueSolver);
                }

                StalledIssueVariant variant;
                StalledIssueSPtr d;

                if (stall->reason() ==
                    mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur)
                {
                    d = mMoveOrRenameCannotOccurFactory->createIssue(multiStepIssueSolver, stall);

                    // If we find a MoveOrRenameCannotOccur issue, we don´t want to show
                    // the DeleteWaitingOnMove and DeleteOrMoveWaitingOnScanning
                    reasonsToFilter
                        << mega::MegaSyncStall::SyncStallReason::DeleteWaitingOnMoves
                        << mega::MegaSyncStall::SyncStallReason::DeleteOrMoveWaitingOnScanning;
                }
                else
                {
                    if (stall->reason() ==
                        mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced)
                    {
                        d = std::make_shared<NameConflictedStalledIssue>(stall);
                    }
                    else if (stall->couldSuggestIgnoreThisPath(false, 0) ||
                             stall->couldSuggestIgnoreThisPath(false, 1) ||
                             stall->couldSuggestIgnoreThisPath(true, 0) ||
                             stall->couldSuggestIgnoreThisPath(true, 1))
                    {
                        d = std::make_shared<IgnoredStalledIssue>(stall);
                    }
                    else if (StalledIssue::isCloudNodeBlocked(stall))
                    {
                        d = std::make_shared<CloudNodeIsBlockedIssue>(stall);
                    }
                    else if (stall->reason() ==
                             mega::MegaSyncStall::SyncStallReason::FolderMatchedAgainstFile)
                    {
                        d = std::make_shared<FolderMatchedAgainstFileIssue>(stall);
                    }
                    else if (stall->reason() ==
                                 mega::MegaSyncStall::SyncStallReason::
                                     LocalAndRemoteChangedSinceLastSyncedState_userMustChoose ||
                             stall->reason() ==
                                 mega::MegaSyncStall::SyncStallReason::
                                     LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose)
                    {
                        d = std::make_shared<LocalOrRemoteUserMustChooseStalledIssue>(stall);
                    }
                    else
                    {
                        d = std::make_shared<StalledIssue>(stall);
                    }
                }

                if (d)
                {
                    variant = StalledIssueVariant(d, stall);
                }

                if (!variant.isValid() || variant.shouldBeIgnored())
                {
                    continue;
                }

                processedStalledIssues.insert(hash, variant);
                variant.addSyncId(syncId);

                // If multiStepIssueSolver is nullptr but a new one was created and added in the
                // previous line, we don´t need to reset it
                if (multiStepIssueSolver && updateType == UpdateType::AUTO_SOLVE)
                {
                    multiStepIssueSolver->resetDeadlineIfNeeded(variant);
                }

                // Check if it is being solved...
                if (!variant.getData()->isSolved())
                {
                    // Init issue file/folder attributes, needed to check if the issue is
                    // autosolvable
                    variant.getData()->endFillingIssue();

                    if (updateType == UpdateType::EVENT)
                    {
                        if (!variant.getData()->isAutoSolvable())
                        {
                            QString eventMessage(
                                QString::fromLatin1("Stalled issue received: Type %1")
                                    .arg(QString::number(stall->reason())));
                            MegaSyncApp->getStatsEventHandler()->sendEvent(
                                AppStatsEvents::EventType::SI_STALLED_ISSUE_RECEIVED,
                                QStringList() << eventMessage);
                        }

                        // We don´t work with these issues, they are just needed to send the event
                        continue;
                    }
                    else
                    {
                        if (variant.getData()->isAutoSolvable())
                        {
                            SolvableIssues issueToSolve;
                            issueToSolve.variant = variant;

                            // For the moment MoveOrRenameCannotOccur are the only issues solved
                            // asynchronously
                            issueToSolve.solvedSynchronously =
                                stall->reason() !=
                                mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur;

                            solvableIssues.append(issueToSolve);
                        }
                        else if (updateType == UpdateType::UI)
                        {
                            mStalledIssues.mActiveStalledIssues.append(variant);
                        }
                    }
                }
            }
        }
        // Add being solved issues taken from the MultiStepIssueSolvers
        if (updateType == UpdateType::UI)
        {
            for (auto it = mMultiStepIssueSolversByReason.keyValueBegin();
                 it != mMultiStepIssueSolversByReason.keyValueEnd();
                 ++it)
            {
                if (it->second->isActive() && !solversWithStall.contains(it->second))
                {
                    auto issue(it->second->getIssue());
                    auto variant = StalledIssueVariant(issue, issue->getOriginalStall().get());
                    variant.getData()->endFillingIssue();
                    mStalledIssues.mActiveStalledIssues.append(variant);
                }
            }
        }

        // Solve autosolvable issues
        IssuesCount solvingIssuesStats;
        solvingIssuesStats.totalIssues = solvableIssues.size();
        solvingIssuesStats.currentIssueBeingSolved = 1;

        foreach(auto solvableIssueInfo, solvableIssues)
        {
            auto solvableIssue(solvableIssueInfo.variant);

            // Blocks the UI and displays a message
            if (solvableIssueInfo.solvedSynchronously)
            {
                emit solvingIssues(solvingIssuesStats);
            }

            auto result(solvableIssue.getData()->autoSolveIssue());

            // Keeps the count of fixed and failed issues
            if (solvableIssueInfo.solvedSynchronously)
            {
                if (result == StalledIssue::AutoSolveIssueResult::FAILED)
                {
                    solvableIssue.getData()->setIsSolved(StalledIssue::SolveType::FAILED);
                    mStalledIssues.mFailedAutoSolvedStalledIssues.append(solvableIssue);
                    solvingIssuesStats.issuesFailed++;
                }
                else if (result == StalledIssue::AutoSolveIssueResult::SOLVED)
                {
                    solvableIssue.getData()->setIsSolved(StalledIssue::SolveType::SOLVED);
                    mStalledIssues.mAutoSolvedStalledIssues.append(solvableIssue);
                    solvingIssuesStats.issuesFixed++;
                }

                solvingIssuesStats.currentIssueBeingSolved++;
            }
        }

        // Only synchronously solved issues increase the issuesFailed and issuesFixed count
        // Even if this signal is sents after solving async issues, it only hides the message
        if ((solvingIssuesStats.issuesFailed + solvingIssuesStats.issuesFixed) != 0)
        {
            emit solvingIssuesFinished(solvingIssuesStats);
        }
        // Finish solving autosolvable issues

        //Filter some issues depending on what you have found on the list or if the issue is invalid
        //We don´t filter the solvable issues as these issues must be solved and not filtered
        QMutableListIterator<StalledIssueVariant> issueIt(mStalledIssues.mActiveStalledIssues);
        while(issueIt.hasNext())
        {
            auto issueToCheck(issueIt.next());
            if(!issueToCheck.consultData()->isValid() ||
                reasonsToFilter.contains(issueToCheck.getData()->getReason()))
            {
                issueIt.remove();
            }
        }

        finish();
    }
}

bool StalledIssuesCreator::multiStepIssueSolveActive() const
{
    for (auto it = mMultiStepIssueSolversByReason.keyValueBegin(); it != mMultiStepIssueSolversByReason.keyValueEnd(); ++it)
    {
        if(it->second->isActive())
        {
            return true;
        }
    }

    return false;
}

void StalledIssuesCreator::start()
{
    mStalledIssues.clear();
    mMoveOrRenameCannotOccurFactory->clear();
}

void StalledIssuesCreator::finish()
{
    mMoveOrRenameCannotOccurFactory->finish();
}

void StalledIssuesCreator::addMultiStepIssueSolver(MultiStepIssueSolverBase* solver)
{
    if(solver)
    {
        auto issue(solver->getIssue());
        auto reason(issue->getReason());

        connect(solver,
            &MultiStepIssueSolverBase::solverFinished,
            this,
            [this, reason](MultiStepIssueSolverBase* solver)
            {
                mMultiStepIssueSolversByReason.remove(reason, solver);
                //Send finish notification only when the last one has finished
                if(!mMultiStepIssueSolversByReason.contains(reason))
                {
                    solver->sendFinishNotification();
                }
                solver->deleteLater();
            });

        mMultiStepIssueSolversByReason.insert(reason, solver);
    }
}

ReceivedStalledIssues StalledIssuesCreator::getStalledIssues() const
{
    return mStalledIssues;
}

QPointer<MultiStepIssueSolverBase>
    StalledIssuesCreator::getMultiStepIssueSolverByStall(const mega::MegaSyncStall* stall,
                                                         mega::MegaHandle syncId)
{
    if (!mMultiStepIssueSolversByReason.isEmpty())
    {
        auto solverFound = std::find_if(mMultiStepIssueSolversByReason.begin(),
                                        mMultiStepIssueSolversByReason.end(),
                                        [stall, syncId](const MultiStepIssueSolverBase* solver)
                                        {
                                            return solver->checkIssue(stall, syncId);
                                        });

        if (solverFound != mMultiStepIssueSolversByReason.end() )
        {
            return (*solverFound);
        }
    }

    return nullptr;
}
