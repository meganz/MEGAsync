#pragma once
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
    TransferRemainingTime(unsigned long long speedBytesSecond, unsigned long long remainingBytes);
    std::chrono::seconds calculateRemainingTimeSeconds(unsigned long long speedBytesSecond, unsigned long long remainingBytes);
    // The median computation code is simpler using an odd buffer size
    static constexpr unsigned int REMAINING_SECONDS_BUFFER_SIZE{9};
    void reset();

private:
    std::chrono::seconds mRemainingSeconds;
    unsigned long mUpdateRemainingTimeCounter;
    std::array<unsigned long long, REMAINING_SECONDS_BUFFER_SIZE> mRemainingTimesBuffer;
    void calculateMedian();
};
