//  placement_move.hpp  ----------------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  This code is experimental and has not been accepted as a boost.org library

#ifndef BOOST_DETAIL_PLACEMENT_MOVE_HPP       
#define BOOST_DETAIL_PLACEMENT_MOVE_HPP

#include <cstddef>
#include <iterator>
#include <boost/assert.hpp>

namespace boost
{
namespace detail
{

//------------------------------------ placement_move() --------------------------------//

template<class InputIterator, class OutputIterator>
OutputIterator
placement_move(InputIterator first, InputIterator last, OutputIterator result)

// Requires: result shall not be in the range [first,last).
//
// Effects: Moves elements in the range [first,last) into the range [result,result +
// (last - first)) starting from first and proceeding to last. For each non-negative
// integer n < (last-first), performs:
//
//    typedef typename std::iterator_traits<OutputIterator>::value_type value_type;
//    ::new (result+n) value_type(std::move(*(first + n)));
//    (first + n)->~value_type();
//
// Returns: result + (last - first).
//
// Complexity: Exactly last - first placement move constructions.

{
  typedef typename std::iterator_traits<OutputIterator>::value_type value_type;
  for (; first != last; ++first, ++result)
  {
    ::new (result) value_type(std::move(*first));
    first->~value_type();
  }
  return result;
}
//
////------------------------------- placement_move_backward() ----------------------------//
//
//template<class RandomIterator>
//void
//placement_move(RandomIterator first, RandomIterator last, RandomIterator result)
//
//// Requires: result shall not be in the range [first,last).
////
//// Effects: Moves elements in the range [first,last) into the range [result-(last-first),
//// result) starting from last - 1 and proceeding to first.273 For each positive integer
//// n <= (last - first), performs:
////
////    typedef typename std::iterator_traits<RandomIterator>::value_type value_type;
////    ::new (result-n) value_type(std::move(*(last-n)));
////
//// Complexity: Exactly last - first move assignments.
//
//{
//  typedef typename std::iterator_traits<RandomIterator>::value_type value_type;
//  for (; first != last; ++first, ++result)
//  {
//...    ::new (result) value_type(std::move(*first));
//...    first->~value_type();
//  }
//}

} // namespace detail

} // namespace boost

#endif  // BOOST_DETAIL_PLACEMENT_MOVE_HPP
