#include "SyncControllerInternal.h"
#include "SyncSettings.h"
#include <catch.hpp>

#include <QSet>

#include <type_traits>
#include <vector>

using namespace SyncControllerInternal;

static_assert(!std::is_copy_constructible<ScopedBackupIdGuard>::value,
              "ScopedBackupIdGuard must not be copyable");
static_assert(!std::is_move_constructible<ScopedBackupIdGuard>::value,
              "ScopedBackupIdGuard must not be movable");

namespace
{

// A MegaSync stand-in with a controllable run state and backupId, so pauseRunAndResumeCore()
// can be exercised without a live MegaApi. copy() must be overridden too: SyncSettings::setSync()
// clones whatever MegaSync it's given, and virtual dispatch through that clone only reaches
// getRunState()/getBackupId() below if the clone is actually a FakeMegaSync.
class FakeMegaSync: public mega::MegaSync
{
public:
    FakeMegaSync(mega::MegaSync::SyncRunningState runState, mega::MegaHandle backupId):
        mRunState(runState),
        mBackupId(backupId)
    {}

    int getRunState() const override
    {
        return mRunState;
    }

    mega::MegaHandle getBackupId() const override
    {
        return mBackupId;
    }

    mega::MegaSync* copy() override
    {
        return new FakeMegaSync(mRunState, mBackupId);
    }

private:
    mega::MegaSync::SyncRunningState mRunState;
    mega::MegaHandle mBackupId;
};

std::shared_ptr<SyncSettings> makeSyncSettings(mega::MegaSync::SyncRunningState runState,
                                               mega::MegaHandle backupId = 1)
{
    FakeMegaSync fake(runState, backupId);
    return std::make_shared<SyncSettings>(&fake);
}

// MegaError's int constructor is protected; a derived type is needed to reach it since
// make_shared<MegaError> constructs the base type directly, not through a subclass.
class FakeMegaError: public mega::MegaError
{
public:
    explicit FakeMegaError(int errorCode):
        mega::MegaError(errorCode)
    {}
};

std::shared_ptr<mega::MegaError> makeError(int errorCode)
{
    return std::make_shared<FakeMegaError>(errorCode);
}

} // namespace

TEST_CASE("isAlreadyStoppedState() classifies run states correctly")
{
    CHECK_FALSE(isAlreadyStoppedState(mega::MegaSync::RUNSTATE_PENDING));
    CHECK_FALSE(isAlreadyStoppedState(mega::MegaSync::RUNSTATE_LOADING));
    CHECK_FALSE(isAlreadyStoppedState(mega::MegaSync::RUNSTATE_RUNNING));
    CHECK(isAlreadyStoppedState(mega::MegaSync::RUNSTATE_SUSPENDED));
    CHECK(isAlreadyStoppedState(mega::MegaSync::RUNSTATE_DISABLED));
}

TEST_CASE("ScopedBackupIdGuard inserts on construction and removes on destruction")
{
    QSet<mega::MegaHandle> inProgress;
    const mega::MegaHandle id = 42;

    REQUIRE_FALSE(inProgress.contains(id));
    {
        ScopedBackupIdGuard guard(inProgress, id);
        CHECK(inProgress.contains(id));
    }
    CHECK_FALSE(inProgress.contains(id));
}

TEST_CASE("ScopedBackupIdGuard tracks independent ids separately")
{
    QSet<mega::MegaHandle> inProgress;
    const mega::MegaHandle firstId = 1;
    const mega::MegaHandle secondId = 2;

    ScopedBackupIdGuard firstGuard(inProgress, firstId);
    {
        ScopedBackupIdGuard secondGuard(inProgress, secondId);
        CHECK(inProgress.contains(firstId));
        CHECK(inProgress.contains(secondId));
    }
    CHECK(inProgress.contains(firstId));
    CHECK_FALSE(inProgress.contains(secondId));
}

TEST_CASE("pauseRunAndResumeCore() rejects a null sync without making any request")
{
    QSet<mega::MegaHandle> inProgress;
    bool requestCalled = false;
    SetSyncRunStateRequest request = [&](mega::MegaHandle, mega::MegaSync::SyncRunningState)
    {
        requestCalled = true;
        return nullptr;
    };

    const auto outcome = pauseRunAndResumeCore(nullptr, nullptr, inProgress, request);

    CHECK(outcome == PauseRunAndResumeOutcome::NullSync);
    CHECK_FALSE(requestCalled);
}

TEST_CASE("pauseRunAndResumeCore() runs duringPause directly for an already-stopped sync")
{
    QSet<mega::MegaHandle> inProgress;
    auto syncSetting = makeSyncSettings(mega::MegaSync::RUNSTATE_SUSPENDED);
    bool duringPauseRan = false;
    bool requestCalled = false;
    SetSyncRunStateRequest request = [&](mega::MegaHandle, mega::MegaSync::SyncRunningState)
    {
        requestCalled = true;
        return nullptr;
    };

    const auto outcome = pauseRunAndResumeCore(
        syncSetting,
        [&]()
        {
            duringPauseRan = true;
        },
        inProgress,
        request);

    CHECK(outcome == PauseRunAndResumeOutcome::AlreadyStopped);
    CHECK(duringPauseRan);
    CHECK_FALSE(requestCalled);
    CHECK_FALSE(inProgress.contains(syncSetting->backupId()));
}

TEST_CASE("pauseRunAndResumeCore() suspends, runs duringPause, then resumes a running sync")
{
    QSet<mega::MegaHandle> inProgress;
    auto syncSetting = makeSyncSettings(mega::MegaSync::RUNSTATE_RUNNING);
    std::vector<mega::MegaSync::SyncRunningState> requestedStates;
    bool duringPauseRan = false;

    SetSyncRunStateRequest request = [&](mega::MegaHandle, mega::MegaSync::SyncRunningState state)
    {
        requestedStates.push_back(state);
        if (state == mega::MegaSync::RUNSTATE_SUSPENDED)
        {
            CHECK_FALSE(duringPauseRan);
        }
        else if (state == mega::MegaSync::RUNSTATE_RUNNING)
        {
            CHECK(duringPauseRan);
        }
        return nullptr;
    };

    const auto outcome = pauseRunAndResumeCore(
        syncSetting,
        [&]()
        {
            duringPauseRan = true;
        },
        inProgress,
        request);

    CHECK(outcome == PauseRunAndResumeOutcome::Completed);
    CHECK(duringPauseRan);
    REQUIRE(requestedStates.size() == 2);
    CHECK(requestedStates[0] == mega::MegaSync::RUNSTATE_SUSPENDED);
    CHECK(requestedStates[1] == mega::MegaSync::RUNSTATE_RUNNING);
    CHECK_FALSE(inProgress.contains(syncSetting->backupId()));
}

TEST_CASE("pauseRunAndResumeCore() reports SuspendFailed without running duringPause")
{
    QSet<mega::MegaHandle> inProgress;
    auto syncSetting = makeSyncSettings(mega::MegaSync::RUNSTATE_RUNNING);
    bool duringPauseRan = false;

    SetSyncRunStateRequest request = [](mega::MegaHandle, mega::MegaSync::SyncRunningState state)
    {
        return state == mega::MegaSync::RUNSTATE_SUSPENDED ?
                   makeError(mega::MegaError::API_EFAILED) :
                   nullptr;
    };

    const auto outcome = pauseRunAndResumeCore(
        syncSetting,
        [&]()
        {
            duringPauseRan = true;
        },
        inProgress,
        request);

    CHECK(outcome == PauseRunAndResumeOutcome::SuspendFailed);
    CHECK_FALSE(duringPauseRan);
    CHECK_FALSE(inProgress.contains(syncSetting->backupId()));
}

TEST_CASE("pauseRunAndResumeCore() runs duringPause but reports ResumeFailed when resuming fails")
{
    QSet<mega::MegaHandle> inProgress;
    auto syncSetting = makeSyncSettings(mega::MegaSync::RUNSTATE_RUNNING);
    bool duringPauseRan = false;

    SetSyncRunStateRequest request = [](mega::MegaHandle, mega::MegaSync::SyncRunningState state)
    {
        return state == mega::MegaSync::RUNSTATE_RUNNING ? makeError(mega::MegaError::API_EFAILED) :
                                                           nullptr;
    };

    const auto outcome = pauseRunAndResumeCore(
        syncSetting,
        [&]()
        {
            duringPauseRan = true;
        },
        inProgress,
        request);

    CHECK(outcome == PauseRunAndResumeOutcome::ResumeFailed);
    CHECK(duringPauseRan);
    CHECK_FALSE(inProgress.contains(syncSetting->backupId()));
}

TEST_CASE("pauseRunAndResumeCore() rejects a reentrant call for the same backupId")
{
    QSet<mega::MegaHandle> inProgress;
    auto syncSetting = makeSyncSettings(mega::MegaSync::RUNSTATE_RUNNING, 7);
    inProgress.insert(7); // a previous call for this sync is still in flight
    bool requestCalled = false;

    SetSyncRunStateRequest request = [&](mega::MegaHandle, mega::MegaSync::SyncRunningState)
    {
        requestCalled = true;
        return nullptr;
    };

    const auto outcome = pauseRunAndResumeCore(syncSetting, nullptr, inProgress, request);

    CHECK(outcome == PauseRunAndResumeOutcome::Reentrant);
    CHECK_FALSE(requestCalled);
    // The guard belongs to the still-in-flight call; the rejected call must not touch it.
    CHECK(inProgress.contains(7));
}

TEST_CASE("pauseRunAndResumeCore() treats different backupIds as independent")
{
    QSet<mega::MegaHandle> inProgress;
    inProgress.insert(99); // some other sync's call is in flight
    auto syncSetting = makeSyncSettings(mega::MegaSync::RUNSTATE_RUNNING, 1);
    bool duringPauseRan = false;

    SetSyncRunStateRequest request = [](mega::MegaHandle, mega::MegaSync::SyncRunningState)
    {
        return nullptr;
    };

    const auto outcome = pauseRunAndResumeCore(
        syncSetting,
        [&]()
        {
            duringPauseRan = true;
        },
        inProgress,
        request);

    CHECK(outcome == PauseRunAndResumeOutcome::Completed);
    CHECK(duringPauseRan);
    CHECK(inProgress.contains(99)); // untouched
    CHECK_FALSE(inProgress.contains(1));
}
