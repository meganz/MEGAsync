#include "Utilities.h"
#include <catch.hpp>

class UtilitiesTests
{
protected:
    void testGetSizeString(unsigned long long value, const std::string& expected)
    {
        CHECK(Utilities::getSizeString(value).toStdString() == expected);
    }
};

TEST_CASE_METHOD(UtilitiesTests, "getQuantityString()")
{
    testGetSizeString(1, "1 Bytes");
    testGetSizeString(10, "10 Bytes");
    testGetSizeString(100, "100 Bytes");
    testGetSizeString(1023, "1023 Bytes");

    testGetSizeString(1024, "1 KB");
    testGetSizeString(1025, "1 KB");
    testGetSizeString(1532, "1.5 KB");
    testGetSizeString(10981, "10.72 KB");
    testGetSizeString(10949, "10.69 KB");
    testGetSizeString(15391, "15.03 KB");
    testGetSizeString(198741, "194.08 KB");

    testGetSizeString(1024 * 1024, "1 MB");
    testGetSizeString(1532 * 1024, "1.5 MB");
    testGetSizeString(15391 * 1024, "15.03 MB");
    testGetSizeString(198741 * 1024, "194.08 MB");

    testGetSizeString(1024 * 1024 * 1024LL, "1 GB");
    testGetSizeString(1532 * 1024 * 1024LL, "1.5 GB");
    testGetSizeString(15391 * 1024 * 1024LL, "15.03 GB");
    testGetSizeString(198741 * 1024 * 1024LL, "194.08 GB");

    testGetSizeString(1024 * 1024 * 1024LL * 1024LL, "1 TB");
    testGetSizeString(1532 * 1024 * 1024LL * 1024LL, "1.5 TB");
    testGetSizeString(15391 * 1024 * 1024LL * 1024LL, "15.03 TB");
    testGetSizeString(198741 * 1024 * 1024LL * 1024LL, "194.08 TB");
}

TEST_CASE_METHOD(UtilitiesTests, "getTimeString()")
{
    // "Changes on time format" (commit 4452c4481) dropped the HTML styling that
    // used to wrap the unit letters in a <span>; getTimeString now returns plain
    // text. The leading unit is space-padded to two chars, the rest zero-padded.
    constexpr auto daySeconds{86400};
    constexpr auto hourSeconds{3600};
    constexpr auto minuteSeconds{60};

    REQUIRE(Utilities::getTimeString((daySeconds * 3) + (13 * hourSeconds)).toStdString() ==
            " 3d 13h");

    REQUIRE(Utilities::getTimeString(hourSeconds + (5 * minuteSeconds)).toStdString() == " 1h 05m");

    REQUIRE(Utilities::getTimeString((5 * minuteSeconds) + 7).toStdString() == " 5m 07s");

    REQUIRE(Utilities::getTimeString((10 * minuteSeconds) + 7).toStdString() == "10m 07s");

    constexpr auto secondsPrecision{false};
    REQUIRE(Utilities::getTimeString((5 * minuteSeconds) + 7, secondsPrecision).toStdString() ==
            " 5m");
}
