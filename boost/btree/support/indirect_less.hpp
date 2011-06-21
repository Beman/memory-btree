//  indirect_less.hpp  -----------------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//  This code is experimental and has not been accepted as a boost.org library

#ifndef BOOST_BTREE_INDIRECT_LESS_HPP       
#define BOOST_BTREE_INDIRECT_LESS_HPP

#include <cstring>

namespace boost
{
  namespace btree
  {

    template <class T> struct indirect_less
    {
      typedef T first_argument_type;
      typedef T second_argument_type;
      typedef bool result_type;

      bool operator()(const T& x, const T& y) const
      {
        return *x < *y;
      }
    };

    template <> struct indirect_less<char*>
    {
      typedef char* first_argument_type;
      typedef char* second_argument_type;
      typedef bool result_type;

      bool operator()(const char* x, const char* y) const
      {
        return std::strcmp(x, y) < 0;
      }
    };

  }  // namespace btree
}  // namespace boost

#endif BOOST_BTREE_INDIRECT_LESS_HPP
