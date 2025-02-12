/*
 * Trompeloeil C++ mocking framework
 *
 * Copyright (C) Björn Fahller
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy atl
 *  http://www.boost.org/LICENSE_1_0.txt)
 *
 * Project home: https://github.com/rollbear/trompeloeil
 */

#ifndef TROMPELOEIL_SET_PREDICATE_HPP
#define TROMPELOEIL_SET_PREDICATE_HPP

#ifndef TROMPELOEIL_MATCHER_HPP
#include "../matcher.hpp"
#endif

namespace trompeloeil
{
namespace impl
{
struct any_of_checker
{
    template<typename T, typename... Cs>
    bool operator()(const T& t, const Cs&... compare) const
    {
        bool any_true = false;
        trompeloeil::ignore(std::initializer_list<bool>{
            (any_true = any_true || trompeloeil::param_matches(compare, std::ref(t)))...});
        return any_true;
    }
};

struct any_of_printer
{
    template<typename... Cs>
    void operator()(std::ostream& os, const Cs&... compare) const
    {
        os << " to be any of {";
        const char* sep = " ";
        trompeloeil::ignore(
            std::initializer_list<int>{((os << detail::exchange(sep, ", ") << compare), 0)...});
        os << " }";
    }
};
}

template<typename Type = trompeloeil::wildcard,
         typename... Cs,
         typename R = make_matcher_return<Type, impl::any_of_checker, impl::any_of_printer, Cs...> >
R any_of(Cs&&... cs)
{
    return trompeloeil::make_matcher<Type>(impl::any_of_checker{},
                                           impl::any_of_printer{},
                                           std::forward<Cs>(cs)...);
}

namespace impl
{
struct none_of_checker
{
    template<typename T, typename... Cs>
    bool operator()(const T& t, const Cs&... compare) const
    {
        bool any_true = false;
        trompeloeil::ignore(std::initializer_list<bool>{
            (any_true = any_true || trompeloeil::param_matches(compare, std::ref(t)))...});
        return !any_true;
    }
};

struct none_of_printer
{
    template<typename... Cs>
    void operator()(std::ostream& os, const Cs&... compare) const
    {
        os << " to be none of {";
        const char* sep = " ";
        trompeloeil::ignore(
            std::initializer_list<int>{((os << detail::exchange(sep, ", ") << compare), 0)...});
        os << " }";
    }
};
}

template<
    typename Type = trompeloeil::wildcard,
    typename... Cs,
    typename R = make_matcher_return<Type, impl::none_of_checker, impl::none_of_printer, Cs...> >
R none_of(Cs&&... cs)
{
    return trompeloeil::make_matcher<Type>(impl::none_of_checker{},
                                           impl::none_of_printer{},
                                           std::forward<Cs>(cs)...);
}

namespace impl
{
struct all_of_checker
{
    template<typename T, typename... Cs>
    bool operator()(const T& t, const Cs&... compare) const
    {
        bool all_true = true;
        trompeloeil::ignore(std::initializer_list<bool>{
            (all_true = all_true && trompeloeil::param_matches(compare, std::ref(t)))...});
        return all_true;
    }
};

struct all_of_printer
{
    template<typename... Cs>
    void operator()(std::ostream& os, const Cs&... compare) const
    {
        os << " to be all of {";
        const char* sep = " ";
        trompeloeil::ignore(
            std::initializer_list<int>{((os << detail::exchange(sep, ", ") << compare), 0)...});
        os << " }";
    }
};
}

template<typename Type = trompeloeil::wildcard,
         typename... Cs,
         typename R = make_matcher_return<Type, impl::all_of_checker, impl::all_of_printer, Cs...> >
R all_of(Cs&&... cs)
{
    return trompeloeil::make_matcher<Type>(impl::all_of_checker{},
                                           impl::all_of_printer{},
                                           std::forward<Cs>(cs)...);
}

}
#endif // TROMPELOEIL_SET_PREDICATE_HPP
