//  mbt_map.hpp  -----------------------------------------------------------------------//

//  Copyright Beman Dawes 2010, 2011

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  This library is experimental and has not been accepted as a boost.org library

#ifndef BOOST_MBT_MAP_HPP
#define BOOST_MBT_MAP_HPP

#define BOOST_NOEXCEPT

#include <boost/config/warning_disable.hpp>
#include <boost/config.hpp>

namespace boost {
namespace btree {

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                  class mbt_map                                       //
//                                                                                      //
//  "mbt_map" is a placeholder name for an in-memory B+tree map class that is similar   //
//  to std::map. The primary difference is that all insert, emplace, and erase          //
//  operations may (and usually do) invalidate iterators.                               //                     //
//                                                                                      //
//--------------------------------------------------------------------------------------//

template <class Key, class T, class Compare = std::less<Key>,
          class Allocator = std::allocator<std::pair<const Key, T> > >
  class mbt_map;   // short for memory_btree_map

template <class Key, class T, class Compare, class Allocator> inline
  bool operator==(const mbt_map<Key,T,Compare,Allocator>& x,
                  const mbt_map<Key,T,Compare,Allocator>& y)
    { return x.size() == y.size()  && std::equal(x.begin(), x.end(), y.begin()); }

template <class Key, class T, class Compare, class Allocator> inline
  bool operator!=(const mbt_map<Key,T,Compare,Allocator>& x,
                  const mbt_map<Key,T,Compare,Allocator>& y)  { return !(x == y); }

template <class Key, class T, class Compare, class Allocator> inline
  bool operator< (const mbt_map<Key,T,Compare,Allocator>& x,
                  const mbt_map<Key,T,Compare,Allocator>& y)
    { return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end()); }

template <class Key, class T, class Compare, class Allocator> inline
  bool operator> (const mbt_map<Key,T,Compare,Allocator>& x,
                  const mbt_map<Key,T,Compare,Allocator>& y)  { return y < x; }

template <class Key, class T, class Compare, class Allocator> inline
  bool operator>=(const mbt_map<Key,T,Compare,Allocator>& x,
                  const mbt_map<Key,T,Compare,Allocator>& y)  { return !(x < y); }

template <class Key, class T, class Compare, class Allocator> inline
  bool operator<=(const mbt_map<Key,T,Compare,Allocator>& x,
                  const mbt_map<Key,T,Compare,Allocator>& y)  { return !(x > y);}

template <class Key, class T, class Compare, class Allocator> inline
  void swap(mbt_map<Key,T,Compare,Allocator>& x, mbt_map<Key,T,Compare,Allocator>& y)
    { x.swap(y); }

//--------------------------------------------------------------------------------------//
//                                  class mbt_map                                       //
//--------------------------------------------------------------------------------------//

template <class Key, class T, class Compare, class Allocator>
class mbt_map   // short for memory_btree_map
  : public mbt_base<Key, mbt_map_base<Key,Compare>, Compare, Allocator>
{

  explicit mbt_map(size_type node_sz = default_node_size,
    const Compare& comp = Compare(), const Allocator& alloc = Allocator());

  template <class InputIterator>
    mbt_map(InputIterator first, InputIterator last,   // range constructor
            size_type node_sz = default_node_size,
            const Compare& comp = Compare(), const Allocator& = Allocator());

  mbt_map(const mbt_map<Key,T,Compare,Allocator>& x);  // copy constructor

  mbt_map(mbt_map<Key,T,Compare,Allocator>&& x);       // move constructor

  std::pair<iterator,bool> insert(const value_type& x);

  std::pair<iterator,bool> insert(value_type&& x);

  template <class InputIterator>
  void insert(InputIterator first, InputIterator last);

};

}  // namespace btree
}  // namespace boost

#endif  // BOOST_MBT_MAP_HPP
