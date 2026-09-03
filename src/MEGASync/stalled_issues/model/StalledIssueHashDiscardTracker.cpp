#include "StalledIssueHashDiscardTracker.h"

namespace
{
using Clock = std::chrono::steady_clock;
}

void StalledIssueHashDiscardTracker::prepareForNewRequest(const StalledIssuesVariantList& issues)
{
    QMutexLocker lock(&mMutex);

    for (const auto& issueVariant: issues)
    {
        const auto issue = issueVariant.consultData();
        if (!issue)
        {
            continue;
        }

        const auto& originalStall = issue->getOriginalStall();
        if (!originalStall)
        {
            continue;
        }

        const auto entryIt = mEntries.find(originalStall->getHash());
        if (entryIt != mEntries.end())
        {
            entryIt->second.recentlyReceived = false;
        }
    }
}

void StalledIssueHashDiscardTracker::keepTrackIssues(StalledIssuesVariantList& issues)
{
    const auto now = Clock::now();
    QMutexLocker lock(&mMutex);

    for (auto it = issues.begin(); it != issues.end();)
    {
        const auto issue = it->consultData();
        if (!issue)
        {
            it = issues.erase(it);
            continue;
        }

        const auto& originalStall = issue->getOriginalStall();
        if (!originalStall)
        {
            it = issues.erase(it);
            continue;
        }

        const auto rule = issue->hashDiscardRuleForState(issue->getIsSolved());

        // Issues without a discard rule are not managed by the tracker; they must
        // remain in the failed list (e.g. LocalOrRemote failures), so keep them
        // instead of treating them as removed.
        if (!rule.has_value())
        {
            ++it;
            continue;
        }

        const auto entryIt = mEntries.find(originalStall->getHash());
        const bool hasEntry = entryIt != mEntries.end();
        const bool recentlyReceived = hasEntry && entryIt->second.recentlyReceived;
        const bool hasExpire = hasEntry && entryIt->second.expireAt.has_value();
        const bool expired = hasExpire && entryIt->second.expireAt.value() <= now;

        // Windowed entries (failed/solved with a discard duration) are kept until the
        // window expires, regardless of whether the SDK reported them this cycle.
        // Entries without a window are kept only while the SDK still reports them.
        const auto shouldKeep = hasEntry && (hasExpire ? !expired : recentlyReceived);

        if (shouldKeep)
        {
            ++it;
        }
        else
        {
            it = issues.erase(it);
        }
    }
}

void StalledIssueHashDiscardTracker::track(const StalledIssue* issue,
                                           StalledIssue::ResolutionState state)
{
    if (!issue)
    {
        return;
    }

    const auto& originalStall = issue->getOriginalStall();
    if (!originalStall)
    {
        return;
    }

    auto hash = originalStall->getHash();

    const auto rule = issue->hashDiscardRuleForState(state);
    if (!rule.has_value())
    {
        return;
    }

    bool recentlyReceived(true);
    QMutexLocker lock(&mMutex);

    if (!rule->discardDuration.has_value())
    {
        mEntries[hash] = {rule.value(), std::nullopt, recentlyReceived};
    }
    else
    {
        const auto it = mEntries.find(hash);

        if (it == mEntries.end())
        {
            auto expireAt = Clock::now() + rule->discardDuration.value();
            mEntries[hash] = {rule.value(), expireAt, recentlyReceived};
        }
        else
        {
            mEntries[hash] = {rule.value(), it->second.expireAt, recentlyReceived};
        }
    }
}

void StalledIssueHashDiscardTracker::markAsRecentlyReceived(const StalledIssue* issue)
{
    if (!issue)
    {
        return;
    }

    const auto& originalStall = issue->getOriginalStall();
    if (!originalStall)
    {
        return;
    }

    auto hash = originalStall->getHash();

    QMutexLocker lock(&mMutex);

    const auto it = mEntries.find(hash);
    if (it != mEntries.end())
    {
        it->second.recentlyReceived = true;
    }
}

void StalledIssueHashDiscardTracker::purgeExpired()
{
    const auto now = Clock::now();

    QMutexLocker lock(&mMutex);

    for (auto it = mEntries.begin(); it != mEntries.end();)
    {
        // Windowed entries live until they expire; entries without a window are
        // dropped once the SDK stops reporting them.
        const bool shouldPurge = it->second.expireAt.has_value() ? *it->second.expireAt <= now :
                                                                   !it->second.recentlyReceived;

        if (shouldPurge)
        {
            it = mEntries.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool StalledIssueHashDiscardTracker::shouldDiscard(size_t hash) const
{
    bool discard(false);

    QMutexLocker lock(&mMutex);

    auto it = mEntries.find(hash);
    if (it != mEntries.end() &&
        (!it->second.expireAt.has_value() || (it->second.expireAt.value() > Clock::now())))
    {
        discard = true;
    }

    return discard;
}
