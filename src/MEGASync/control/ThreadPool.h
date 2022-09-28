#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <QtGlobal>

class ThreadPool
{
public:

    explicit ThreadPool(std::size_t threadCount);
    ~ThreadPool();

    Q_DISABLE_COPY(ThreadPool)

    void push(std::function<void()> functor);
    static bool isThreadInterrupted();

private:
    void worker(std::size_t index);

    void shutdown();

    std::atomic<bool> mDone = false;
    static thread_local std::atomic<bool>* mLocalToThreadDone;

    std::vector<std::thread> mThreads;
    std::queue<std::function<void()>> mFunctors;
    std::condition_variable mCv;
    std::mutex mMutex;
};

