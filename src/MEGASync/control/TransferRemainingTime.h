#pragma once
#include <array>

class TransferRemainingTime
{
public:
    TransferRemainingTime();
    int calculateRemainingTimeSeconds(long long speedBytesSecond, long long remainingBytes);
    static const int REMAINING_SECONDS_BUFFER_SIZE{10};
    void reset();

private:
    int mRemainingSeconds;
    int mUpdateRemainingTimeCounter;
    std::array<int, REMAINING_SECONDS_BUFFER_SIZE> mRemainingTimesBuffer;
};
