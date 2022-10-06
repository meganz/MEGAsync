#include <catch.hpp>
#include "TransferRemainingTime.h"

using namespace std::chrono_literals;

TEST_CASE("Calculate transfer remaining time")
{
    TransferRemainingTime transferRemainingTime;
    constexpr auto speedBytesSecond{10};
    constexpr auto remainingBytes{100};

    // return zero transfer remaining time for the first values until the buffer is filled
    constexpr auto bufferSize{9};
    for(int i=0; i < bufferSize-1; i++)
    {
        REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(speedBytesSecond, remainingBytes) == 0s);
    }

    // return median of the values from the buffer when is filled
    REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(speedBytesSecond, remainingBytes) == 10s);

    // return last value calculated until the buffer is filled again
    const std::vector<long long>speeds{20, 100, 20, 1, 100, 20, 2, 2};
    for(const auto speed : speeds)
    {
        REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(speed, remainingBytes) == 10s);
    }

    // return median of the values from the buffer when is filled
    REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(100, remainingBytes) == 5s);
}

TEST_CASE("Calculate transfer remaining time when the buffer is filled with some zero values")
{
    TransferRemainingTime transferRemainingTime;
    constexpr auto speedBytesSecond{101};
    constexpr auto remainingBytes{100};

    // instant remaining time in seconds will be zero when speedBytesSecond is greater than remainingBytes
    constexpr auto bufferSize{9};
    for(int i=0; i < bufferSize-1; i++)
    {
        REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(speedBytesSecond, remainingBytes) == 0s);
    }
}

TEST_CASE("Calculate transfer remaining time when after some speed zero values")
{
    TransferRemainingTime transferRemainingTime;
    constexpr auto speedBytesSecond{0};
    constexpr auto remainingBytes{100};

    // instant remaining time in seconds will be zero when speedBytesSecond is greater than remainingBytes
    constexpr auto bufferSize{9};
    for(int i=0; i < bufferSize-1; i++)
    {
        REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(speedBytesSecond, remainingBytes) == 0s);
    }

    REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(speedBytesSecond, remainingBytes) == std::chrono::seconds::max());
}
