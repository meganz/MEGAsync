#pragma once
#include <array>

class TransferRemainingTime
{
public:
    TransferRemainingTime();
    void addTransferSpeedBytesSecond(long long speedBytesSecond, long long remainingBytes);
    int getRemainingTimeSeconds() const;
    static const int REMAINING_SECONDS_BUFFER_SIZE{10};

private:
    int mRemainingSeconds;
    int mUpdateRemainingTimeCounter;
    std::array<int, REMAINING_SECONDS_BUFFER_SIZE> mRemainingTimesBuffer;
};
