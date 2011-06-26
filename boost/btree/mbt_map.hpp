//  mbt_map.hpp  -----------------------------------------------------------------------//

//  Copyright Beman Dawes 2010, 2011

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  This library is experimental and has not been accepted as a boost.org library

#ifndef BOOST_MBT_MAP_HPP
#define BOOST_MBT_MAP_HPP

#include <boost/btree/detail/mbt_base.hpp>

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

template <class Key, class T, class Compare> class mbt_map_base;

//--------------------------------------------------------------------------------------//
//                                  class mbt_map                                       //
//--------------------------------------------------------------------------------------//

template <class Key, class T, class Compare, class Allocator>
class mbt_map   // short for memory_btree_map
  : public mbt_base<Key, mbt_map_base<Key,T,Compare>, Compare, Allocator>
{
public:

  explicit mbt_map(size_type node_sz = default_node_size,
    const Compare& comp = Compare(), const Allocator& alloc = Allocator())
      : mbt_base(node_sz, comp, alloc) {}


  template <class InputIterator>
    mbt_map(InputIterator first, InputIterator last,   // range constructor
            size_type node_sz = default_node_size,
            const Compare& comp = Compare(), const Allocator& alloc = Allocator())
      : mbt_base(first, last, node_sz, comp, alloc) {}

  mbt_map(const mbt_map<Key,T,Compare,Allocator>& x)  // copy constructor
    : mbt_base(x) {}

  mbt_map(mbt_map<Key,T,Compare,Allocator>&& x)       // move constructor
    : mbt_base() {swap(x);}

  mbt_map<Key,T,Compare,Allocator>&
  operator=(mbt_map<Key,T,Compare,Allocator>&& x)     // move assignment
  {
    swap(x);
    return *this;
  }

  T&        operator[](const Key& x) {return m_op_square_brackets(x);}
  T&        operator[](Key&& x)      {return m_op_square_brackets(x);}
//  T&        at(const Key& x);
//  const T&  at(const Key& x) const;

  std::pair<iterator,bool>
    insert(const value_type& x)  { return m_insert_unique(x); }

  template <class P>
  std::pair<iterator,bool>
    insert(P&& x)
      { return m_insert_unique(std::forward<value_type>(x)); }

  template <class InputIterator>
    void insert(InputIterator first, InputIterator last)
  {
    for (; first != last; ++first)
      m_insert_unique(*first);
  }

};

//--------------------------------------------------------------------------------------//
//                            class mbt_map_common_base                                 //
//--------------------------------------------------------------------------------------//

template <class Key, class T, class Compare>
class mbt_map_common_base
{
protected:
  class unique{};
  class non_unique{};
  typedef std::pair<Key, T>        leaf_value;


public:
  typedef T                        mapped_type;
  typedef std::pair<const Key, T>  value_type;
  typedef value_type               iterator_value_type;

  class value_compare
  {
  protected:
    Compare m_comp;
  public:
    typedef bool                   result_type;
    typedef value_type             first_argument_type;
    typedef value_type             second_argument_type;

    value_compare(Compare c) : m_comp(c) {}
    bool operator()(const value_type& x, const value_type& y) const
      { return m_comp(x.first, y.first); }

    bool operator()(const value_type& x, const Key& y) const
      { return m_comp(x.first, y); }

    bool operator()(const Key& x, const value_type& y) const
      { return m_comp(x, y.first); }
  };

  //  Functions not required by associative container requirements and not supplied by
  //  standard library containers. Primary use cases include generic code such as a 
  //  test suite or the implementation itself that wishes to abstract away the
  //  difference between maps and sets.
  static const Key& key(const value_type& v)          {return v.first;}
  static const T&   mapped_value(const value_type& v) {return v.second;}
  static value_type make_value(const Key& k)          {return value_type(k, mapped_type());}
  static value_type make_value(const Key& k, const mapped_type& v) {return value_type(k, v);}
};

//--------------------------------------------------------------------------------------//
//                                class mbt_map_base                                    //
//--------------------------------------------------------------------------------------//

template <class Key, class T, class Compare>
class mbt_map_base : public mbt_map_common_base<Key, T, Compare>
{
protected:
  typedef unique                   uniqueness;
};

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                               class mbt_multimap                                     //
//                                                                                      //
//--------------------------------------------------------------------------------------//

template <class Key, class T, class Compare = std::less<Key>,
          class Allocator = std::allocator<std::pair<const Key, T> > >
  class mbt_multimap;   // short for memory_btree_multimap

template <class Key, class T, class Compare, class Allocator> inline
  bool operator==(const mbt_multimap<Key,T,Compare,Allocator>& x,
                  const mbt_multimap<Key,T,Compare,Allocator>& y)
    { return x.size() == y.size()  && std::equal(x.begin(), x.end(), y.begin()); }

template <class Key, class T, class Compare, class Allocator> inline
  bool operator!=(const mbt_multimap<Key,T,Compare,Allocator>& x,
                  const mbt_multimap<Key,T,Compare,Allocator>& y)  { return !(x == y); }

template <class Key, class T, class Compare, class Allocator> inline
  bool operator< (const mbt_multimap<Key,T,Compare,Allocator>& x,
                  const mbt_multimap<Key,T,Compare,Allocator>& y)
    { return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end()); }

template <class Key, class T, class Compare, class Allocator> inline
  bool operator> (const mbt_multimap<Key,T,Compare,Allocator>& x,
                  const mbt_multimap<Key,T,Compare,Allocator>& y)  { return y < x; }

template <class Key, class T, class Compare, class Allocator> inline
  bool operator>=(const mbt_multimap<Key,T,Compare,Allocator>& x,
                  const mbt_multimap<Key,T,Compare,Allocator>& y)  { return !(x < y); }

template <class Key, class T, class Compare, class Allocator> inline
  bool operator<=(const mbt_multimap<Key,T,Compare,Allocator>& x,
                  const mbt_multimap<Key,T,Compare,Allocator>& y)  { return !(x > y);}

template <class Key, class T, class Compare, class Allocator> inline
  void swap(mbt_multimap<Key,T,Compare,Allocator>& x, mbt_multimap<Key,T,Compare,Allocator>& y)
    { x.swap(y); }

template <class Key, class T, class Compare> class mbt_multimap_base;

//--------------------------------------------------------------------------------------//
//                               class mbt_multimap                                     //
//--------------------------------------------------------------------------------------//

template <class Key, class T, class Compare, class Allocator>
class mbt_multimap   // short for memory_btree_multimap
  : public mbt_base<Key, mbt_multimap_base<Key,T,Compare>, Compare, Allocator>
{
public:

  explicit mbt_multimap(size_type node_sz = default_node_size,
    const Compare& comp = Compare(), const Allocator& alloc = Allocator())
      : mbt_base(node_sz, comp, alloc) {}


  template <class InputIterator>
    mbt_multimap(InputIterator first, InputIterator last,   // range constructor
            size_type node_sz = default_node_size,
            const Compare& comp = Compare(), const Allocator& alloc = Allocator())
      : mbt_base(first, last, node_sz, comp, alloc) {}

  mbt_multimap(const mbt_multimap<Key,T,Compare,Allocator>& x)  // copy constructor
    : mbt_base(x) {}

  mbt_multimap(mbt_multimap<Key,T,Compare,Allocator>&& x)       // move constructor
    : mbt_base() {swap(x);}

  mbt_multimap<Key,T,Compare,Allocator>&
  operator=(mbt_multimap<Key,T,Compare,Allocator>&& x)     // move assignment
  {
    swap(x);
    return *this;
  }

  iterator  insert(const value_type& x)         { return m_insert_non_unique(x); }

  template <class P>
  iterator  insert(P&& x)                       { return m_insert_non_unique(x); }

  template <class InputIterator>
    void insert(InputIterator first, InputIterator last)
  {
    for (; first != last; ++first)
      m_insert_non_unique(*first);
  }

};

//--------------------------------------------------------------------------------------//
//                             class mbt_multimap_base                                  //
//--------------------------------------------------------------------------------------//

template <class Key, class T, class Compare>
class mbt_multimap_base : public mbt_map_common_base<Key, T, Compare>
{
protected:
  typedef non_unique               uniqueness;
};

}  // namespace btree
}  // namespace boost

#endif  // BOOST_MBT_MAP_HPP
