#include "ThreadedQueueShellNotifier.h"
#include "megaapi.h"

ThreadedQueueShellNotifier::ThreadedQueueShellNotifier(std::shared_ptr<AbstractShellNotifier> baseNotifier)
    : ShellNotifierDecorator(baseNotifier)
{
    connect(mBaseNotifier.get(), &AbstractShellNotifier::shellNotificationProcessed,
            this, &AbstractShellNotifier::shellNotificationProcessed);
}

ThreadedQueueShellNotifier::~ThreadedQueueShellNotifier()
{
    if (!mThread.joinable()) // thread wasn't started
    {
        return;
    }

    // signal the thread to stop
    {
        std::unique_lock<std::mutex> lock(mQueueAccessMutex);
        mExit = true;
        mWaitCondition.notify_all();
    }

    mThread.join();
}

void ThreadedQueueShellNotifier::checkReportQueueSize()
{
    // mutex already locked

    auto now = mPendingNotifications.size();
    auto last = lastReportedQueueSize;

    // report if climbs above 1000, or gets back to 0
    if ((now > 1000 && last > 1000) ||
        (now == 0 && last > 1000) ||
        (now > 1000 && last == 0))
    {
        if (now * 10 > last * 12 ||
            last * 10 > now * 12)
        {
            // increased or decreased by factor 1.2 since last report
            lastReportedQueueSize = now;
            ::mega::MegaApi::log(::mega::MegaApi::LOG_LEVEL_INFO, ("Queue to notify shell size is now:" + std::to_string(now)).c_str());
        }
    }
}

void ThreadedQueueShellNotifier::notify(const QString &localPath)
{
    // make sure the thread was started
    if (!mThread.joinable())
    {
        mThread = std::thread([this]() { doInThread(); });
    }

    std::unique_lock<std::mutex> lock(mQueueAccessMutex);

    mPendingNotifications.emplace(localPath);
    checkReportQueueSize();
    mWaitCondition.notify_one();
}

void ThreadedQueueShellNotifier::doInThread()
{
    for (;;)
    {
        QString path;

        { // lock scope
            std::unique_lock<std::mutex> lock(mQueueAccessMutex);

            if (mPendingNotifications.empty())
            {
                // end execution only when the notification queue was emptied
                if (mExit)
                {
                    return;
                }

                mWaitCondition.wait(lock);
            }
            else
            {
                // pop next pending notification
                path.swap(mPendingNotifications.front());
                mPendingNotifications.pop();
            }
        } // end of lock scope

        if (mExit)
        {
            return;  // There could be many of these already queued.  Don't delay app exit (possibly for minutes).  These are irrelevant if MEGAsync is no longer running
        }
        else if (!path.isEmpty())
        {
            mBaseNotifier->notify(path);
            checkReportQueueSize();
        }
    }
}

