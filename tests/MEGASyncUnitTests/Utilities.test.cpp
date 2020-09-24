#include <catch.hpp>
#include "Utilities.h"

TEST_CASE("Create string with sufix from quantities")
{
    CHECK(Utilities::getQuantityString(1).toStdString() == "1");
    CHECK(Utilities::getQuantityString(10).toStdString() == "10");
    CHECK(Utilities::getQuantityString(100).toStdString() == "100");

    CHECK(Utilities::getQuantityString(1000).toStdString() == "1K");
    CHECK(Utilities::getQuantityString(1009).toStdString() == "1K");
    CHECK(Utilities::getQuantityString(1532).toStdString() == "1.53K");
    CHECK(Utilities::getQuantityString(10981).toStdString() == "10.9K");
    CHECK(Utilities::getQuantityString(10949).toStdString() == "10.9K");
    CHECK(Utilities::getQuantityString(15391).toStdString() == "15.3K");
    CHECK(Utilities::getQuantityString(198741).toStdString() == "198K");

    CHECK(Utilities::getQuantityString(1000*1000).toStdString() == "1M");
    CHECK(Utilities::getQuantityString(1532*1000).toStdString() == "1.53M");
    CHECK(Utilities::getQuantityString(15391*1000).toStdString() == "15.3M");
    CHECK(Utilities::getQuantityString(198741*1000).toStdString() == "198M");

    CHECK(Utilities::getQuantityString(1000*1000*1000LL).toStdString() == "1G");
    CHECK(Utilities::getQuantityString(1532*1000*1000LL).toStdString() == "1.53G");
    CHECK(Utilities::getQuantityString(15391*1000*1000LL).toStdString() == "15.3G");
    CHECK(Utilities::getQuantityString(198741*1000*1000LL).toStdString() == "198G");
}
