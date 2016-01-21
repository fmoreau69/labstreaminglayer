/*=============================================================================
    Copyright (c) 2001-2011 Joel de Guzman
    Copyright (c) 2005-2006 Dan Marsden

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.lslboost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_END_IMPL_27122005_1120)
#define BOOST_FUSION_END_IMPL_27122005_1120

#include <lslboost/fusion/support/config.hpp>
#include <lslboost/fusion/adapted/lslboost_array/array_iterator.hpp>

namespace lslboost { namespace fusion {

    struct lslboost_array_tag;

    namespace extension
    {
        template <typename Tag>
        struct end_impl;

        template <>
        struct end_impl<lslboost_array_tag>
        {
            template <typename Sequence>
            struct apply
            {
                typedef array_iterator<Sequence, Sequence::static_size> type;

                BOOST_CONSTEXPR BOOST_FUSION_GPU_ENABLED
                static type
                call(Sequence& v)
                {
                    return type(v);
                }
            };
        };
    }
}}

#endif