/* 
   Copyright (c) Marshall Clow 2011-2012.

   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.lslboost.org/LICENSE_1_0.txt)
*/

/// \file  partition_point.hpp
/// \brief Find the partition point in a sequence
/// \author Marshall Clow

#ifndef BOOST_ALGORITHM_PARTITION_POINT_HPP
#define BOOST_ALGORITHM_PARTITION_POINT_HPP

#include <algorithm>    // for std::partition_point, if available

#include <lslboost/range/begin.hpp>
#include <lslboost/range/end.hpp>

namespace lslboost { namespace algorithm {

/// \fn partition_point ( ForwardIterator first, ForwardIterator last, Predicate p )
/// \brief Given a partitioned range, returns the partition point, i.e, the first element 
///     that does not satisfy p
/// 
/// \param first    The start of the input sequence
/// \param last     One past the end of the input sequence
/// \param p        The predicate to test the values with
/// \note           This function is part of the C++2011 standard library.
///  We will use the standard one if it is available, 
///  otherwise we have our own implementation.
template <typename ForwardIterator, typename Predicate>
ForwardIterator partition_point ( ForwardIterator first, ForwardIterator last, Predicate p )
{
    std::size_t dist = std::distance ( first, last );
    while ( first != last ) {
        std::size_t d2 = dist / 2;
        ForwardIterator ret_val = first;
        std::advance (ret_val, d2);
        if (p (*ret_val)) {
            first = ++ret_val;
            dist -= d2 + 1;
            }
        else {
            last = ret_val;
            dist = d2;
            }
        }
    return first;
}

/// \fn partition_point ( Range &r, Predicate p )
/// \brief Given a partitioned range, returns the partition point
/// 
/// \param r        The input range
/// \param p        The predicate to test the values with
///
template <typename Range, typename Predicate>
typename lslboost::range_iterator<Range>::type partition_point ( Range &r, Predicate p )
{
    return lslboost::algorithm::partition_point (lslboost::begin(r), lslboost::end(r), p);
}


}}

#endif  // BOOST_ALGORITHM_PARTITION_POINT_HPP