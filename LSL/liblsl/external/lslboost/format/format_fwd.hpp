// ----------------------------------------------------------------------------
//  format_fwd.hpp :  forward declarations
// ----------------------------------------------------------------------------

//  Copyright Samuel Krempp 2003. Use, modification, and distribution are
//  subject to the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.lslboost.org/LICENSE_1_0.txt)

//  See http://www.lslboost.org/libs/format for library home page

// ----------------------------------------------------------------------------

#ifndef BOOST_FORMAT_FWD_HPP
#define BOOST_FORMAT_FWD_HPP

#include <string>
#include <iosfwd>

#include <lslboost/format/detail/compat_workarounds.hpp> 

namespace lslboost {

    template <class Ch, 
        class Tr = BOOST_IO_STD char_traits<Ch>, class Alloc = std::allocator<Ch> >
    class basic_format;

    typedef basic_format<char >     format;

#if !defined(BOOST_NO_STD_WSTRING)  && !defined(BOOST_NO_STD_WSTREAMBUF)
    typedef basic_format<wchar_t >  wformat;
#endif

    namespace io {
        enum format_error_bits { bad_format_string_bit = 1, 
                                 too_few_args_bit = 2, too_many_args_bit = 4,
                                 out_of_range_bit = 8,
                                 all_error_bits = 255, no_error_bits=0 };
                  
    } // namespace io

} // namespace lslboost

#endif // BOOST_FORMAT_FWD_HPP