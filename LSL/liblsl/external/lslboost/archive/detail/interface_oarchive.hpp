#ifndef BOOST_ARCHIVE_DETAIL_INTERFACE_OARCHIVE_HPP
#define BOOST_ARCHIVE_DETAIL_INTERFACE_OARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// interface_oarchive.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.lslboost.org/LICENSE_1_0.txt)

//  See http://www.lslboost.org for updates, documentation, and revision history.
#include <cstddef> // NULL
#include <lslboost/cstdint.hpp>
#include <lslboost/mpl/bool.hpp>

#include <lslboost/archive/detail/auto_link_archive.hpp>
#include <lslboost/archive/detail/oserializer.hpp>
#include <lslboost/archive/detail/abi_prefix.hpp> // must be the last header

#include <lslboost/serialization/singleton.hpp>

namespace lslboost {
namespace archive {
namespace detail {

class BOOST_ARCHIVE_OR_WARCHIVE_DECL basic_pointer_oserializer;

template<class Archive>
class interface_oarchive 
{
protected:
    interface_oarchive(){};
public:
    /////////////////////////////////////////////////////////
    // archive public interface
    typedef mpl::bool_<false> is_loading;
    typedef mpl::bool_<true> is_saving;

    // return a pointer to the most derived class
    Archive * This(){
        return static_cast<Archive *>(this);
    }

    template<class T>
    const basic_pointer_oserializer * 
    register_type(const T * = NULL){
        const basic_pointer_oserializer & bpos =
            lslboost::serialization::singleton<
                pointer_oserializer<Archive, T>
            >::get_const_instance();
        this->This()->register_basic_serializer(bpos.get_basic_serializer());
        return & bpos;
    }
    
    template<class Helper>
    Helper &
    get_helper(void * const id = 0){
        helper_collection & hc = this->This()->get_helper_collection();
        return hc.template find_helper<Helper>(id);
    }

    template<class T>
    Archive & operator<<(const T & t){
        this->This()->save_override(t);
        return * this->This();
    }
    
    // the & operator 
    template<class T>
    Archive & operator&(const T & t){
        return * this ->This() << t;
    };
};

} // namespace detail
} // namespace archive
} // namespace lslboost

#include <lslboost/archive/detail/abi_suffix.hpp> // pops abi_suffix.hpp pragmas

#endif // BOOST_ARCHIVE_DETAIL_INTERFACE_IARCHIVE_HPP