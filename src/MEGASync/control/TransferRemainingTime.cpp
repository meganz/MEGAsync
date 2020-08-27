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

void TransferRemainingTime::addTransferSpeedBytesSecond(long long speedBytesSecond, long long remainingBytes)
{
    const auto totalRemainingSeconds{speedBytesSecond ? (remainingBytes / speedBytesSecond) : 0};
    if(mUpdateRemainingTimeCounter == REMAINING_SECONDS_BUFFER_SIZE)
    {
        mUpdateRemainingTimeCounter = 0;
        mRemainingSeconds = calculateMedian(mRemainingTimesBuffer);
    }
    else if(totalRemainingSeconds > 0)
    {
        mRemainingTimesBuffer[mUpdateRemainingTimeCounter] = static_cast<int>(totalRemainingSeconds);
        mUpdateRemainingTimeCounter++;
    }
}


int TransferRemainingTime::getRemainingTimeSeconds() const
{
    return mRemainingSeconds;
}
