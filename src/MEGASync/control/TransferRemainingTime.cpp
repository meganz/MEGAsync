#include "TransferRemainingTime.h"
#include <algorithm>
#include <functional>
#include <vector>

TransferRemainingTime::TransferRemainingTime()
    :mRemainingSeconds{0}, mUpdateRemainingTimeCounter{0},
        mRemainingTimesBuffer{}
{
}

TransferRemainingTime::TransferRemainingTime(unsigned long long speedBytesSecond, unsigned long long remainingBytes)
    :mRemainingSeconds{0}, mUpdateRemainingTimeCounter{0},
        mRemainingTimesBuffer{}
{
    // Seed the object with given values
    if (speedBytesSecond > 0)
    {
        for (unsigned int i(0); i < REMAINING_SECONDS_BUFFER_SIZE; ++i)
        {
            calculateRemainingTimeSeconds(speedBytesSecond, remainingBytes);
        }
    }
}

void TransferRemainingTime::calculateMedian()
{
    // Avoid calls 2 size() and divisions
    static constexpr unsigned int MEDIAN_IDX{REMAINING_SECONDS_BUFFER_SIZE / 2};

    // This code only works for odd value of REMAINING_SECONDS_BUFFER_SIZE
    std::nth_element(mRemainingTimesBuffer.begin(),
                     mRemainingTimesBuffer.begin() + MEDIAN_IDX,
                     mRemainingTimesBuffer.end());
    mRemainingSeconds = std::chrono::seconds(mRemainingTimesBuffer[MEDIAN_IDX]);
}

std::chrono::seconds TransferRemainingTime::calculateRemainingTimeSeconds(unsigned long long speedBytesSecond,
                                                                          unsigned long long remainingBytes)
{
    // If the speed is positive, compute real remaining time value. Otherwise, set remaining time to
    // infinite remaining time.
    unsigned long long remTime (static_cast<unsigned long long>(std::chrono::seconds::max().count()));
    if (speedBytesSecond > 0)
    {
        remTime = remainingBytes / speedBytesSecond;
    }
    mRemainingTimesBuffer[mUpdateRemainingTimeCounter] = remTime;

    // When the buffer is full, wrap to start and compute median.
    mUpdateRemainingTimeCounter = (mUpdateRemainingTimeCounter + 1) % REMAINING_SECONDS_BUFFER_SIZE;
    if (mUpdateRemainingTimeCounter == 0)
    {
        calculateMedian();
    }

    return mRemainingSeconds;
}

void TransferRemainingTime::reset()
{
    mRemainingSeconds = std::chrono::seconds(0);
    mUpdateRemainingTimeCounter = 0;
    std::fill(std::begin(mRemainingTimesBuffer), std::end(mRemainingTimesBuffer), 0);
}
