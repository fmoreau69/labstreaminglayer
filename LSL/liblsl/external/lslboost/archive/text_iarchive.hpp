#ifndef BOOST_ARCHIVE_TEXT_IARCHIVE_HPP
#define BOOST_ARCHIVE_TEXT_IARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// text_iarchive.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.lslboost.org/LICENSE_1_0.txt)

//  See http://www.lslboost.org for updates, documentation, and revision history.

#include <istream>

#include <lslboost/config.hpp>
#include <lslboost/archive/detail/auto_link_archive.hpp>
#include <lslboost/archive/basic_text_iprimitive.hpp>
#include <lslboost/archive/basic_text_iarchive.hpp>
#include <lslboost/archive/detail/register_archive.hpp>
#include <lslboost/serialization/item_version_type.hpp>

#include <lslboost/archive/detail/abi_prefix.hpp> // must be the last header

#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4511 4512)
#endif

namespace lslboost { 
namespace archive {

namespace detail {
    template<class Archive> class interface_iarchive;
} // namespace detail

template<class Archive>
class BOOST_SYMBOL_VISIBLE text_iarchive_impl :
    public basic_text_iprimitive<std::istream>,
    public basic_text_iarchive<Archive>
{
#ifdef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
public:
#else
protected:
    #if BOOST_WORKAROUND(BOOST_MSVC, < 1500)
        // for some inexplicable reason insertion of "class" generates compile erro
        // on msvc 7.1
        friend detail::interface_iarchive<Archive>;
        friend load_access;
    #else
        friend class detail::interface_iarchive<Archive>;
        friend class load_access;
    #endif
#endif
    template<class T>
    void load(T & t){
        basic_text_iprimitive<std::istream>::load(t);
    }
    void load(version_type & t){
        unsigned int v;
        load(v);
        t = version_type(v);
    }
    void load(lslboost::serialization::item_version_type & t){
        unsigned int v;
        load(v);
        t = lslboost::serialization::item_version_type(v);
    }
    BOOST_ARCHIVE_DECL void 
    load(char * t);
    #ifndef BOOST_NO_INTRINSIC_WCHAR_T
    BOOST_ARCHIVE_DECL void 
    load(wchar_t * t);
    #endif
    BOOST_ARCHIVE_DECL void 
    load(std::string &s);
    #ifndef BOOST_NO_STD_WSTRING
    BOOST_ARCHIVE_DECL void 
    load(std::wstring &ws);
    #endif
    template<class T>
    void load_override(T & t){
        basic_text_iarchive<Archive>::load_override(t);
    }
    BOOST_ARCHIVE_DECL void
    load_override(class_name_type & t);
    BOOST_ARCHIVE_DECL void
    init();
    BOOST_ARCHIVE_DECL 
    text_iarchive_impl(std::istream & is, unsigned int flags);
    // don't import inline definitions! leave this as a reminder.
    //BOOST_ARCHIVE_DECL 
    ~text_iarchive_impl(){};
};

} // namespace archive
} // namespace lslboost

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

#include <lslboost/archive/detail/abi_suffix.hpp> // pops abi_suffix.hpp pragmas

#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable : 4511 4512)
#endif

namespace lslboost { 
namespace archive {

class BOOST_SYMBOL_VISIBLE text_iarchive : 
    public text_iarchive_impl<text_iarchive>{
public:
    text_iarchive(std::istream & is_, unsigned int flags = 0) :
        // note: added _ to suppress useless gcc warning
        text_iarchive_impl<text_iarchive>(is_, flags)
    {}
    ~text_iarchive(){}
};

} // namespace archive
} // namespace lslboost

// required by export
BOOST_SERIALIZATION_REGISTER_ARCHIVE(lslboost::archive::text_iarchive)

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

#endif // BOOST_ARCHIVE_TEXT_IARCHIVE_HPP