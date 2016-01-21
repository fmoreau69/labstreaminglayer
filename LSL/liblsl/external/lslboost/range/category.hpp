// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2006. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.lslboost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.lslboost.org/libs/range/
//

#ifndef BOOST_RANGE_CATEGORY_HPP
#define BOOST_RANGE_CATEGORY_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include <lslboost/range/config.hpp>
#include <lslboost/range/iterator.hpp>
#include <lslboost/iterator/iterator_traits.hpp>

namespace lslboost
{
    template< class T >
    struct range_category : iterator_category< typename range_iterator<T>::type >
    { };
}

#endif