//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_URI_DETAIL_PARSE_HPP
#define BOOST_BEAST_URI_DETAIL_PARSE_HPP

#include <boost/beast/uri/error.hpp>
#include <boost/beast/uri/detail/rfc3986.hpp>
#include <boost/beast/uri/detail/types.hpp>

namespace boost {
namespace beast {
namespace uri {

/*  References:

    Uniform Resource Identifier (URI): Generic Syntax
    https://tools.ietf.org/html/rfc3986

    Hypertext Transfer Protocol (HTTP/1.1): Semantics and Content
    https://tools.ietf.org/html/rfc7231

    Internationalized Resource Identifiers (IRIs)
    https://tools.ietf.org/html/rfc3987

    URL Living Standard
    https://url.spec.whatwg.org
*/

namespace detail {

/*
    literal     = CHAR
*/
inline
void
parse_literal(cursor& c, char ch, error_code& ec)
{
    if(c.empty())
    {
        // expected CHAR
        ec = error::syntax;
        return;
    }
    if(*c.pos != ch)
    {
        // expected literal
        ec = error::syntax;
        return;
    }
    ++c.pos;
}

/*
    scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
*/
inline
void
parse_scheme(
    raw_parts& r,
    cursor& c,
    error_code& ec)
{
    if(c.empty())
    {
        // bad scheme
        ec = error::syntax;
        return;
    }
    auto it = c.pos;
    if(! is_alpha(*it))
    {
        // bad scheme
        ec = error::syntax;
        return;
    }
    for(;;)
    {
        ++it;
        if(it >= c.end)
        {
            // bad scheme
            ec = error::syntax;
            return;
        }
        if(*it == ':')
            break;
        if( ! is_alpha(*it) &&
            ! is_digit(*it) &&
            *it != '+' &&
            *it != '-' &&
            *it != '.')
        {
            // bad scheme
            ec = error::syntax;
            return;
        }
    }
    r.scheme = c.extract(it);
    ++c.pos; // skip ':'
}

//------------------------------------------------------------------------------

/*  5.3 Request Target
    https://tools.ietf.org/html/rfc7230#section-5.3

    request-target  = origin-form
                    / absolute-form
                    / authority-form
                    / asterisk-form

    origin-form     = absolute-path [ "?" query ]

    absolute-form   = absolute-URI

    authority-form  = authority
 
    asterisk-form   = "*"
*/

/*  Used in direct requests to an origin server,
    except for CONNECT or OPTIONS *
*/
void
parse_origin_form()
{
}

/*  Used in requests to a proxy, except for CONNECT or OPTIONS *

    absolute-URI    = scheme ":" hier-part [ "?" query ]

    https://tools.ietf.org/html/rfc3986#section-4.3
*/
void
parse_absolute_form(
    raw_parts& r,
    cursor& c,
    error_code& ec)
{
    ec.assign(0, ec.category());
    parse_scheme(r, c, ec);
    if(ec)
        return;
}

/*  Used in CONNECT requests
*/
void
parse_authority_form()
{
}

/*  Used for server-wide OPTIONS requests
*/
void
parse_asterisk_form()
{
}

} // detail

} // uri
} // beast
} // boost

#endif
