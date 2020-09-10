#include "TransferRemainingTime.h"
#include <algorithm>
#include <functional>
#include <vector>

TransferRemainingTime::TransferRemainingTime()
    :mRemainingSeconds{0}, mUpdateRemainingTimeCounter{0}
{
}

int calculateMedian(std::array<int, TransferRemainingTime::REMAINING_SECONDS_BUFFER_SIZE>& values)
{
    std::nth_element(values.begin(), values.begin() + values.size()/2, values.end());
    return values[values.size()/2];
}

int TransferRemainingTime::calculateRemainingTimeSeconds(long long speedBytesSecond, long long remainingBytes)
{
    const auto totalRemainingSeconds{speedBytesSecond ? (remainingBytes / speedBytesSecond) : 0};
    if(totalRemainingSeconds > 0)
    {
        mRemainingTimesBuffer[mUpdateRemainingTimeCounter] = static_cast<int>(totalRemainingSeconds);
        const auto bufferIsFull{mUpdateRemainingTimeCounter == REMAINING_SECONDS_BUFFER_SIZE - 1};
        if(bufferIsFull)
        {
            mUpdateRemainingTimeCounter = 0;
            mRemainingSeconds = calculateMedian(mRemainingTimesBuffer);
        }
        else
        {
            mUpdateRemainingTimeCounter++;
        }
    }
    return mRemainingSeconds;
}

void TransferRemainingTime::reset()
{
    mRemainingSeconds = 0;
    mUpdateRemainingTimeCounter = 0;
    std::fill(std::begin(mRemainingTimesBuffer), std::end(mRemainingTimesBuffer), 0);
}
