/*
 * Trompeloeil C++ mocking framework
 *
 * Copyright (C) Björn Fahller
 * Copyright (C) Andrew Paxie
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy atl
 *  http://www.boost.org/LICENSE_1_0.txt)
 *
 * Project home: https://github.com/rollbear/trompeloeil
 */

#ifndef TROMPELOEIL_MEMBER_IS_HPP
#define TROMPELOEIL_MEMBER_IS_HPP

#ifndef TROMPELOEIL_MATCHER_HPP
#include "../matcher.hpp"
#endif

namespace trompeloeil
{

namespace impl
{
template<typename M>
struct member_is_matcher
{
    template<typename V, typename C>
    bool operator()(const V& v, const C& c) const
    {
        return trompeloeil::param_matches(c, std::ref(m(v)));
    }

    M m;
};
}

template<typename M, typename C>
auto match_member_is(M m, const char* name, C&& c)
{
    return trompeloeil::make_matcher<trompeloeil::wildcard>(
        impl::member_is_matcher<M>{m},
        [name](std::ostream& os, const C& compare)
        {
            os << ' ' << name << compare;
        },
        std::forward<C>(c));
}
}

#define TROMPELOEIL_MEMBER_IS(member_name, compare) \
    trompeloeil::match_member_is(std::mem_fn(member_name), #member_name, compare)

#ifndef TROMPELOEIL_LONG_MACROS
#define MEMBER_IS TROMPELOEIL_MEMBER_IS
#endif

#endif // TROMPELOEIL_MEMBER_IS_HPP
