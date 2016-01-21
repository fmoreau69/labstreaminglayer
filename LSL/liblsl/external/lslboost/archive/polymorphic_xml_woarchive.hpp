#ifndef BOOST_ARCHIVE_POLYMORPHIC_XML_WOARCHIVE_HPP
#define BOOST_ARCHIVE_POLYMORPHIC_XML_WOARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// polymorphic_xml_oarchive.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.lslboost.org/LICENSE_1_0.txt)

//  See http://www.lslboost.org for updates, documentation, and revision history.

#include <lslboost/config.hpp>
#ifdef BOOST_NO_STD_WSTREAMBUF
#error "wide char i/o not supported on this platform"
#else

#include <lslboost/archive/xml_woarchive.hpp>
#include <lslboost/archive/detail/polymorphic_oarchive_route.hpp>

namespace lslboost { 
namespace archive {

typedef detail::polymorphic_oarchive_route<
        xml_woarchive_impl<xml_woarchive> 
> polymorphic_xml_woarchive;

} // namespace archive
} // namespace lslboost

// required by export
BOOST_SERIALIZATION_REGISTER_ARCHIVE(
    lslboost::archive::polymorphic_xml_woarchive
)

#endif // BOOST_NO_STD_WSTREAMBUF
#endif // BOOST_ARCHIVE_POLYMORPHIC_XML_WOARCHIVE_HPP
