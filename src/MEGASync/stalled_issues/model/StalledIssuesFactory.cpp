#include "StalledIssuesFactory.h"

#include <MegaApplication.h>

#include <NameConflictStalledIssue.h>
#include <LocalOrRemoteUserMustChooseStalledIssue.h>
#include <IgnoredStalledIssue.h>
#include <MoveOrRenameCannotOccurIssue.h>
#include <StatsEventHandler.h>

#include <mega/types.h>

StalledIssuesCreator::StalledIssuesCreator() :
    mMoveOrRenameCannotOccurFactory(std::make_shared<MoveOrRenameCannotOccurFactory>())
{
}

void StalledIssuesCreator::createIssues(mega::MegaSyncStallList* stalls, UpdateType updateType)
{
    clear();

    if(stalls)
    {
        StalledIssuesVariantList solvableIssues;
        auto totalSize(stalls->size());

        QSet<mega::MegaSyncStall::SyncStallReason> reasonsToFilter;
        QList<MultiStepIssueSolverBase*> solversWithStall;

        for(size_t i = 0; i < totalSize; ++i)
        {
            auto stall = stalls->get(i);

            //Just in case this is not the first issue of a multistep issue solver
            auto multiStepIssueSolver(getMultiStepIssueSolverByStall(stall));

            if(multiStepIssueSolver)
            {
                solversWithStall.append(multiStepIssueSolver);
            }

            StalledIssueVariant variant;
            std::shared_ptr<StalledIssue> d;

            if(stall->reason() == mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur)
            {
                d = mMoveOrRenameCannotOccurFactory->createIssue(multiStepIssueSolver, stall);

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

            if(d && d->isValid())
            {
                variant = StalledIssueVariant(d, stall);
            }

            if(!variant.isValid() || variant.shouldBeIgnored())
            {
                continue;
            }

            //If multiStepIssueSolver is nullptr but a new one was created and added in the previous line, we don´t need to reset it
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
                            AppStatsEvents::EventType::SI_STALLED_ISSUE_RECEIVED, QStringList() << eventMessage);
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
                        mPendingIssues.append(variant);
                    }
                }
            }
        }

        //Add being solved issues taken from the MultiStepIssueSolvers
        for (auto it = mMultiStepIssueSolversByReason.keyValueBegin(); it != mMultiStepIssueSolversByReason.keyValueEnd(); ++it)
        {
            if(it->second->isActive() && !solversWithStall.contains(it->second))
            {
                auto issue(it->second->getIssue());
                auto variant = StalledIssueVariant(issue, issue->getOriginalStall().get());
                variant.getData()->endFillingIssue();
                mPendingIssues.append(variant);
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
                mPendingIssues.append(solvableIssue);
            }

            counter++;
        }

        //Filter some issues depending on what you have found on the list
        //We don´t filter the solvable issues as these issues must be solved and not filtered
        if(!reasonsToFilter.isEmpty())
        {
            QMutableListIterator<StalledIssueVariant> issueIt(mPendingIssues);
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
    return mPendingIssues;
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

void StalledIssuesCreator::clear()
{
    mPendingIssues.clear();
    mMoveOrRenameCannotOccurFactory->clear();
}

void StalledIssuesCreator::addMultiStepIssueSolver(MultiStepIssueSolverBase* solver)
{
    if(solver)
    {
        auto issue(solver->getIssue());
        auto reason(issue->getReason());

        connect(solver,
            &QObject::destroyed,
            this,
            [this, solver, reason](QObject*)
            {
                mMultiStepIssueSolversByReason.remove(reason, solver);
            });

        mMultiStepIssueSolversByReason.insert(reason, solver);
    }
}

QPointer<MultiStepIssueSolverBase> StalledIssuesCreator::getMultiStepIssueSolverByStall(const mega::MegaSyncStall* stall)
{
    if (!mMultiStepIssueSolversByReason.isEmpty())
    {
        auto solverFound = std::find_if(mMultiStepIssueSolversByReason.begin(),
            mMultiStepIssueSolversByReason.end(),
            [stall](const MultiStepIssueSolverBase* solver)
            { return solver->checkIssue(stall); });

        if (solverFound != mMultiStepIssueSolversByReason.end() )
        {
            return (*solverFound);
        }
    }

    return nullptr;
}
