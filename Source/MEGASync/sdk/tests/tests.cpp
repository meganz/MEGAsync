/**
 * @file tests/tests.cpp
 * @brief Mega SDK main test file
 *
 * (c) 2013 by Mega Limited, Wellsford, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#include "mega.h"
#include "gtest/gtest.h"

bool debug;

TEST(JSON, storeobject) {
  string in_str("Test");
  JSON j;
  j.storeobject (&in_str);
}

int main (int argc, char *argv[])
{
    return RUN_ALL_TESTS();
}
