#include "ThreadPool.h"

#include <array>
#include <chrono>
#include <string>

#include <QtGlobal>

#ifdef Q_OS_LINUX
#include <pthread.h>
#endif

thread_local std::atomic<bool>* ThreadPool::mLocalToThreadDone = nullptr;

ThreadPool::ThreadPool(const std::size_t threadCount)
{
    Q_ASSERT(threadCount > 0);
    for (std::size_t i = 0; i < threadCount; ++i)
    {
        std::thread thread;
        try
        {
            thread = std::thread(&ThreadPool::worker, this, i);
        }
        catch (...)
        {
            shutdown();
            throw;
        }
        try
        {
            mThreads.push_back(std::move(thread));
        }
        catch (...)
        {
            shutdown();
            thread.join();
            throw;
        }
    }
}

ThreadPool::~ThreadPool()
{
    shutdown();
}

void ThreadPool::push(std::function<void()> functor)
{
    {
        std::lock_guard<std::mutex> lock{mMutex};
        mFunctors.push(std::move(functor));
    }
    mCv.notify_one();
}

bool ThreadPool::isThreadInterrupted()
{
    if(mLocalToThreadDone && (*mLocalToThreadDone))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ThreadPool::worker(const std::size_t index)
{
    const auto threadName = "TPw" + std::to_string(index);
#ifdef Q_OS_LINUX
    if (pthread_setname_np(pthread_self(), threadName.c_str()))
    {
        qWarning("Unable to set thread name");
        Q_ASSERT(false);
    }
#endif
    mLocalToThreadDone = &mDone;
    for (;;)
    {
        std::function<void()> functor;
        {
            std::unique_lock<std::mutex> lock{mMutex};
            mCv.wait(lock, [this]
            {
                return mDone || !mFunctors.empty();
            });
            if (mDone && mFunctors.empty())
            {
                break;
            }
            functor = std::move(mFunctors.front());
            mFunctors.pop();
        }
        try
        {
            functor();
        }
        catch (const std::exception& e)
        {
            qCritical("ThreadPool: Error: %s", e.what());
            Q_ASSERT(false);
        }
    }
}

void ThreadPool::shutdown()
{
    {
        std::lock_guard<std::mutex> lock{mMutex};
        mDone = true;
    }
    mCv.notify_all();
    for (std::thread& thread : mThreads)
    {
        thread.join();
    }
    mThreads.clear();
}

