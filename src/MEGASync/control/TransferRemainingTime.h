#ifndef TRANSFER_REMAINING_TIME_H
#define TRANSFER_REMAINING_TIME_H

#include <array>
#include <chrono>

/// Responsability: calculates remaining time in seconds after adding the speed in bytes seconds and the remaining
/// bytes left. In order to filter the results it uses a buffer that is filled with a few samples (9 samples buffer size)
/// and when the buffer is filled calculates the median of the values on the buffer. First time until the buffer is filled
/// returns zero. After calculating the median and until the buffer is filled again it returns the last calculated value.
class TransferRemainingTime
{
public:
    TransferRemainingTime();
    TransferRemainingTime(long long speedBytesSecond,long long remainingBytes);
    std::chrono::seconds calculateRemainingTimeSeconds(long long speedBytesSecond, long long remainingBytes);
    // The median computation code is simpler using an odd buffer size
    static constexpr unsigned int REMAINING_SECONDS_BUFFER_SIZE{9};
    void reset();

private:
    std::chrono::seconds mRemainingSeconds;
    std::array<long long, REMAINING_SECONDS_BUFFER_SIZE>::size_type mUpdateRemainingTimeCounter;
    std::array<long long, REMAINING_SECONDS_BUFFER_SIZE> mRemainingTimesBuffer;
    void calculateMedian();
};
#endif
