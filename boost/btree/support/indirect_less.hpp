//  indirect_less.hpp  -----------------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  This code is experimental and has not been accepted as a boost.org library

#ifndef BOOST_BTREE_INDIRECT_LESS_HPP       
#define BOOST_BTREE_INDIRECT_LESS_HPP

namespace boost
{
  namespace btree
  {

    template <class T> struct indirect_less
    {
      typedef T first_argument_type;
      typedef T second_argument_type;
      typedef bool result_type;
      bool operator()(T* x, T* y) const
      {
        return *x < *y;
      }
    };

  }  // namespace btree
}  // namespace boost

#endif BOOST_BTREE_INDIRECT_LESS_HPP
