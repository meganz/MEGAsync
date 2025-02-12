/*
 * Trompeloeil C++ mocking framework
 *
 * Copyright Björn Fahller 2014-2019
 * Copyright Tore Martin Hagen 2019
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 *
 * Project home: https://github.com/rollbear/trompeloeil
 */

#ifndef TROMPELOEIL_CATCH2_HPP_
#define TROMPELOEIL_CATCH2_HPP_

#if !((defined(CATCH_CONFIG_PREFIX_ALL) && defined(CATCH_CHECK)) || \
      (!defined(CATCH_CONFIG_PREFIX_ALL) && defined(CHECK)))
#error "Catch2 macros must be defined before including <catch2/trompeloeil.hpp>"
#endif

#include "../trompeloeil.hpp"

namespace trompeloeil
{
template<>
inline void
    reporter<specialized>::send(severity s, const char* file, unsigned long line, const char* msg)
{
    std::ostringstream os;
    if (line)
        os << file << ':' << line << '\n';
    os << msg;
    auto failure = os.str();
    if (s == severity::fatal)
    {
#ifdef CATCH_CONFIG_PREFIX_ALL
        CATCH_FAIL(failure);
#else
        FAIL(failure);
#endif
    }
    else
    {
#ifdef CATCH_CONFIG_PREFIX_ALL
        CATCH_CAPTURE(failure);
        CATCH_CHECK(failure.empty());
#else
        CAPTURE(failure);
        CHECK(failure.empty());
#endif
    }
}

template<>
inline void reporter<specialized>::sendOk(const char* trompeloeil_mock_calls_done_correctly)
{
#ifdef CATCH_CONFIG_PREFIX_ALL
    CATCH_REQUIRE(trompeloeil_mock_calls_done_correctly != 0);
#else
    REQUIRE(trompeloeil_mock_calls_done_correctly != 0);
#endif
}
}

#endif // TROMPELOEIL_CATCH2_HPP_
