//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_URI_URI_HPP
#define BOOST_BEAST_URI_URI_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/string.hpp>
#include <boost/beast/uri/detail/parts.hpp>

namespace boost {
namespace beast {
namespace uri {

namespace detail {
template<class Buffer>
class parser;
} // detail

template<class Buffer>
class basic_uri
{
    template<class Buffer>
    friend class detail::parser;

    detail::parts p_;
    Buffer b_;

    explicit
    basic_uri(detail::parts const& p)
        : p_(p)
    {
    }

public:
    basic_uri() = default;
    basic_uri(basic_uri&&) = default;
    basic_uri(basic_uri const&) = default;
    basic_uri& operator=(basic_uri&&) = default;
    basic_uri& operator=(basic_uri const&) = default;

    template<class... Args>
    explicit
    basic_uri(Args&&... args)
        : b_(std::forward<Args>(args)...)
    {
    }

    //
    // Observers
    //

    known_scheme
    scheme_value() const
    {
        return p_.scheme_value;
    }

    string_view
    scheme() const
    {
        return p_.scheme(b_.data());
    }

    //
    // Modifiers
    //

    void
    clear()
    {
        p_ = parse{};
        b_.clear();
    }
};

} // uri
} // beast
} // boost

#endif
