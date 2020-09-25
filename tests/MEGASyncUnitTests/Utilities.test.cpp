#include <catch.hpp>
#include "Utilities.h"

TEST_CASE("Create string with sufix from quantities")
{
    REQUIRE(Utilities::getQuantityString(1).toStdString() == "1");
    REQUIRE(Utilities::getQuantityString(10).toStdString() == "10");
    REQUIRE(Utilities::getQuantityString(100).toStdString() == "100");

    REQUIRE(Utilities::getQuantityString(1000).toStdString() == "1K");
    REQUIRE(Utilities::getQuantityString(1009).toStdString() == "1K");
    REQUIRE(Utilities::getQuantityString(1532).toStdString() == "1.53K");
    REQUIRE(Utilities::getQuantityString(10981).toStdString() == "10.9K");
    REQUIRE(Utilities::getQuantityString(10949).toStdString() == "10.9K");
    REQUIRE(Utilities::getQuantityString(15391).toStdString() == "15.3K");
    REQUIRE(Utilities::getQuantityString(198741).toStdString() == "198K");

    REQUIRE(Utilities::getQuantityString(1000*1000).toStdString() == "1M");
    REQUIRE(Utilities::getQuantityString(1532*1000).toStdString() == "1.53M");
    REQUIRE(Utilities::getQuantityString(15391*1000).toStdString() == "15.3M");
    REQUIRE(Utilities::getQuantityString(198741*1000).toStdString() == "198M");

    REQUIRE(Utilities::getQuantityString(1000*1000*1000L).toStdString() == "1G");
    REQUIRE(Utilities::getQuantityString(1532*1000*1000L).toStdString() == "1.53G");
    REQUIRE(Utilities::getQuantityString(15391*1000*1000L).toStdString() == "15.3G");
    REQUIRE(Utilities::getQuantityString(198741*1000*1000L).toStdString() == "198G");
}

TEST_CASE("Create time string")
{
    const auto dayDecorator{std::string{"<span style=\"color:#777777; text-decoration:none;\">d</span>"}};
    const auto hourDecorator{std::string{"<span style=\"color:#777777; text-decoration:none;\">h</span>"}};
    const auto minuteDecorator{std::string{"<span style=\"color:#777777; text-decoration:none;\">m</span>"}};
    const auto secondDecorator{std::string{"<span style=\"color:#777777; text-decoration:none;\">s</span>"}};
    constexpr auto daySeconds{86400};
    constexpr auto hourSeconds{3600};
    constexpr auto minuteSeconds{60};

    auto expected{" 3 " + dayDecorator + " 13 " + hourDecorator};
    REQUIRE(Utilities::getTimeString((daySeconds * 3) + (13 * hourSeconds)).toStdString() == expected);

    expected = " 1 " + hourDecorator + " 05 " + minuteDecorator;
    REQUIRE(Utilities::getTimeString(hourSeconds + (5 * minuteSeconds)).toStdString() == expected);

    expected = " 5 " + minuteDecorator + " 07 " + secondDecorator;
    REQUIRE(Utilities::getTimeString((5*minuteSeconds) + 7).toStdString() == expected);

    expected = "10 " + minuteDecorator + " 07 " + secondDecorator;
    REQUIRE(Utilities::getTimeString((10*minuteSeconds) + 7).toStdString() == expected);

    expected = " 5 " + minuteDecorator;
    constexpr auto secondsPrecision{false};
    REQUIRE(Utilities::getTimeString((5*minuteSeconds) + 7, secondsPrecision).toStdString() == expected);
}
