#ifndef PROTECTED_QUEUE
#define PROTECTED_QUEUE

#include <queue>
#include <thread>
#include <mutex>


template <typename T>
class ProtectedQueue
{
public:
    ProtectedQueue(){}
    virtual ~ProtectedQueue(){}

    ProtectedQueue(const ProtectedQueue& other)
    {
        std::lock_guard<std::mutex> guard( other.mMutex );
        mQueue = other.mQueue;
    }

    ProtectedQueue& operator= (ProtectedQueue& other)
    {
        if (&other == this)
        {
            return *this;
        }

        std::unique_lock<std::mutex> lock1(mMutex, std::defer_lock);
        std::unique_lock<std::mutex> lock2(other.mMutex, std::defer_lock);
        std::lock(lock1, lock2);
        mQueue = other.mQueue;

        return *this;
    }

    bool pop(T& item)
    {
        std::lock_guard<std::mutex> guard(mMutex);
        if (!mQueue.empty())
        {
            item = mQueue.front();
            mQueue.pop();
            return true;
        }

        return false;
    }

    bool pop()
    {
        std::lock_guard<std::mutex> guard(mMutex);
        if (!mQueue.empty())
        {
            mQueue.pop();
            return true;
        }

        return false;
    }

    void push(const T& item)
    {
        std::lock_guard<std::mutex> guard(mMutex);
        mQueue.push(std::move(item));
    }

    void push(T&& item)
    {
        std::lock_guard<std::mutex> guard(mMutex);
        mQueue.push(std::move(item));
    }

    void push_to_front(const T& element)
    {
        std::lock_guard<std::mutex> guard(mMutex);
        std::queue<T> temp_queue;
        temp_queue.push(element);

        while (!mQueue.empty())
        {
            temp_queue.push(mQueue.front());
            mQueue.pop();
        }

        mQueue = std::move(temp_queue);
    }

    bool empty()
    {
        std::lock_guard<std::mutex> guard(mMutex);
        return mQueue.empty();
    }

    int size()
    {
        std::lock_guard<std::mutex> guard(mMutex);
        return static_cast<int>(mQueue.size());
    }

    void clear()
    {
        std::lock_guard<std::mutex> guard(mMutex);
        std::queue<T> empty;
        std::swap(mQueue, empty);
    }

private:
    std::queue<T> mQueue;
    mutable std::mutex mMutex;
};

#endif // PROTECTED_QUEUE
