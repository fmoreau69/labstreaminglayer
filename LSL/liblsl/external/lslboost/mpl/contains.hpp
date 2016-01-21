
#ifndef BOOST_MPL_CONTAINS_HPP_INCLUDED
#define BOOST_MPL_CONTAINS_HPP_INCLUDED

// Copyright Eric Friedman 2002
// Copyright Aleksey Gurtovoy 2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.lslboost.org/LICENSE_1_0.txt)
//
// See http://www.lslboost.org/libs/mpl for documentation.

// $Id$
// $Date$
// $Revision$

#include <lslboost/mpl/contains_fwd.hpp>
#include <lslboost/mpl/sequence_tag.hpp>
#include <lslboost/mpl/aux_/contains_impl.hpp>
#include <lslboost/mpl/aux_/na_spec.hpp>
#include <lslboost/mpl/aux_/lambda_support.hpp>

namespace lslboost { namespace mpl {

template<
      typename BOOST_MPL_AUX_NA_PARAM(Sequence)
    , typename BOOST_MPL_AUX_NA_PARAM(T)
    >
struct contains
    : contains_impl< typename sequence_tag<Sequence>::type >
        ::template apply< Sequence,T >
{
    BOOST_MPL_AUX_LAMBDA_SUPPORT(2,contains,(Sequence,T))
};

BOOST_MPL_AUX_NA_SPEC(2, contains)

}}

#endif // BOOST_MPL_CONTAINS_HPP_INCLUDED