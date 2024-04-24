#include "StalledIssuesFactory.h"

#include <MegaApplication.h>

#include <NameConflictStalledIssue.h>
#include <LocalOrRemoteUserMustChooseStalledIssue.h>
#include <IgnoredStalledIssue.h>
#include <MoveOrRenameCannotOccurIssue.h>

StalledIssuesFactory::StalledIssuesFactory(bool isEventRequest)
    : mIsEventRequest(isEventRequest)
{
}

void StalledIssuesFactory::createIssues(mega::MegaSyncStallList* stalls)
{
    StalledIssuesVariantList solvableIssues;

    if (stalls)
    {
        auto totalSize(stalls->size());

        for (size_t i = 0; i < totalSize; ++i)
        {
            auto stall = stalls->get(i);
            StalledIssueVariant variant;
            std::shared_ptr<StalledIssue> d;

            if(stall->reason() == mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced)
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
            else if(stall->reason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose
                     || stall->reason() == mega::MegaSyncStall::SyncStallReason::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose)
            {
                d = std::make_shared<LocalOrRemoteUserMustChooseStalledIssue>(stall);
            }
            else if(stall->reason() == mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur)
            {
                d = std::make_shared<MoveOrRenameCannotOccurIssue>(stall);
            }
            else
            {
                d = std::make_shared<StalledIssue>(stall);
            }

            variant = StalledIssueVariant(d, stall);

            if(variant.shouldBeIgnored())
            {
                continue;
            }

            //Check if it is being solved...
            if(!variant.getData()->isSolved())
            {
                variant.getData()->endFillingIssue();

                if(mIsEventRequest)
                {
                    if(!variant.getData()->isSolvable())
                    {
                        QString eventMessage(QString::fromLatin1("Stalled issue received: Type %1").arg(QString::number(stall->reason())));
                        MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EVENT_SI_STALLED_ISSUE_RECEIVED, eventMessage.toUtf8().constData());
                    }
                }
                else
                {
                    if(variant.getData()->isSolvable())
                    {
                        solvableIssues.append(variant);
                    }
                    else
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
            if(Preferences::instance()->stalledIssuesMode() == Preferences::StalledIssuesModeType::Smart)
            {
                emit solvingIssues(counter, solvableTotalIssues);
                solvableIssue.getData()->autoSolveIssue();
            }

            if(!solvableIssue.getData()->isSolved())
            {
                mIssues.append(solvableIssue);
            }

            counter++;
        }
    }
}

StalledIssuesVariantList StalledIssuesFactory::issues() const
{
    return mIssues;
}
