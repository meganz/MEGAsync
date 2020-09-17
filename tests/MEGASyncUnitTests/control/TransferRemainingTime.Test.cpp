#include <catch.hpp>
#include "TransferRemainingTime.h"

TEST_CASE("Calculate transfer remaining time")
{
    TransferRemainingTime transferRemainingTime;
    auto speedBytesSecond{10};
    auto remainingBytes{100};

    for(int i=0; i<9; i++)
    {
        REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(speedBytesSecond, remainingBytes) == 0);
    }
    REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(speedBytesSecond, remainingBytes) == 10);

    const std::vector<long long>speeds{20, 100, 20, 1, 100, 20, 2, 2, 100};
    for(const auto speed : speeds)
    {
        REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(speed, remainingBytes) == 10);
    }
    REQUIRE(transferRemainingTime.calculateRemainingTimeSeconds(speedBytesSecond, remainingBytes) == 5);
}
