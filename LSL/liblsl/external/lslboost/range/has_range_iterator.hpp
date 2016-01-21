// Boost.Range library
//
//  Copyright Neil Groves 2010. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.lslboost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.lslboost.org/libs/range/
//
// Acknowledgments:
// Ticket #8341: Arno Schoedl - improved handling of has_range_iterator upon
// use-cases where T was const.
#ifndef BOOST_RANGE_HAS_ITERATOR_HPP_INCLUDED
#define BOOST_RANGE_HAS_ITERATOR_HPP_INCLUDED

#include <lslboost/mpl/bool.hpp>
#include <lslboost/mpl/eval_if.hpp>
#include <lslboost/mpl/has_xxx.hpp>
#include <lslboost/range/iterator.hpp>
#include <lslboost/type_traits/remove_reference.hpp>
#include <lslboost/utility/enable_if.hpp>

namespace lslboost
{
    namespace range_detail
    {
        BOOST_MPL_HAS_XXX_TRAIT_DEF(type)

        template<class T, class Enabler = void>
        struct has_range_iterator_impl
            : lslboost::mpl::false_
        {
        };

        template<class T>
        struct has_range_iterator_impl<
            T,
            BOOST_DEDUCED_TYPENAME ::lslboost::enable_if<
                BOOST_DEDUCED_TYPENAME mpl::eval_if<is_const<T>,
                    has_type<range_const_iterator<
                                BOOST_DEDUCED_TYPENAME remove_const<T>::type> >,
                    has_type<range_mutable_iterator<T> >
                >::type
            >::type
        >
            : lslboost::mpl::true_
        {
        };

        template<class T, class Enabler = void>
        struct has_range_const_iterator_impl
            : lslboost::mpl::false_
        {
        };

        template<class T>
        struct has_range_const_iterator_impl<
            T,
            BOOST_DEDUCED_TYPENAME ::lslboost::enable_if<
                has_type<range_const_iterator<T> >
            >::type
        >
            : lslboost::mpl::true_
        {
        };

    } // namespace range_detail

    template<class T>
    struct has_range_iterator
        : range_detail::has_range_iterator_impl<
            BOOST_DEDUCED_TYPENAME remove_reference<T>::type>
    {};

    template<class T>
    struct has_range_const_iterator
        : range_detail::has_range_const_iterator_impl<
            BOOST_DEDUCED_TYPENAME remove_reference<T>::type>
    {};
} // namespace lslboost

#endif // include guard
