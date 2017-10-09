//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_URI_DETAIL_TYPES_HPP
#define BOOST_BEAST_URI_DETAIL_TYPES_HPP

#include <boost/beast/core/string.hpp>
#include <boost/beast/uri/scheme.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace beast {
namespace uri {

namespace detail {

struct piece
{
    unsigned short offset = 0;
    unsigned short size = 0;

    piece() = default;

    piece(
        char const* base,
        char const* first, char const* last)
        : offset(static_cast<unsigned short>(first - base))
        , size(static_cast<unsigned short>(last - first))
    {
        BOOST_ASSERT(first - base < (
            std::numeric_limits<unsigned short>::max)());
        BOOST_ASSERT(last - first < (
            std::numeric_limits<unsigned short>::max)());
    }

    bool
    empty() const
    {
        return size == 0;
    }

    explicit
    operator bool() const
    {
        return ! empty();
    }

    string_view
    operator()(char const* base) const
    {
        return {base + offset, size};
    }
};

struct cursor
{
    char const* begin;
    char const* pos;
    char const* end;

    explicit
    cursor(string_view s)
        : begin(s.data())
        , pos(begin)
        , end(begin + s.size())
    {
    }

    bool
    empty() const
    {
        return pos >= end;
    }

    std::size_t
    remain() const
    {
        return static_cast<std::size_t>(end - pos);
    }

    piece
    extract(char const* it)
    {
        piece p{begin, pos, it};
        pos = it;
        return p;
    }
};

} // detail

} // uri
} // beast
} // boost

#endif
