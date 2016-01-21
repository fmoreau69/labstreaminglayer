/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// binary_wiarchive.cpp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.lslboost.org/LICENSE_1_0.txt)

//  See http://www.lslboost.org for updates, documentation, and revision history.

#include <lslboost/config.hpp>

#ifdef BOOST_NO_STD_WSTREAMBUF
#error "wide char i/o not supported on this platform"
#else

#define BOOST_WARCHIVE_SOURCE
#include <lslboost/archive/binary_wiarchive.hpp>
#include <lslboost/archive/detail/archive_serializer_map.hpp>

// explicitly instantiate for this type of text stream
#include <lslboost/archive/impl/archive_serializer_map.ipp>
#include <lslboost/archive/impl/basic_binary_iprimitive.ipp>
#include <lslboost/archive/impl/basic_binary_iarchive.ipp>

namespace lslboost {
namespace archive {

// explicitly instantiate for this type of text stream
template class detail::archive_serializer_map<binary_wiarchive>;
template class basic_binary_iprimitive<
    binary_wiarchive,
    wchar_t, 
    std::char_traits<wchar_t> 
>;
template class basic_binary_iarchive<binary_wiarchive> ;
template class binary_iarchive_impl<
    binary_wiarchive, 
    wchar_t, 
    std::char_traits<wchar_t> 
>;

} // namespace archive
} // namespace lslboost

#endif  // BOOST_NO_STD_WSTREAMBUF
