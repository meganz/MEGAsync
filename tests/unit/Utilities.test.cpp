#include <catch.hpp>
#include "Utilities.h"

TEST_CASE("Create string with sufix from quantities")
{
    REQUIRE(Utilities::getQuantityString(1).toStdString() == "1");
    REQUIRE(Utilities::getQuantityString(10).toStdString() == "10");
    REQUIRE(Utilities::getQuantityString(100).toStdString() == "100");

    REQUIRE(Utilities::getQuantityString(1000).toStdString() == "1K");
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
