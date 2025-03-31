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

#ifndef TROMPELOEIL_RANGE_HPP
#define TROMPELOEIL_RANGE_HPP

#ifndef TROMPELOEIL_MATCHER_HPP
#include "../matcher.hpp"
#endif

#include <vector>

namespace trompeloeil
{

namespace impl
{

template<typename... Ts>
struct disjunction
{
    static constexpr bool test()
    {
        bool all_false = false;
        ignore(std::initializer_list<bool>{(all_false = all_false || Ts::value)...});
        return all_false;
    }

    static constexpr bool value = test();
};

template<typename E, typename M>
auto make_predicate_matcher(M& m)
{
    return std::function<bool(const E&)>(
        [&m](const E& value)
        {
            return trompeloeil::param_matches(m, std::ref(value));
        });
}

template<typename T>
struct store_as
{
    using type = T;

    template<typename C>
    static type make(C&& c)
    {
        return T(std::forward<C>(c));
    }
};

template<typename T, size_t N>
struct store_as<T (&)[N]>
{
    using type = mini_span<T>;

    static type make(T* p)
    {
        return type(p, N);
    }
};

template<typename T>
using store_as_t = typename store_as<T>::type;

template<typename, typename = void>
struct value_type
{};

template<typename T, unsigned N>
struct value_type<T (&)[N], void>
{
    using type = T&;
};

template<typename T>
struct value_type<T, void_t<decltype(*std::declval<T&>().begin())>>
{
    using type = decltype(*std::declval<T&>().begin());
};

template<typename R>
using value_type_t = typename value_type<R>::type;

template<typename T, typename = void>
struct is_range: std::false_type
{};

template<typename T>
struct is_range<T, void_t<value_type_t<T>>>: std::true_type
{};

template<typename T>
constexpr bool is_range_v = is_range<T>::value;
}

namespace impl
{
struct is_elements_checker
{
    template<typename R, typename... Es>
    bool operator()(const R& range, const Es&... elements) const
    {
        using std::begin;
        using std::end;
        auto it = begin(range);
        const auto e = end(range);
        bool all_true = true;
        const auto match = [&](const auto& compare)
        {
            if (it == e)
                return false;
            const auto& v = *it++;
            return trompeloeil::param_matches(compare, std::ref(v));
        };
        trompeloeil::ignore(
            std::initializer_list<bool>{(all_true = all_true && match(elements))...});
        return all_true && it == e;
    }
};

struct is_elements_printer
{
    template<typename... Es>
    void operator()(std::ostream& os, const Es&... elements) const
    {
        os << " range is {";
        const char* sep = "";
        const auto print = [&](const auto& v)
        {
            os << std::exchange(sep, ", ") << v;
            return 0;
        };
        trompeloeil::ignore(std::initializer_list<int>{(print(elements))...});
        os << " }";
    }
};

}

template<typename Type = trompeloeil::wildcard,
         typename... Es,
         typename = typename std::enable_if<!impl::disjunction<impl::is_range<Es>...>::value>::type,
         typename R = make_matcher_return<Type,
                                          impl::is_elements_checker,
                                          impl::is_elements_printer,
                                          impl::store_as_t<Es>...>>
R range_is(Es&&... es)
{
    return trompeloeil::make_matcher<Type>(impl::is_elements_checker{},
                                           impl::is_elements_printer{},
                                           std::forward<Es>(es)...);
}

namespace impl
{
struct is_range_checker
{
    template<typename R, typename C>
    bool operator()(const R& r, const C& cs) const
    {
        using std::begin;
        using std::end;
        return std::equal(begin(r),
                          end(r),
                          begin(cs),
                          end(cs),
                          [](const auto& rv, const auto& cv)
                          {
                              return trompeloeil::param_matches(cv, std::ref(rv));
                          });
    }
};

struct is_range_printer
{
    template<typename C>
    void operator()(std::ostream& os, const C& cs) const
    {
        os << " range is {";
        const char* sep = " ";
        for (auto& c: cs)
        {
            os << std::exchange(sep, ", ") << c;
        }
        os << " }";
    }
};
}

template<typename Type = trompeloeil::wildcard,
         typename C,
         typename = impl::value_type_t<C>,
         typename R = make_matcher_return<Type,
                                          impl::is_range_checker,
                                          impl::is_range_printer,
                                          impl::store_as_t<C>>>
R range_is(C&& c)
{
    return trompeloeil::make_matcher<Type>(impl::is_range_checker{},
                                           impl::is_range_printer{},
                                           impl::store_as<C>::make(std::forward<C>(c)));
}

namespace impl
{
struct is_permutation_elements_checker
{
    template<typename R, typename... Es>
    bool operator()(const R& range, const Es&... elements) const
    {
        using std::begin;
        using std::end;
        auto it = begin(range);
        const auto e = end(range);
        using element_type = decltype(*it);
        std::vector<std::function<bool(const element_type&)>> matchers{
            make_predicate_matcher<element_type>(elements)...};
        while (it != e)
        {
            auto found = std::find_if(matchers.begin(),
                                      matchers.end(),
                                      [it](const auto& matcher)
                                      {
                                          return matcher(*it);
                                      });
            if (found == matchers.end())
            {
                break;
            }
            *found = std::move(matchers.back());
            matchers.pop_back();
            ++it;
        }
        return it == e && matchers.empty();
    }
};

struct is_permutation_elements_printer
{
    template<typename... Es>
    void operator()(std::ostream& os, const Es&... elements) const
    {
        os << " range is permutation of {";
        const char* sep = "";
        const auto print = [&](const auto& v)
        {
            os << std::exchange(sep, ", ") << v;
            return 0;
        };
        trompeloeil::ignore(std::initializer_list<int>{(print(elements))...});
        os << " }";
    }
};

}

template<typename Type = trompeloeil::wildcard,
         typename... Es,
         typename = typename std::enable_if<!impl::disjunction<impl::is_range<Es>...>::value>::type,
         typename R = make_matcher_return<Type,
                                          impl::is_permutation_elements_checker,
                                          impl::is_permutation_elements_printer,
                                          Es...>>
R range_is_permutation(Es&&... es)
{
    return trompeloeil::make_matcher<Type>(impl::is_permutation_elements_checker{},
                                           impl::is_permutation_elements_printer{},
                                           std::forward<Es>(es)...);
}

namespace impl
{
struct is_permutation_range_checker
{
    template<typename R, typename C>
    bool operator()(const R& range, const C& elements) const
    {
        using std::begin;
        using std::end;
        auto it = begin(range);
        const auto e = end(range);
        using element_type = decltype(*it);
        std::vector<std::function<bool(const element_type&)>> matchers;
        for (const auto& element: elements)
        {
            matchers.push_back(impl::make_predicate_matcher<element_type>(element));
        }
        while (it != e)
        {
            auto found = std::find_if(matchers.begin(),
                                      matchers.end(),
                                      [it](const auto& matcher)
                                      {
                                          return matcher(*it);
                                      });
            if (found == matchers.end())
            {
                break;
            }
            *found = std::move(matchers.back());
            matchers.pop_back();
            ++it;
        }
        return it == e && matchers.empty();
    }
};

struct is_permutation_range_printer
{
    template<typename C>
    void operator()(std::ostream& os, const C& c) const
    {
        os << " range is permutation of {";
        const char* sep = " ";
        for (const auto& v: c)
        {
            os << std::exchange(sep, ", ") << v;
        };

        os << " }";
    }
};
}

template<typename Type = trompeloeil::wildcard,
         typename C,
         typename R = make_matcher_return<Type,
                                          impl::is_permutation_range_checker,
                                          impl::is_permutation_range_printer,
                                          impl::store_as_t<C>>>
R range_is_permutation(C&& c)
{
    return trompeloeil::make_matcher<Type>(impl::is_permutation_range_checker{},
                                           impl::is_permutation_range_printer{},
                                           impl::store_as<C>::make(std::forward<C>(c)));
}

namespace impl
{
struct includes_elements_checker
{
    template<typename R, typename... Es>
    bool operator()(const R& range, const Es&... elements) const
    {
        using std::begin;
        using std::end;
        auto it = begin(range);
        const auto e = end(range);
        using element_type = decltype(*it);
        std::vector<std::function<bool(const element_type&)>> matchers{
            make_predicate_matcher<element_type>(elements)...};
        while (it != e)
        {
            auto found = std::find_if(matchers.begin(),
                                      matchers.end(),
                                      [it](const auto& matcher)
                                      {
                                          return matcher(*it);
                                      });
            if (found != matchers.end())
            {
                *found = std::move(matchers.back());
                matchers.pop_back();
            }
            ++it;
        }
        return matchers.empty();
    }
};

struct includes_elements_printer
{
    template<typename... Es>
    void operator()(std::ostream& os, const Es&... elements) const
    {
        os << " range has {";
        const char* sep = "";
        const auto print = [&](const auto& v)
        {
            os << std::exchange(sep, ", ") << v;
            return 0;
        };
        trompeloeil::ignore(std::initializer_list<int>{(print(elements))...});
        os << " }";
    }
};

}

template<typename Type = trompeloeil::wildcard,
         typename... Es,
         typename = typename std::enable_if<!impl::disjunction<impl::is_range<Es>...>::value>::type,
         typename R = make_matcher_return<Type,
                                          impl::includes_elements_checker,
                                          impl::includes_elements_printer,
                                          Es...>>
R range_includes(Es&&... es)
{
    return trompeloeil::make_matcher<Type>(impl::includes_elements_checker{},
                                           impl::includes_elements_printer{},
                                           std::forward<Es>(es)...);
}

namespace impl
{
struct includes_range_checker
{
    template<typename R, typename C>
    bool operator()(const R& range, const C& elements) const
    {
        using std::begin;
        using std::end;
        auto it = begin(range);
        const auto e = end(range);
        using element_type = decltype(*it);
        std::vector<std::function<bool(const element_type&)>> matchers;
        for (auto& element: elements)
        {
            matchers.push_back(make_predicate_matcher<element_type>(element));
        }
        while (it != e)
        {
            auto found = std::find_if(matchers.begin(),
                                      matchers.end(),
                                      [it](const auto& matcher)
                                      {
                                          return matcher(*it);
                                      });
            if (found != matchers.end())
            {
                *found = std::move(matchers.back());
                matchers.pop_back();
            }
            ++it;
        }
        return matchers.empty();
    }
};

struct includes_range_printer
{
    template<typename E>
    void operator()(std::ostream& os, const E& elements) const
    {
        os << " range has {";
        const char* sep = " ";
        for (const auto& v: elements)
        {
            os << std::exchange(sep, ", ") << v;
        };
        os << " }";
    }
};
}

template<typename Type = trompeloeil::wildcard,
         typename C,
         typename = typename std::enable_if<impl::is_range_v<C>>::type,
         typename R = make_matcher_return<Type,
                                          impl::includes_range_checker,
                                          impl::includes_range_printer,
                                          impl::store_as_t<C>>>
R range_includes(C&& c)
{
    return trompeloeil::make_matcher<Type>(impl::includes_range_checker{},
                                           impl::includes_range_printer{},
                                           impl::store_as<C>::make(std::forward<C>(c)));
}

namespace impl
{
struct range_all_of_checker
{
    template<typename R, typename C>
    bool operator()(const R& range, const C& comp) const
    {
        using std::begin;
        using std::end;
        auto it = begin(range);
        const auto e = end(range);
        return std::all_of(it,
                           e,
                           [&](const auto& t)
                           {
                               return trompeloeil::param_matches(comp, std::ref(t));
                           });
    }
};

struct range_all_of_printer
{
    template<typename C>
    void operator()(std::ostream& os, const C& comp) const
    {
        os << " range is all";
        trompeloeil::print_expectation(os, comp);
    }
};
}

template<typename Type = trompeloeil::wildcard,
         typename C,
         typename R =
             make_matcher_return<Type, impl::range_all_of_checker, impl::range_all_of_printer, C>>
R range_all_of(C&& compare)
{
    return trompeloeil::make_matcher<Type>(impl::range_all_of_checker{},
                                           impl::range_all_of_printer{},
                                           std::forward<C>(compare));
}

namespace impl
{
struct range_none_of_checker
{
    template<typename R, typename C>
    bool operator()(const R& range, const C& comp) const
    {
        using std::begin;
        using std::end;
        auto it = begin(range);
        const auto e = end(range);
        return std::none_of(it,
                            e,
                            [&](const auto& t)
                            {
                                return trompeloeil::param_matches(comp, std::ref(t));
                            });
    }
};

struct range_none_of_printer
{
    template<typename C>
    void operator()(std::ostream& os, const C& comp) const
    {
        os << " range is none";
        trompeloeil::print_expectation(os, comp);
    }
};
}

template<typename Type = trompeloeil::wildcard,
         typename C,
         typename R =
             make_matcher_return<Type, impl::range_none_of_checker, impl::range_none_of_printer, C>>
R range_none_of(C&& compare)
{
    return trompeloeil::make_matcher<Type>(impl::range_none_of_checker{},
                                           impl::range_none_of_printer{},
                                           std::forward<C>(compare));
}

namespace impl
{
struct range_any_of_checker
{
    template<typename R, typename C>
    bool operator()(const R& range, const C& comp) const
    {
        using std::begin;
        using std::end;
        auto it = begin(range);
        const auto e = end(range);
        return std::any_of(it,
                           e,
                           [&](const auto& t)
                           {
                               return trompeloeil::param_matches(comp, std::ref(t));
                           });
    }
};

struct range_any_of_printer
{
    template<typename C>
    void operator()(std::ostream& os, const C& comp) const
    {
        os << " range is any";
        trompeloeil::print_expectation(os, comp);
    }
};
}

template<typename Type = trompeloeil::wildcard,
         typename C,
         typename R =
             make_matcher_return<Type, impl::range_any_of_checker, impl::range_any_of_printer, C>>
R range_any_of(C&& compare)
{
    return trompeloeil::make_matcher<Type>(impl::range_any_of_checker{},
                                           impl::range_any_of_printer{},
                                           std::forward<C>(compare));
}

namespace impl
{
struct starts_with_elements_checker
{
    template<typename R, typename... Es>
    bool operator()(const R& range, const Es&... elements) const
    {
        using std::begin;
        using std::end;
        auto it = begin(range);
        const auto e = end(range);
        bool all_true = true;
        const auto match = [&](const auto& compare)
        {
            if (it == e)
                return false;
            const auto& v = *it++;
            return trompeloeil::param_matches(compare, std::ref(v));
        };
        trompeloeil::ignore(
            std::initializer_list<bool>{(all_true = all_true && match(elements))...});
        return all_true;
    }
};

struct starts_with_elements_printer
{
    template<typename... Es>
    void operator()(std::ostream& os, const Es&... elements) const
    {
        os << " range starts with {";
        const char* sep = "";
        const auto print = [&](const auto& v)
        {
            os << std::exchange(sep, ", ") << v;
            return 0;
        };
        trompeloeil::ignore(std::initializer_list<int>{(print(elements))...});
        os << " }";
    }
};

}

template<typename Type = trompeloeil::wildcard,
         typename... Es,
         typename = typename std::enable_if<!impl::disjunction<impl::is_range<Es>...>::value>::type,
         typename R = make_matcher_return<Type,
                                          impl::starts_with_elements_checker,
                                          impl::starts_with_elements_printer,
                                          Es...>>
R range_starts_with(Es&&... es)
{
    return trompeloeil::make_matcher<Type>(impl::starts_with_elements_checker{},
                                           impl::starts_with_elements_printer{},
                                           std::forward<Es>(es)...);
}

namespace impl
{
struct starts_with_range_checker
{
    template<typename R, typename E>
    bool operator()(const R& range, const E& elements) const
    {
        using std::begin;
        using std::end;
        auto result = std::mismatch(begin(range),
                                    end(range),
                                    begin(elements),
                                    end(elements),
                                    [](const auto& t, const auto& c)
                                    {
                                        return trompeloeil::param_matches(c, std::ref(t));
                                    });
        return result.second == end(elements);
    }
};

struct starts_with_range_printer
{
    template<typename E>
    void operator()(std::ostream& os, const E& elements) const
    {
        os << " range starts with {";
        const char* sep = " ";
        for (const auto& v: elements)
        {
            os << std::exchange(sep, ", ") << v;
        }
        os << " }";
    }
};
}

template<typename Type = trompeloeil::wildcard,
         typename C,
         typename = typename std::enable_if<impl::is_range_v<C>>::type,
         typename R = make_matcher_return<Type,
                                          impl::starts_with_range_checker,
                                          impl::starts_with_range_printer,
                                          impl::store_as_t<C>>>
R range_starts_with(C&& c)
{
    return trompeloeil::make_matcher<Type>(impl::starts_with_range_checker{},
                                           impl::starts_with_range_printer{},
                                           impl::store_as<C>::make(std::forward<C>(c)));
}

namespace impl
{
struct ends_with_checker
{
    template<typename R, typename... Es>
    bool operator()(const R& range, const Es&... elements) const
    {
        using std::begin;
        using std::end;
        auto it = begin(range);
        const auto e = end(range);
        const auto num_values = static_cast<std::ptrdiff_t>(sizeof...(elements));
        const auto size = std::distance(it, e);
        if (size < num_values)
        {
            return false;
        }
        std::advance(it, size - num_values);
        bool all_true = true;
        const auto match = [&](const auto& compare)
        {
            const auto& v = *it++;
            return trompeloeil::param_matches(compare, std::ref(v));
        };
        trompeloeil::ignore(
            std::initializer_list<bool>{(all_true = all_true && match(elements))...});
        return all_true;
    }
};

struct ends_with_printer
{
    template<typename... Es>
    void operator()(std::ostream& os, const Es&... elements) const
    {
        os << " range ends with {";
        const char* sep = "";
        const auto print = [&](const auto& v)
        {
            os << std::exchange(sep, ", ") << v;
            return 0;
        };
        trompeloeil::ignore(std::initializer_list<int>{(print(elements))...});
        os << " }";
    }
};

}

template<
    typename Type = trompeloeil::wildcard,
    typename... Es,
    typename = typename std::enable_if<!impl::disjunction<impl::is_range<Es>...>::value>::type,
    typename R = make_matcher_return<Type, impl::ends_with_checker, impl::ends_with_printer, Es...>>
R range_ends_with(Es&&... es)
{
    return trompeloeil::make_matcher<Type>(impl::ends_with_checker{},
                                           impl::ends_with_printer{},
                                           std::forward<Es>(es)...);
}

namespace impl
{
struct ends_with_range_checker
{
    template<typename R, typename E>
    bool operator()(const R& range, const E& elements) const
    {
        using std::begin;
        using std::end;
        auto it = begin(range);
        const auto e = end(range);
        const auto num_values = static_cast<ptrdiff_t>(elements.size());
        const auto size = std::distance(it, e);
        if (size < num_values)
        {
            return false;
        }
        std::advance(it, size - num_values);
        auto result = std::mismatch(it,
                                    e,
                                    begin(elements),
                                    end(elements),
                                    [](const auto& v, const auto& c)
                                    {
                                        return trompeloeil::param_matches(c, std::ref(v));
                                    });
        return result.second == elements.end();
    }
};

struct ends_with_range_printer
{
    template<typename E>
    void operator()(std::ostream& os, const E& elements) const
    {
        os << " range ends with {";
        const char* sep = " ";
        for (const auto& e: elements)
        {
            os << std::exchange(sep, ", ") << e;
        };
        os << " }";
    }
};
}

template<typename Type = trompeloeil::wildcard,
         typename C,
         typename = typename std::enable_if<impl::is_range_v<C>>::type,
         typename R = make_matcher_return<Type,
                                          impl::ends_with_range_checker,
                                          impl::ends_with_range_printer,
                                          impl::store_as_t<C>>>
R range_ends_with(C&& c)
{
    return trompeloeil::make_matcher<Type>(impl::ends_with_range_checker{},
                                           impl::ends_with_range_printer{},
                                           impl::store_as<C>::make(std::forward<C>(c)));
}

}
#endif // TROMPELOEIL_RANGE_HPP
