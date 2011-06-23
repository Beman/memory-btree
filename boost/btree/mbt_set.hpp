//  mbt_set.hpp  -----------------------------------------------------------------------//

//  Copyright Beman Dawes 2010, 2011

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  This library is experimental and has not been accepted as a boost.org library

#ifndef BOOST_MBT_SET_HPP
#define BOOST_MBT_SET_HPP

#include <boost/btree/detail/mbt_base.hpp>

namespace boost {
namespace btree {

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                  class mbt_set                                       //
//                                                                                      //
//  "mbt_set" is a placeholder name for an in-memory B+tree set class that is similar   //
//  to std::set. The primary difference is that all insert, emplace, and erase          //
//  operations may (and usually do) invalidate iterators.                               //                     //
//                                                                                      //
//--------------------------------------------------------------------------------------//

template <class Key, class Compare = std::less<Key>,
          class Allocator = std::allocator<const Key> >
  class mbt_set;   // short for memory_btree_set

template <class Key, class Compare, class Allocator> inline
  bool operator==(const mbt_set<Key,Compare,Allocator>& x,
                  const mbt_set<Key,Compare,Allocator>& y)
    { return x.size() == y.size()  && std::equal(x.begin(), x.end(), y.begin()); }

template <class Key, class Compare, class Allocator> inline
  bool operator!=(const mbt_set<Key,Compare,Allocator>& x,
                  const mbt_set<Key,Compare,Allocator>& y)  { return !(x == y); }

template <class Key, class Compare, class Allocator> inline
  bool operator< (const mbt_set<Key,Compare,Allocator>& x,
                  const mbt_set<Key,Compare,Allocator>& y)
    { return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end()); }

template <class Key, class Compare, class Allocator> inline
  bool operator> (const mbt_set<Key,Compare,Allocator>& x,
                  const mbt_set<Key,Compare,Allocator>& y)  { return y < x; }

template <class Key, class Compare, class Allocator> inline
  bool operator>=(const mbt_set<Key,Compare,Allocator>& x,
                  const mbt_set<Key,Compare,Allocator>& y)  { return !(x < y); }

template <class Key, class Compare, class Allocator> inline
  bool operator<=(const mbt_set<Key,Compare,Allocator>& x,
                  const mbt_set<Key,Compare,Allocator>& y)  { return !(x > y);}

template <class Key, class Compare, class Allocator> inline
  void swap(mbt_set<Key,Compare,Allocator>& x, mbt_set<Key,Compare,Allocator>& y)
    { x.swap(y); }

template <class Key, class Compare> class mbt_set_base;

//--------------------------------------------------------------------------------------//
//                                  class mbt_set                                       //
//--------------------------------------------------------------------------------------//

template <class Key, class Compare, class Allocator>
class mbt_set   // short for memory_btree_set
  : public mbt_base<Key, mbt_set_base<Key,Compare>, Compare, Allocator>
{
public:

  explicit mbt_set(size_type node_sz = default_node_size,
    const Compare& comp = Compare(), const Allocator& alloc = Allocator())
      : mbt_base(node_sz, comp, alloc) {}


  template <class InputIterator>
    mbt_set(InputIterator first, InputIterator last,   // range constructor
            size_type node_sz = default_node_size,
            const Compare& comp = Compare(), const Allocator& alloc = Allocator())
      : mbt_base(first, last, node_sz, comp, alloc) {}

  mbt_set(const mbt_set<Key,Compare,Allocator>& x)  // copy constructor
    : mbt_base(x) {}

  mbt_set(mbt_set<Key,Compare,Allocator>&& x)       // move constructor
    : mbt_base() {swap(x);}

  mbt_set<Key,Compare,Allocator>&
  operator=(mbt_set<Key,Compare,Allocator>&& x)     // move assignment
  {
    swap(x);
    return *this;
  }

  std::pair<iterator,bool>  insert(const value_type& x)
    { return m_insert_unique(x); }

  std::pair<iterator,bool>  insert(value_type&& x)
    { return m_insert_unique(x); }

  template <class InputIterator>
    void insert(InputIterator first, InputIterator last)
  {
    for (; first != last; ++first)
      m_insert_unique(*first);
  }
};

//--------------------------------------------------------------------------------------//
//                                class btree_set_base                                  //
//--------------------------------------------------------------------------------------//

template <class Key, class Compare>
class mbt_set_base
{
protected:
  typedef Key           leaf_value;
  class unique{};
  class non_unique{};
  typedef unique        uniqueness;

public:
  typedef Key           value_type;
  typedef Key           mapped_type;
  typedef Compare       value_compare;

  const Key& key(const value_type& v) const {return v;}  // really handy, so expose
  const Key& mapped_value(const value_type& v) const {return v;} 
  value_type make_value(const Key& k) const {return k;}
  value_type&& make_value(Key&& k) const {return k;}
};

}  // namespace btree
}  // namespace boost

#endif  // BOOST_MBT_SET_HPP
