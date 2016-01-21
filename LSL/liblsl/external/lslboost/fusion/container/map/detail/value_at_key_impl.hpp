/*=============================================================================
    Copyright (c) 2001-2013 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.lslboost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_FUSION_MAP_DETAIL_VALUE_AT_KEY_IMPL_02042013_0821)
#define BOOST_FUSION_MAP_DETAIL_VALUE_AT_KEY_IMPL_02042013_0821

#include <lslboost/fusion/support/config.hpp>
#include <lslboost/fusion/support/detail/access.hpp>
#include <lslboost/type_traits/is_const.hpp>
#include <lslboost/mpl/at.hpp>
#include <lslboost/mpl/identity.hpp>
#include <lslboost/utility/declval.hpp>

namespace lslboost { namespace fusion
{
    struct map_tag;

    namespace extension
    {
        template <typename Tag>
        struct value_at_key_impl;

        template <>
        struct value_at_key_impl<map_tag>
        {
            template <typename Sequence, typename Key>
            struct apply
            {
                typedef
                    decltype(lslboost::declval<Sequence>().get_val(mpl::identity<Key>()))
                type;
            };
        };
    }
}}

#endif