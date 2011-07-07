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
          class Allocator = std::allocator<Key> >
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
  typedef std::size_t size_type;
  typedef typename mbt_base<Key,mbt_set_base<Key,Compare>,Compare,Allocator>::iterator
    iterator;
  typedef typename mbt_base<Key,mbt_set_base<Key,Compare>,Compare,Allocator>::value_type
    value_type;

  explicit mbt_set(size_type node_sz = default_node_size,
    const Compare& comp = Compare(), const Allocator& alloc = Allocator())
      : mbt_base<Key,mbt_set_base<Key,Compare>,Compare,Allocator>(node_sz, comp, alloc) {}

   template <class InputIterator>
    mbt_set(InputIterator first, InputIterator last,   // range constructor
            size_type node_sz = default_node_size,
            const Compare& comp = Compare(), const Allocator& alloc = Allocator())
      : mbt_base<Key,mbt_set_base<Key,Compare>,Compare,Allocator>(first, last, node_sz, comp, alloc) {}

  mbt_set(const mbt_set<Key,Compare,Allocator>& x)  // copy constructor
    : mbt_base<Key,mbt_set_base<Key,Compare>,Compare,Allocator>(x) {}

  mbt_set(mbt_set<Key,Compare,Allocator>&& x)       // move constructor
    : mbt_base<Key,mbt_set_base<Key,Compare>,Compare,Allocator>() {this->swap(x);}

  mbt_set<Key,Compare,Allocator>&
  operator=(const mbt_set<Key,Compare,Allocator>& x)  // copy assignment
  {
    mbt_base<Key,mbt_set_base<Key,Compare>,Compare,Allocator>::operator=(x);
    return *this;
  }

  mbt_set<Key,Compare,Allocator>&
  operator=(mbt_set<Key,Compare,Allocator>&& x)     // move assignment
  {
    this->swap(x);
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
//                            class btree_set_common_base                               //
//--------------------------------------------------------------------------------------//

template <class Key, class Compare>
class mbt_set_common_base
{
protected:
  typedef Key           leaf_value;
  class unique{};
  class non_unique{};

public:
  typedef Key               value_type;
  typedef const value_type  iterator_value_type;
  typedef Key               mapped_type;
  typedef Compare           value_compare;

  //  Functions not required by associative container requirements and not supplied by
  //  standard library containers. Primary use cases include generic code such as a
  //  test suite or the implementation itself that wishes to abstract away the
  //  difference between maps and sets.
  static const Key&  key(const value_type& v)          {return v;}
  static const Key&  mapped_value(const value_type& v) {return v;}
  static const Key&  make_value(const Key& k)          {return k;}
  static Key&&       make_value(Key&& k)               {return k;}
  static const Key&  make_value(const Key& k, const mapped_type&) {return k;}
};

//--------------------------------------------------------------------------------------//
//                                class btree_set_base                                  //
//--------------------------------------------------------------------------------------//

template <class Key, class Compare>
class mbt_set_base : public mbt_set_common_base<Key, Compare>
{
protected:
  typedef typename boost::btree::mbt_set_common_base<Key, Compare>::unique
    uniqueness;
};

//--------------------------------------------------------------------------------------//
//                                                                                      //
//                                  class mbt_multiset                                       //
//                                                                                      //
//--------------------------------------------------------------------------------------//

template <class Key, class Compare = std::less<Key>,
          class Allocator = std::allocator<Key> >
  class mbt_multiset;   // short for memory_btree_multiset

template <class Key, class Compare, class Allocator> inline
  bool operator==(const mbt_multiset<Key,Compare,Allocator>& x,
                  const mbt_multiset<Key,Compare,Allocator>& y)
    { return x.size() == y.size()  && std::equal(x.begin(), x.end(), y.begin()); }

template <class Key, class Compare, class Allocator> inline
  bool operator!=(const mbt_multiset<Key,Compare,Allocator>& x,
                  const mbt_multiset<Key,Compare,Allocator>& y)  { return !(x == y); }

template <class Key, class Compare, class Allocator> inline
  bool operator< (const mbt_multiset<Key,Compare,Allocator>& x,
                  const mbt_multiset<Key,Compare,Allocator>& y)
    { return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end()); }

template <class Key, class Compare, class Allocator> inline
  bool operator> (const mbt_multiset<Key,Compare,Allocator>& x,
                  const mbt_multiset<Key,Compare,Allocator>& y)  { return y < x; }

template <class Key, class Compare, class Allocator> inline
  bool operator>=(const mbt_multiset<Key,Compare,Allocator>& x,
                  const mbt_multiset<Key,Compare,Allocator>& y)  { return !(x < y); }

template <class Key, class Compare, class Allocator> inline
  bool operator<=(const mbt_multiset<Key,Compare,Allocator>& x,
                  const mbt_multiset<Key,Compare,Allocator>& y)  { return !(x > y);}

template <class Key, class Compare, class Allocator> inline
  void swap(mbt_multiset<Key,Compare,Allocator>& x,
            mbt_multiset<Key,Compare,Allocator>& y) { x.swap(y); }

template <class Key, class Compare> class mbt_multiset_base;

//--------------------------------------------------------------------------------------//
//                                  class mbt_multiset                                       //
//--------------------------------------------------------------------------------------//

template <class Key, class Compare, class Allocator>
class mbt_multiset   // short for memory_btree_multiset
  : public mbt_base<Key, mbt_multiset_base<Key,Compare>, Compare, Allocator>
{
public:
  typedef std::size_t size_type;
  typedef typename mbt_base<Key,mbt_multiset_base<Key,Compare>,Compare,Allocator>::iterator
    iterator;
  typedef typename mbt_base<Key,mbt_multiset_base<Key,Compare>,Compare,Allocator>::value_type
    value_type;

  explicit mbt_multiset(size_type node_sz = default_node_size,
    const Compare& comp = Compare(), const Allocator& alloc = Allocator())
      : mbt_base<Key,mbt_multiset_base<Key,Compare>,Compare,Allocator>
         (node_sz, comp, alloc) {}


  template <class InputIterator>
    mbt_multiset(InputIterator first, InputIterator last,   // range constructor
            size_type node_sz = default_node_size,
            const Compare& comp = Compare(), const Allocator& alloc = Allocator())
      : mbt_base<Key,mbt_multiset_base<Key,Compare>,Compare,Allocator>
         (first, last, node_sz, comp, alloc) {}

  mbt_multiset(const mbt_multiset<Key,Compare,Allocator>& x)  // copy constructor
    : mbt_base<Key,mbt_multiset_base<Key,Compare>,Compare,Allocator>(x) {}

  mbt_multiset(mbt_multiset<Key,Compare,Allocator>&& x)       // move constructor
    : mbt_base<Key,mbt_multiset_base<Key,Compare>,Compare,Allocator>() {this->swap(x);}

  mbt_multiset<Key,Compare,Allocator>&
  operator=(const mbt_multiset<Key,Compare,Allocator>& x)     // copy assignment
  {
    mbt_base<Key,mbt_multiset_base<Key,Compare>,Compare,Allocator>::operator=(x);
    return *this;
  }

  mbt_multiset<Key,Compare,Allocator>&
  operator=(mbt_multiset<Key,Compare,Allocator>&& x)          // move assignment
  {
    this->swap(x);
    return *this;
  }

  iterator  insert(const value_type& x)
    { return m_insert_non_unique(x); }

  iterator  insert(value_type&& x)
    { return m_insert_non_unique(x); }

  template <class InputIterator>
    void insert(InputIterator first, InputIterator last)
  {
    for (; first != last; ++first)
      m_insert_non_unique(*first);
  }
};


//--------------------------------------------------------------------------------------//
//                             class btree_multiset_base                                //
//--------------------------------------------------------------------------------------//

template <class Key, class Compare>
class mbt_multiset_base : public mbt_set_common_base<Key, Compare>
{
protected:
  typedef typename boost::btree::mbt_set_common_base<Key, Compare>::non_unique
    uniqueness;
};

}  // namespace btree
}  // namespace boost

#endif  // BOOST_MBT_SET_HPP
