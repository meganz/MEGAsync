#ifndef ASYNC_HANDLER
#define ASYNC_HANDLER


#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "ProtectedQueue.h"


template<typename T>
class AsyncHandler
{
public:
    AsyncHandler() :
        mHandleThreadRunning{false}
    {
        startHandleThread();
    }

    ~AsyncHandler()
    {
        stopHandleThread();
    }

    //! This is not a pure virtual function, as it is called in the running thread:
    //! In case the child is already distructed, but the AsyncHandler parent is still alive,
    //! the application will crash with a "pure virtual method called" exception
    virtual void handleTriggerAction(T& t) { (void) t; }


protected:
    void triggerHandlerThread(const T& triggerActionStruct)
    {
        if (!mHandleThreadRunning) {  return; }

        {
            std::unique_lock<std::mutex> lk(mHandleThreadMutex);
            mTriggerActionQueue.push(triggerActionStruct);
        }

        mHandleThreadCondVar.notify_all();
    }

    void stop()
    {
        stopHandleThread();
    }


private:    // functions
    void startHandleThread()
    {
        // Terminate thread first if already running
        stopHandleThread();

        {
            std::unique_lock<std::mutex> lk(mHandleThreadMutex);
            mHandleThreadRunning = true;
        }

        mHandleThread = std::thread(&AsyncHandler::runHandleThread, this);
    }


    void stopHandleThread()
    {
        {
            std::unique_lock<std::mutex> lk(mHandleThreadMutex);

            // Stop triggering the handler thread to do something
            mTriggerActionQueue.clear();

            // Stop the thread if it is running
            if (!mHandleThreadRunning) { return; }

            mHandleThreadRunning = false;
        }

        mHandleThreadCondVar.notify_all();

        if (mHandleThread.joinable())
            mHandleThread.join();
    }


    void runHandleThread()
    {
        while (mHandleThreadRunning)
        {
            {
                std::unique_lock<std::mutex> lk(mHandleThreadMutex);
                mHandleThreadCondVar.wait(lk,
                                              [this]{return ((!mTriggerActionQueue.empty()) ||
                                                               (!mHandleThreadRunning));});
            }

            if (!mHandleThreadRunning) { return; }

            // Handle actions while there are triggers available
            while (!mTriggerActionQueue.empty())
            {
                T triggerActionStruct = {};
                if (mTriggerActionQueue.pop(triggerActionStruct))
                {
                    handleTriggerAction(triggerActionStruct);
                }
            }
        }
    }

private:    // variables
    ProtectedQueue<T> mTriggerActionQueue;
    std::thread mHandleThread;
    std::atomic<bool> mHandleThreadRunning;
    std::condition_variable mHandleThreadCondVar;
    std::mutex mHandleThreadMutex;
};

#endif // ASYNC_HANDLER
