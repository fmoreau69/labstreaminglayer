// ----------------------------------------------------------------------------
// msvc_disambiguater.hpp : msvc workarounds. (for put_{head|last} overloads)
//               the trick was described in lslboost's list  by Aleksey Gurtovoy
// ----------------------------------------------------------------------------

//  Copyright Samuel Krempp 2003. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.lslboost.org/LICENSE_1_0.txt)

// see http://www.lslboost.org/libs/format for library home page

// ----------------------------------------------------------------------------

#ifndef BOOST_MSVC_DISAMBIGUATER_HPP
#define BOOST_MSVC_DISAMBIGUATER_HPP

#if BOOST_WORKAROUND(__DECCXX_VER, BOOST_TESTED_AT(60590042))

#include <lslboost/format/group.hpp>
#include <ostream>

namespace lslboost {
namespace io {
namespace detail {

template< class Ch, class Tr, class T >
struct disambiguater
{
   template< typename U >
   static void put_head(BOOST_IO_STD basic_ostream<Ch, Tr>& os, group1<U> const& x, long)
   {
       os << group_head(x.a1_); 
   }
   static void put_head(BOOST_IO_STD basic_ostream<Ch, Tr>& os, T const& x, int)
   {
   }
   template< typename U >
   static void put_last(BOOST_IO_STD basic_ostream<Ch, Tr>& os, group1<U> const& x, long)
   {
       os << group_last(x.a1_); 
   }
   static void put_last(BOOST_IO_STD basic_ostream<Ch, Tr>& os, T const& x, int)
   {
     os << x;
   }
};

} // namespace detail
} // namespace io
} // namespace lslboost

#endif // -__DECCXX_VER

#endif // -BOOST_MSVC_DISAMBIGUATER_HPP