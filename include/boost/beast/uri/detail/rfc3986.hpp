//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_URI_DETAIL_RFC3986_HPP
#define BOOST_BEAST_URI_DETAIL_RFC3986_HPP

namespace boost {
namespace beast {
namespace uri {
namespace detail {

inline
bool
is_alpha(char c)
{
    unsigned constexpr a = 'a';
    return ((static_cast<unsigned>(c) | 32) - a) < 26U;
}

inline
bool
is_digit(char c)
{
    unsigned constexpr zero = '0';
    return (static_cast<unsigned>(c) - zero) < 10;
}

inline
char
to_lower(char c)
{
    if((static_cast<unsigned>(c) - 65U) < 26)
        return c + ('a' - 'A');
    return c;
}

} // detail
} // uri
} // beast
} // boost

#endif
