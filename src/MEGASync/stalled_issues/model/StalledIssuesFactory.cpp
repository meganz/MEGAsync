#include "StalledIssuesFactory.h"

#include <MegaApplication.h>

#include <NameConflictStalledIssue.h>
#include <LocalOrRemoteUserMustChooseStalledIssue.h>
#include <IgnoredStalledIssue.h>
#include <MoveOrRenameCannotOccurIssue.h>

#include <mega/types.h>

bool StalledIssuesCreator::mIsEventRequest = false;

StalledIssuesCreator::StalledIssuesCreator(bool isEventRequest) :
    mMoveOrRenameCannotOccurFactory(std::make_shared<MoveOrRenameCannotOccurFactory>())
{
    mIsEventRequest = isEventRequest;
}

StalledIssuesCreator::StalledIssuesCreator()
{
}

void StalledIssuesCreator::createIssues(mega::MegaSyncStallList* stalls)
{
    clear();

    if (stalls)
    {
        StalledIssuesVariantList solvableIssues;
        auto totalSize(stalls->size());

        for (size_t i = 0; i < totalSize; ++i)
        {
            auto stall = stalls->get(i);
            StalledIssueVariant variant;
            std::shared_ptr<StalledIssue> d;

            if (stall->reason() == mega::MegaSyncStall::SyncStallReason::MoveOrRenameCannotOccur)
            {
                variant = mMoveOrRenameCannotOccurFactory->createIssue(stall);
                if(!variant.isValid())
                {
                    continue;
                }
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

                variant = StalledIssueVariant(d, stall);
            }

            if (variant.shouldBeIgnored())
            {
                continue;
            }

            //Check if it is being solved...
            if (!variant.getData()->isSolved())
            {
                variant.getData()->endFillingIssue();

                if (mIsEventRequest)
                {
                    if (!variant.getData()->isSolvable())
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
                    if (variant.getData()->isSolvable())
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

StalledIssuesVariantList StalledIssuesCreator::issues() const
{
    return mIssues;
}

void StalledIssuesCreator::clear()
{
    mIssues.clear();
    mMoveOrRenameCannotOccurFactory->clear();
}
