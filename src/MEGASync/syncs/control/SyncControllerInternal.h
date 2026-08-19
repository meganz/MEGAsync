#ifndef SYNC_CONTROLLER_INTERNAL_H
#define SYNC_CONTROLLER_INTERNAL_H

#include "megaapi.h"
#include "SyncSettings.h"

#include <QSet>

#include <functional>
#include <memory>

// Decomposes SyncController::pauseRunAndResume()'s control flow so it can be unit tested
// without a live MegaApi/SyncController instance. Deliberately NOT included by
// SyncController.h: only SyncController.cpp and its tests should see this - everything
// else that includes SyncController.h gets just the class's public interface.
namespace SyncControllerInternal
{
enum class PauseRunAndResumeOutcome
{
    NullSync,
    Reentrant,
    AlreadyStopped, // duringPause ran directly; no SDK requests were made
    SuspendFailed,
    Completed,
    ResumeFailed,
};

// Runs a setSyncRunState-equivalent request and returns its result the same way
// MegaApiSynchronizedRequest::runRequestWithResult() does (null means API_OK).
using SetSyncRunStateRequest =
    std::function<std::shared_ptr<mega::MegaError>(mega::MegaHandle,
                                                   mega::MegaSync::SyncRunningState)>;

// Whether a sync in this run state is already stopped in a way the user chose
// (SUSPENDED/DISABLED), as opposed to a transitional state on its way toward RUNNING on
// its own (PENDING/LOADING) - only the former is safe to mutate the local folder for
// without forcing a pause/resume cycle first.
bool isAlreadyStoppedState(mega::MegaSync::SyncRunningState runState);

// RAII helper: inserts id into set for its lifetime, so a reentrant call for the same id
// can be detected and rejected. Non-copyable/movable - copying it would duplicate
// cleanup ownership and let one copy's destructor remove id while another is still meant
// to be holding it.
class ScopedBackupIdGuard
{
public:
    ScopedBackupIdGuard(QSet<mega::MegaHandle>& set, mega::MegaHandle id);
    ~ScopedBackupIdGuard();

    Q_DISABLE_COPY_MOVE(ScopedBackupIdGuard)

private:
    QSet<mega::MegaHandle>& mSet;
    mega::MegaHandle mId;
};

PauseRunAndResumeOutcome pauseRunAndResumeCore(const std::shared_ptr<SyncSettings>& syncSetting,
                                               const std::function<void()>& duringPause,
                                               QSet<mega::MegaHandle>& inProgress,
                                               const SetSyncRunStateRequest& request);
} // namespace SyncControllerInternal

#endif // SYNC_CONTROLLER_INTERNAL_H
