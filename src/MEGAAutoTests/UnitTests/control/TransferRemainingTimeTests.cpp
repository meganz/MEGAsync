#include "TransferRemainingTime.h"
#include <catch.hpp>

using namespace std::chrono_literals;

class TransferRemainingTimeTests
{
protected:
    void testCalculateWhileBufferNotFilled(TransferRemainingTime& transferObj,
                                           int speedBytesSecond,
                                           int remainingBytes)
    {
        constexpr auto bufferSize = 9;

        // We test for bufferSize - 1 because once the buffer is filled, the returned value is the
        // median value. This test is specifically testing for buffer NOT filled, so we leave the
        // last value not filled.
        for (int i = 0; i < bufferSize - 1; i++)
        {
            REQUIRE(transferObj.calculateRemainingTimeSeconds(speedBytesSecond, remainingBytes) ==
                    0s);
        }
    }
};

TEST_CASE_METHOD(TransferRemainingTimeTests, "calculateRemainingTimeSeconds()")
{
    TransferRemainingTime transferRemainingTime;

    SECTION("Common use case")
    {
        constexpr auto speedBytesSecond{10};
        constexpr auto remainingBytes{100};

        // return zero transfer remaining time for the first values until the buffer is filled
        testCalculateWhileBufferNotFilled(transferRemainingTime, speedBytesSecond, remainingBytes);

        // return median of the values from the buffer when is filled
        REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(speedBytesSecond,
                                                                    remainingBytes) == 10s);

        // return last value calculated until the buffer is filled again
        const std::vector<long long> speeds{20, 100, 20, 1, 100, 20, 2, 2};
        for (const auto speed: speeds)
        {
            REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(speed, remainingBytes) ==
                    10s);
        }

        // return median of the values from the buffer when is filled
        REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(100, remainingBytes) == 5s);
    }

    SECTION("Buffer is filled with some zero values")
    {
        constexpr auto speedBytesSecond{101};
        constexpr auto remainingBytes{100};

        // instant remaining time in seconds will be zero when speedBytesSecond is greater than
        // remainingBytes
        testCalculateWhileBufferNotFilled(transferRemainingTime, speedBytesSecond, remainingBytes);
    }

    SECTION("Speed is zero")
    {
        constexpr auto speedBytesSecond{0};
        constexpr auto remainingBytes{100};

        // instant remaining time in seconds will be zero when speedBytesSecond is greater than
        // remainingBytes
        testCalculateWhileBufferNotFilled(transferRemainingTime, speedBytesSecond, remainingBytes);

        REQUIRE(
            transferRemainingTime.calculateRemainingTimeSeconds(speedBytesSecond, remainingBytes) ==
            std::chrono::seconds::max());
    }
}
