#include "IntervalExecutioner.h"

IntervalExecutioner::IntervalExecutioner(int minAmountMsBetweenExecutions)
    : QObject()
    , mExecutionPending(false)
{
    mTimer = std::make_unique<QTimer>();
    mTimer->setSingleShot(true);
    mTimer->setInterval(minAmountMsBetweenExecutions);

    // Connect QTimer's timeout signal to the private slot
    connect(mTimer.get(), &QTimer::timeout, this, &IntervalExecutioner::timerTimeout);

    // Set up QTimer interval, but don't start it yet
    // We'll start it when execution is scheduled
}

void IntervalExecutioner::scheduleExecution()
{
    if (!mTimer->isActive())
    {
        // Timer is not running, so we can execute immediately
        startExecution();
        return;
    }

    // Timer is running; wait at least the specified delay before the next execution
    mExecutionPending = true;
}

void IntervalExecutioner::timerTimeout()
{    
    if (mExecutionPending)
    {
        // There was a request for execution, enough time has passed to actually start execution
        startExecution();
    }
}

void IntervalExecutioner::startExecution()
{
    // Start the timer, so next execution request will have to wait
    mTimer->start();

    // Reset
    mExecutionPending = false;

    // Notify that execution can start
    emit execute();
}
