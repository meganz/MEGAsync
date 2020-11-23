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

std::chrono::seconds TransferRemainingTime::calculateRemainingTimeSeconds(long long speedBytesSecond, long long remainingBytes)
{
    if(speedBytesSecond)
    {
        mRemainingTimesBuffer[mUpdateRemainingTimeCounter] = static_cast<int>(remainingBytes / speedBytesSecond);
    }
    else
    {
        mRemainingTimesBuffer[mUpdateRemainingTimeCounter] = std::numeric_limits<int>::max();
    }
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
    if(mRemainingSeconds == std::numeric_limits<int>::max())
    {
        return std::chrono::seconds::max();
    }
    else
    {
        return std::chrono::seconds{mRemainingSeconds};
    }
}

void TransferRemainingTime::reset()
{
    mRemainingSeconds = 0;
    mUpdateRemainingTimeCounter = 0;
    std::fill(std::begin(mRemainingTimesBuffer), std::end(mRemainingTimesBuffer), 0);
}
