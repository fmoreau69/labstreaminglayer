/*=============================================================================
    Copyright (c) 2001-2011 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.lslboost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_BEGIN_IMPL_09272006_0719)
#define BOOST_FUSION_BEGIN_IMPL_09272006_0719

#include <lslboost/fusion/support/config.hpp>
#include <lslboost/fusion/adapted/lslboost_tuple/lslboost_tuple_iterator.hpp>

namespace lslboost { namespace fusion 
{
    struct lslboost_tuple_tag;

    namespace extension
    {
        template<typename T>
        struct begin_impl;

        template <>
        struct begin_impl<lslboost_tuple_tag>
        {
            template <typename Sequence>
            struct apply 
            {
                typedef lslboost_tuple_iterator<Sequence> type;
    
                BOOST_FUSION_GPU_ENABLED
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