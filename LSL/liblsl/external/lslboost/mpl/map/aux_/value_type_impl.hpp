
#ifndef BOOST_MPL_MAP_AUX_VALUE_TYPE_IMPL_HPP_INCLUDED
#define BOOST_MPL_MAP_AUX_VALUE_TYPE_IMPL_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.lslboost.org/LICENSE_1_0.txt)
//
// See http://www.lslboost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <lslboost/mpl/value_type_fwd.hpp>
#include <lslboost/mpl/pair.hpp>
#include <lslboost/mpl/map/aux_/tag.hpp>

namespace lslboost {
namespace mpl {

template<>
struct value_type_impl< aux::map_tag >
{
    template< typename Map, typename T > struct apply
        : second<T>
    {
    };
};

}}

#endif // BOOST_MPL_MAP_AUX_VALUE_TYPE_IMPL_HPP_INCLUDED