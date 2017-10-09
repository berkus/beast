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

#include <boost/beast/core/string.hpp>
#include <boost/beast/uri/error.hpp>
#include <boost/beast/uri/scheme.hpp>
#include <boost/beast/uri/detail/rfc3986.hpp>
#include <boost/assert.hpp>

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

struct parts
{
    known_scheme scheme = known_scheme::unknown;
    piece scheme_string;
    piece authority;
        piece userinfo;
            piece username;
            piece password;
        piece host;
        piece port;
    piece path;
    piece query;
    piece fragment;
};

//------------------------------------------------------------------------------

template<class Buffer>
class parser
{
    friend class parse_test;

    struct input
    {
        char const* begin;
        char const* pos;
        char const* end;

        explicit
        input(string_view s)
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

    struct output
    {
        char* begin = nullptr;
        char* mark = nullptr;
        char* pos = nullptr;
        char* end = nullptr;
        Buffer& buffer;

        output(Buffer& buffer_)
            : buffer(buffer_)
        {
            reset(buffer.data(), buffer.size());
        }

        bool
        full() const
        {
            return pos >= end;
        }

        std::size_t
        remain() const
        {
            return static_cast<std::size_t>(end - pos);
        }

        void
        reset(char* begin_, std::size_t size)
        {
            std::size_t const n1 = mark - begin;
            std::size_t const n2 = pos - begin;
            begin = begin_;
            mark = begin + n1;
            pos = begin + n2;
            end = begin + size;
        }

        piece
        extract()
        {
            piece p{begin, mark, pos};
            mark = pos;
            return p;
        }
    };

    parts& p_;
    input in_;
    output out_;

public:
    parser(
        string_view s,
        parts& p,
        Buffer& b)
        : p_(p)
        , in_(s)
        , out_(b)
    {
    }

   /*
        5.3 Request Target
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
    parse_origin_form(error_code& ec)
    {
        ec.assign(0, ec.category());
    }

    /*  Used in requests to a proxy, except for CONNECT or OPTIONS *

        absolute-URI    = scheme ":" hier-part [ "?" query ]

        https://tools.ietf.org/html/rfc3986#section-4.3
    */
    void
    parse_absolute_form(error_code& ec)
    {
        ec.assign(0, ec.category());
        parse_scheme(ec);
        if(ec)
            return;
        parse_hier_part(ec);
        if(ec)
            return;
    }

    /*  Used in CONNECT requests

        The authority-form of request-target is only used for CONNECT requests
        https://tools.ietf.org/html/rfc7230#section-5.3.3

        Although CONNECT must exclude userinfo and '@' we parse it anyway and
        let the caller decide what to do with it.

        authority-form  = authority
    */
    inline
    void
    parse_authority_form(error_code& ec)
    {
        ec.assign(0, ec.category());
    }

    /*  Used for server-wide OPTIONS requests
    */
    void
    parse_asterisk_form(error_code& ec)
    {
        ec.assign(0, ec.category());
    }

private:
    /*
        scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ) ":"
    */
    void
    parse_scheme(error_code& ec)
    {
        if(in_.empty())
        {
            // bad scheme
            ec = error::syntax;
            return;
        }
        auto it = in_.pos;
        if(! is_alpha(*it))
        {
            // bad scheme
            ec = error::syntax;
            return;
        }
        for(;;)
        {
            ++it;
            if(it >= in_.end)
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
        p_.scheme_string = in_.extract(it);
        p_.scheme = string_to_scheme(p_.scheme_string(in_.begin));
        ++in_.pos; // skip ':'
    }

    /*
       hier-part    = "//" authority path-abempty
                    / path-absolute
                    / path-rootless
                    / path-empty
    */
    inline
    void
    parse_hier_part(error_code& ec)
    {
        if( in_.remain() >= 2 &&
            in_.pos[0] == '/' &&
            in_.pos[1] == '/')
        {
            parse_authority(ec);
            if(ec)
                return;
            parse_path_abempty(ec);
            if(ec)
                return;
        }

    }

    //--------------------------------------------------------------------------

    /*
    */
    void
    parse_authority(error_code& ec)
    {
    /*
        https://tools.ietf.org/html/rfc3986#section-3.2
        The authority component is preceded by a double slash ("//") and is
        terminated by the next slash ("/"), question mark ("?"), or number
        sign ("#") character, or by the end of the URI.
    */
        auto it = in_.pos;
        for(; it != in_.end; ++it)
            if( *it == '/' ||
                *it == '?' ||
                *it == '#')
                break;
        if(it != in_.pos)
            p_.authority = in_.extract(it);
    }

    /*
        path-abempty    = *( "/" segment )
        segment         = *pchar

    */
    void
    parse_path_abempty(error_code& ec)
    {
    }

    //--------------------------------------------------------------------------

    /*
        literal     = CHAR
    */
    void
    parse_literal(input& c, char ch, error_code& ec)
    {
        if(in_.empty())
        {
            // expected CHAR
            ec = error::syntax;
            return;
        }
        if(*in_.pos != ch)
        {
            // expected literal
            ec = error::syntax;
            return;
        }
        ++in_.pos;
    }

    //--------------------------------------------------------------------------

    void
    append(char c)
    {
        if(out_.pos < out_.end)
        {
            *out_.pos++ = c;
            return;
        }
        throw std::bad_alloc{};
    }
};

template<unsigned short N>
class static_buffer
{
    char buf_[N];

public:
    char*
    data()
    {
        return buf_;
    }

    std::size_t
    size() const
    {
        return N;
    }
};

} // detail

} // uri
} // beast
} // boost

#endif
