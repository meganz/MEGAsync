#include "StalledIssuesFactory.h"

#include <MegaApplication.h>

#include <NameConflictStalledIssue.h>
#include <LocalOrRemoteUserMustChooseStalledIssue.h>
#include <IgnoredStalledIssue.h>
#include <MoveOrRenameCannotOccurIssue.h>

#include <mega/types.h>

StalledIssuesCreator::StalledIssuesCreator() :
    mMoveOrRenameCannotOccurFactory(std::make_shared<MoveOrRenameCannotOccurFactory>())
{
}

void StalledIssuesCreator::createIssues(mega::MegaSyncStallList* stalls, UpdateType updateType, const QMultiMap<mega::MegaSyncStall::SyncStallReason,
                                                                                                    QPointer<MultiStepIssueSolverBase>>& multiStepIssueSolversByReason)
{
    clear();

    if(stalls)
    {
        StalledIssuesVariantList solvableIssues;
        auto totalSize(stalls->size());

        QSet<mega::MegaSyncStall::SyncStallReason> reasonsToFilter;

        for(size_t i = 0; i < totalSize; ++i)
        {
            auto stall = stalls->get(i);
            StalledIssueVariant variant;
            std::shared_ptr<StalledIssue> d;

            if(stall->reason() == mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur)
            {
                d = mMoveOrRenameCannotOccurFactory->createIssue(stall);
                //If we find a MoveOrRenameCannotOccur issue, we don´t want to show
                //the DeleteWaitingOnMove and DeleteOrMoveWaitingOnScanning
                reasonsToFilter << mega::MegaSyncStall::SyncStallReason::DeleteWaitingOnMoves
                                << mega::MegaSyncStall::SyncStallReason::DeleteOrMoveWaitingOnScanning;
            }
            else
            {
                if(stall->reason() ==
                    mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced)
                {
                    d = std::make_shared<NameConflictedStalledIssue>(stall);
                }
                else if(stall->couldSuggestIgnoreThisPath(false, 0) ||
                        stall->couldSuggestIgnoreThisPath(false, 1) ||
                        stall->couldSuggestIgnoreThisPath(true, 0) ||
                        stall->couldSuggestIgnoreThisPath(true, 1))
                {
                    d = std::make_shared<IgnoredStalledIssue>(stall);
                }
                else if(stall->reason() ==
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

            if(d)
            {
                variant = StalledIssueVariant(d, stall);
            }

            if(!variant.isValid() || variant.shouldBeIgnored())
            {
                continue;
            }

            auto multiStepIssueSolver(
                getMultiStepIssueSolverByStall(multiStepIssueSolversByReason, d));
            if(multiStepIssueSolver && updateType == UpdateType::AUTO_SOLVE)
            {
                multiStepIssueSolver->resetDeadlineIfNeeded(variant);
            }

            //Check if it is being solved...
            if(!variant.getData()->isSolved())
            {
                variant.getData()->endFillingIssue();

                if(updateType == UpdateType::EVENT)
                {
                    if(!variant.getData()->isAutoSolvable())
                    {
                        QString eventMessage(QString::fromLatin1("Stalled issue received: Type %1")
                                                 .arg(QString::number(stall->reason())));
                        MegaSyncApp->getStatsEventHandler()->sendEvent(
                            AppStatsEvents::EVENT_SI_STALLED_ISSUE_RECEIVED,
                            eventMessage.toUtf8().constData());
                    }
                }
                else
                {
                    if(variant.getData()->isAutoSolvable())
                    {
                        solvableIssues.append(variant);
                    }
                    else if(updateType == UpdateType::UI)
                    {
                        mIssues.append(variant);
                    }
                }
            }
        }

        auto solvableTotalIssues(solvableIssues.size());
        auto counter(1);
        foreach(auto solvableIssue, solvableIssues)
        {
            emit solvingIssues(counter, solvableTotalIssues);
            auto hasBeenSolved(solvableIssue.getData()->autoSolveIssue());

            if(updateType == UpdateType::UI && !hasBeenSolved)
            {
                mIssues.append(solvableIssue);
            }

            counter++;
        }

        //Filter some issues depending on what you have found on the list
        //We don´t filter the solvable issues as these issues must be solved and not filtered
        if(!reasonsToFilter.isEmpty())
        {
            QMutableListIterator<StalledIssueVariant> issueIt(mIssues);
            while(issueIt.hasNext())
            {
                if(reasonsToFilter.contains(issueIt.next().getData()->getReason()))
                {
                    issueIt.remove();
                }
            }
        }
    }
}

StalledIssuesVariantList StalledIssuesCreator::issues() const
{
    return mIssues;
}

void StalledIssuesCreator::clear()
{
    mIssues.clear();
    mMoveOrRenameCannotOccurFactory->clear();
}

QPointer<MultiStepIssueSolverBase> StalledIssuesCreator::getMultiStepIssueSolverByStall(
    const QMultiMap<mega::MegaSyncStall::SyncStallReason, QPointer<MultiStepIssueSolverBase>>&
        multiStepIssueSolversByReason,
    std::shared_ptr<StalledIssue> issue)
{
    auto multiStepIssueSolvers(multiStepIssueSolversByReason.values(issue->getReason()));
    if (!multiStepIssueSolvers.isEmpty())
    {
        auto solverFound = std::find_if(multiStepIssueSolvers.begin(),
            multiStepIssueSolvers.end(),
            [issue](const QPointer<MultiStepIssueSolverBase>& solver)
            { return solver->checkIssue(issue); });
        if (solverFound != multiStepIssueSolvers.end())
        {
            return (*solverFound);
        }
    }

    return nullptr;
}
