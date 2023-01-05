#ifndef THREADEDQUEUESHELLNOTIFIER_H
#define THREADEDQUEUESHELLNOTIFIER_H

#include "platform/ShellNotifier.h"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

/**
 * @brief Implements a queue where notifications are added and
 * a separate thread is consuming the queue and calling baseNotifier
 * to do the notification.
 */
class ThreadedQueueShellNotifier : public ShellNotifierDecorator
{
public:
    ThreadedQueueShellNotifier(std::shared_ptr<AbstractShellNotifier> baseNotifier);
    virtual ~ThreadedQueueShellNotifier();

    void notify(const QString& path) override;

private:
    void doInThread();

    std::thread mThread;
    std::queue<QString> mPendingNotifications;
    std::mutex mQueueAccessMutex;
    std::condition_variable mWaitCondition;
    bool mExit = false;
};

#endif // THREADEDQUEUESHELLNOTIFIER_H
