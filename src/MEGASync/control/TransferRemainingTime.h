#pragma once
#include <array>
#include <chrono>

/// Responsability: calculates remaining time in seconds after adding the speed in bytes seconds and the remaining
/// time left. In order to filter the results it uses a buffer that is filled with a few samples (10 samples buffer size)
/// and when the buffer is filled calculates the median of the values on the buffer. First time until the buffer is filled
/// returns zero. After calculating the median and until the buffer is filled again it returns the last calculated value.

class TransferRemainingTime
{
public:
    TransferRemainingTime();
    std::chrono::seconds calculateRemainingTimeSeconds(long long speedBytesSecond, long long remainingBytes);
    static const int REMAINING_SECONDS_BUFFER_SIZE{10};
    void reset();

private:
    int mRemainingSeconds;
    int mUpdateRemainingTimeCounter;
    std::array<int, REMAINING_SECONDS_BUFFER_SIZE> mRemainingTimesBuffer;
};
