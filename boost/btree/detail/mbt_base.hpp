//  mbt_base.hpp  ----------------------------------------------------------------------//

//  Copyright Beman Dawes 2010, 2011

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  This library is experimental and has not been accepted as a boost.org library

#ifndef BOOST_MBT_MAP_BASE_HPP
#define BOOST_MBT_MAP_BASE_HPP

#define BOOST_NOEXCEPT

#include <boost/config/warning_disable.hpp>
#include <boost/config.hpp>

#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable: 4996)  // ... Function call with parameters that may be unsafe
#  pragma warning(disable: 4200)  // nonstandard extension used : zero-sized array in struct/union
#endif

#include <cstddef>
#include <functional>
#include <utility>
#include <memory>
#include <new>
#include <iterator>
#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <boost/cstdint.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/assert.hpp>
#include <boost/btree/detail/placement_move.hpp>
#include <cstring> // for memset


/*
TODO:

  * r-value insert not being tested

  * erase: doesn't m_erase_from_parent invalidate nxt_it? Fix or document.

  * uniqueness s/b renamed unique and be a typedef for true_type or false_type
  
  * Tighten requirements on Key and T to match standard library.
    Change archetype accordingly.

  * Should use allocators, allocator_traits!

  * Review exception safety.

*/

namespace boost {
namespace btree {

//--------------------------------------------------------------------------------------//
//                                  class mbt_base                                      //
//--------------------------------------------------------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
class mbt_base : public Base
{
  class node;
  class leaf_node;
  class branch_node;
  template <class VT>
    class iterator_type;
public:

  // types:
  typedef Key                                     key_type;
  typedef typename Base::value_type               value_type;
  typedef typename Base::mapped_type              mapped_type;
  typedef Compare                                 key_compare;
  typedef typename Base::value_compare            value_compare; 
  typedef Allocator                               allocator_type;
  typedef value_type&                             reference;
  typedef const value_type&                       const_reference;
  typedef iterator_type<value_type>               iterator;        // see 23.2
  typedef iterator_type<const value_type>         const_iterator;  // see 23.2
  typedef std::size_t                             size_type;       // see 23.2
  typedef std::ptrdiff_t                          difference_type; // see 23.2
//    typedef
//      typename std::allocator_traits<Allocator>::pointer pointer;
//    typedef
//      typename std::allocator_traits<Allocator>::const_pointer
//                                                    const_pointer;
  typedef std::reverse_iterator<iterator>         reverse_iterator;
  typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

  static const size_type                          default_node_size = 2048;

  // 23.4.4.2, construct/copy/destroy:

  explicit mbt_base(size_type node_sz = default_node_size,
    const Compare& comp = Compare(), const Allocator& alloc = Allocator());

  template <class InputIterator>
    mbt_base(InputIterator first, InputIterator last,   // range constructor
            size_type node_sz = default_node_size,
            const Compare& comp = Compare(), const Allocator& = Allocator());

  mbt_base(const mbt_base<Key,Base,Compare,Allocator>& x);  // copy constructor

  mbt_base(mbt_base<Key,Base,Compare,Allocator>&& x);       // move constructor

//  explicit mbt_base(const Allocator&);
//  mbt_base(const mbt_base&, const Allocator&);
//  mbt_base(mbt_base&&, const Allocator&);
//  mbt_base(initializer_list<value_type>, const Compare& = Compare(),
//    const Allocator& = Allocator());

  ~mbt_base()  {m_free_all(m_root);}

  mbt_base<Key,Base,Compare,Allocator>&
    operator=(const mbt_base<Key,Base,Compare,Allocator>& x);  // copy assignment

  mbt_base<Key,Base,Compare,Allocator>&
    operator=(mbt_base<Key,Base,Compare,Allocator>&& x);       // move assignment

//  mbt_base& operator=(initializer_list<value_type>);

  // iterators:
  iterator                begin() BOOST_NOEXCEPT           {return m_begin();}
  const_iterator          begin() const BOOST_NOEXCEPT     {return const_cast<mbt_base*>(this)->m_begin();}
  iterator                end() BOOST_NOEXCEPT             {return iterator(this);}
  const_iterator          end() const BOOST_NOEXCEPT       {return const_iterator(this);}
  reverse_iterator        rbegin() BOOST_NOEXCEPT;
  const_reverse_iterator  rbegin() const BOOST_NOEXCEPT;
  reverse_iterator        rend() BOOST_NOEXCEPT;
  const_reverse_iterator  rend() const BOOST_NOEXCEPT;
  const_iterator          cbegin() const BOOST_NOEXCEPT    {return m_begin();}
  const_iterator          cend() const BOOST_NOEXCEPT      {return const_iterator(this);}
  const_reverse_iterator  crbegin() const BOOST_NOEXCEPT;
  const_reverse_iterator  crend() const BOOST_NOEXCEPT;

  // capacity:
  bool                    empty() const BOOST_NOEXCEPT     {return m_size == 0;}
  size_type               size() const BOOST_NOEXCEPT      {return m_size;}
  size_type               max_size() const BOOST_NOEXCEPT;

  // 23.4.4.4, modifiers:
  //template <class... Args>
  //  std::pair<iterator, bool>
  //                        emplace(Args&&... args);
  //template <class... Args>
  //  iterator              emplace_hint(const_iterator position, Args&&... args);

//    template <class P>
//      std::pair<iterator, bool>
//                            insert(P&& x);
//    iterator                insert(const_iterator position, const value_type& x);
//    template <class P>
//      iterator              insert(const_iterator position, P&&);
//    void                    insert(initializer_list<value_type>);

  iterator                erase(const_iterator position);
  size_type               erase(const key_type& x);
  iterator                erase(const_iterator first, const_iterator last);
  void                    swap(mbt_base<Key,Base,Compare,Allocator>&x);
  void                    clear() BOOST_NOEXCEPT;

  // observers:
  key_compare             key_comp() const   {return m_key_compare;}
  value_compare           value_comp() const {return m_value_compare;}
  allocator_type          get_allocator() const BOOST_NOEXCEPT {return m_alloc;}
  size_type               node_size() const BOOST_NOEXCEPT {return m_node_size;}
  int                     height() const     {return m_root->height();}  // aids testing, tuning
  void                    dump_dot(std::ostream& os) const;

  // 23.4.4.5, map operations:
  iterator                find(const key_type& x);
  const_iterator          find(const key_type& x) const {return const_cast<mbt_base*>(this)->find(x);}
  size_type               count(const key_type& x) const;
  iterator                lower_bound(const key_type& x);
  const_iterator          lower_bound(const key_type& x) const {return const_cast<mbt_base*>(this)->lower_bound(x);}
  iterator                upper_bound(const key_type& x);
  const_iterator          upper_bound(const key_type& x) const {return const_cast<mbt_base*>(this)->upper_bound(x);}
  std::pair<iterator, iterator>
                          equal_range(const key_type& x) {return std::make_pair(lower_bound(x), upper_bound(x));}
  std::pair<const_iterator, const_iterator>
                          equal_range(const key_type& x) const {return std::make_pair(lower_bound(x), upper_bound(x));}

private:

  friend class node;
  typedef typename Base::unique      unique;
  typedef typename Base::non_unique  non_unique;
  typedef typename Base::uniqueness  uniqueness;
  typedef typename Base::leaf_value  leaf_value;
  typedef std::pair<node*, Key>      branch_value;  // first is pointer to child node

  //----------------------------------------------------------------------------------//
  //                             private nested classes                               //
  //----------------------------------------------------------------------------------//

  //-------------------------  class branch_value_compare  ---------------------------//

  class branch_value_compare
  {
    friend class mbt_base;
  protected:
    Compare comp;
    branch_value_compare(Compare c) : comp(c) {}
  public:
    typedef bool                                  result_type;
    typedef value_type                            first_argument_type;
    typedef value_type                            second_argument_type;
    bool operator()(const branch_value& x, const branch_value& y) const
    {
      return comp(x.second, y.second);
    }
    bool operator()(const Key& k, const branch_value& y) const
    {
      return comp(k, y.second);
    }
    bool operator()(const branch_value& x, const Key& k) const
    {
      return comp(x.second, k);
    }
  };

  //--------------------------------  class node  ------------------------------------//

  class node
  {
    friend class mbt_base;
  public:
    uint16_t        _height;          // 0 for a leaf node
    uint16_t        _size;
    branch_node*    _parent_node;     // 0 for the root node
    union
    {
      branch_value* _parent_element;  // non-root node
      mbt_base*      _owner;           // root node
    };

    uint16_t      height() const                  {return _height;}
    bool          is_leaf() const                 {return _height == 0;}
    bool          is_branch() const               {return _height != 0;}
    bool          is_root() const                 {return _parent_node == 0;}
    bool          is_empty() const                {return _size == 0;}
    std::size_t   size() const                    {return _size;}
    branch_node*  parent_node() const             {return _parent_node;}
    branch_value* parent_element() const          {return _parent_element;}
    mbt_base*      owner() const                   {return _owner;}

    void          height(uint16_t h)              {_height = h;}
    void          size(std::size_t n)             {_size = n;}
    void          parent_node(branch_node* p)     {_parent_node = p;}
    void          parent_element(branch_value* p) {_parent_element = p;}
    void          owner(mbt_base* o)               {_owner = o;}

    // GCC 4.5.2 only worked on these functions when the type was deduced - thus the
    // unused function argument
    template <class Node>
    Node* next_node(Node*);  // returns next node at same height; root node if end

    template <class Node>
    Node* prior_node(Node*);  // returns prior node at same height; root node if end
  };

  //------------------------------  class leaf_node  ---------------------------------//

  class leaf_node : public node
  {
  public:
    typedef typename mbt_base::leaf_value   value_type;
    typedef typename mbt_base::mapped_type  mapped_type;

    leaf_value     _leaf_values[];                // actual size determined at runtime

    leaf_value*    begin()                        {return _leaf_values;}
    leaf_value*    end()                          {return _leaf_values + node::_size;}

    static
      std::size_t  extra_space()                  {return 0;}
    };

  //-----------------------------  class branch_node  --------------------------------//

  class branch_node : public node
  {
  public:
    typedef typename mbt_base::branch_value  value_type;
    typedef node*                           mapped_type;

    branch_value   _branch_values[];              // actual size determined at runtime

    branch_value*  begin()                        {return _branch_values;}
    branch_value*  end()                          {return _branch_values + node::_size;}
    // pseudo-element end()->first is valid; b-tree branches have size() + 1
    // child pointers - see your favorite computer science textbook.

    static
      std::size_t  extra_space()                  {return sizeof(node*);}
  };

  //----------------------------------------------------------------------------------//
  //                                  iterator_type                                   //
  //----------------------------------------------------------------------------------//

  template <class VT>
  class iterator_type
    : public boost::iterator_facade<iterator_type<VT>, VT, bidirectional_traversal_tag>
  {

  public:
    iterator_type()
#     ifdef NDEBUG
      {}
#     else
      : m_node(0), m_element(0) {}
#     endif

    iterator_type(typename mbt_base::leaf_node* np,
                  typename mbt_base::leaf_value* ep)
      : m_node(np), m_element(ep) {}

    template <class VU>
    iterator_type(iterator_type<VU> const& other)
      : m_node(other.m_node), m_element(other.m_element) {}

  private:
    iterator_type(mbt_base* owner)  // construct end iterator
      : m_node(0), m_owner(owner) {}
    iterator_type(const mbt_base* owner)  // construct end iterator
      : m_node(0), m_owner(const_cast<mbt_base*>(owner)) {}

    friend class boost::iterator_core_access;
    friend class mbt_base;

    typename mbt_base::leaf_node*    m_node;      // 0 indicates end iterator

    union  // discriminated by m_node
    {
      typename mbt_base::leaf_value*  m_element;  // not end iterator
      mbt_base*                       m_owner;    // end iterator
    };

    VT& dereference() const
    {
      BOOST_ASSERT_MSG(m_element, "attempt to dereference uninitialized iterator");
      BOOST_ASSERT_MSG(m_node, "attempt to dereference end iterator");

      return *reinterpret_cast<VT*>(m_element);
    }

    template <class VU>
    bool equal(const iterator_type<VU>& rhs) const {return m_element == rhs.m_element;}

    void increment();
    void decrement();
  };

  //----------------------------------------------------------------------------------//
  //                              private data members                                //
  //----------------------------------------------------------------------------------//

  size_type             m_size;             // number of elements in container
  size_type             m_max_leaf_size;    // maximum number of elements
  size_type             m_max_branch_size;  // maximum number of elements
  node*                 m_root;             // invariant: there is always a root
  size_type             m_node_size;
  key_compare           m_key_compare;
  value_compare         m_value_compare;
  branch_value_compare  m_branch_value_compare;
  allocator_type        m_alloc;

  //----------------------------------------------------------------------------------//
  //                          protected member functions                              //
  //----------------------------------------------------------------------------------//

protected:

  iterator               m_begin() BOOST_NOEXCEPT;

  branch_value_compare   branch_comp() const {return m_branch_value_compare;}

  void      m_init();
  void      m_free_all(node* np);
  void      m_new_root();
  iterator  m_special_lower_bound(const key_type& k) const;
  iterator  m_special_upper_bound(const key_type& k) const;
  iterator  m_last();
  void      m_erase_from_parent(node* child);
  void      m_dump_node(std::ostream& os, node* np) const;

  std::pair<iterator, bool>
            m_insert_unique(const value_type& x);
  std::pair<iterator, bool>
            m_insert_unique(value_type&& x);

  iterator  m_insert_non_unique(const value_type& x);

  template <class P>
  iterator  m_insert_non_unique(P&& x);

  void      m_leaf_insert(value_type&& v, leaf_node*& np, leaf_value*& ep);
  // Remarks:  np points to the node where insertion is to occur
  //           ep points to the element where insertion is to occur
  // Effects:  Inserts v at *ep. If the insertion causes a node to be split,
  //           and the ep falls on the newly split node, np and ep are set to point to
  //           the new node and appropriate element

  void      m_branch_insert(key_type&& k, node* old_np, node* new_np);
  // Effects:  inserts k and new_np at old_np->parent_element()->second and
  //           (old_np->parent_element()+1)->first, respectively
  // Postcondition: For the nodes pointed to by old_np and new_np, parent_node() and
  //           parent_element() are valid. i.e. updated if needed

  template <class Node>
  Node*     m_new_node(uint16_t height_, size_type max_elements);

  template <class Node>
  void      m_free_node(Node* np);

  template <class Node>
  static Node* node_cast(node* np) {return reinterpret_cast<Node*>(np);}

  void      m_insert(const value_type& x, unique)  { m_insert_unique(x); }
  //void      m_insert(const value_type& x, non_unique) { m_insert_non_unique(x); }

  //  mbt_map is the only derived class that provides a public interface to these:
  mapped_type&        m_op_square_brackets(const Key& x);
  mapped_type&        m_op_square_brackets(Key&& x);
  //mapped_type&        at(const Key& x);
  //const mapped_type&  at(const Key& x) const;

};  // class mbt_base

//--------------------------------------------------------------------------------------//
//                                  implementation                                      //
//--------------------------------------------------------------------------------------//

//------------------------  default and general constructor  ---------------------------//

template <class Key, class Base, class Compare, class Allocator>
mbt_base<Key,Base,Compare,Allocator>::
mbt_base(size_type node_sz, const Compare& comp, const Allocator& alloc)
    : m_node_size(node_sz), m_key_compare(comp), m_value_compare(comp),
      m_branch_value_compare(comp), m_alloc(alloc)
{
  m_init();
 }

//------------------------------  range constructor  -----------------------------------//

template <class Key, class Base, class Compare, class Allocator>
template <class InputIterator>
mbt_base<Key,Base,Compare,Allocator>::
mbt_base(InputIterator first, InputIterator last,
        size_type node_sz, const Compare& comp, const Allocator& alloc)
    : m_node_size(node_sz), m_key_compare(comp), m_value_compare(comp),
      m_branch_value_compare(comp), m_alloc(alloc)
{
  m_init();

  for (; first != last; ++first)
    m_insert(*first, uniqueness());
}

//-------------------------------  copy constructor  -----------------------------------//

template <class Key, class Base, class Compare, class Allocator>
mbt_base<Key,Base,Compare,Allocator>::
mbt_base(const mbt_base<Key,Base,Compare,Allocator>& x)
  : m_node_size(x.node_size()), m_key_compare(x.key_comp()),
    m_value_compare(x.key_comp()), m_branch_value_compare(x.key_comp()),
    m_alloc(x.get_allocator())
{
  m_init();

  for (const_iterator it = x.begin(); it != x.end(); ++it)
    m_insert(*it, uniqueness());
}

//-------------------------------  move constructor  -----------------------------------//

template <class Key, class Base, class Compare, class Allocator>
mbt_base<Key,Base,Compare,Allocator>::
mbt_base(mbt_base<Key,Base,Compare,Allocator>&& x)
  : m_node_size(x.node_size()), m_key_compare(x.key_comp()),
    m_value_compare(x.key_comp()), m_branch_value_compare(x.key_comp()),
    m_alloc(x.get_allocator())
{
  m_init();
  swap(x);
}

//-------------------------------  copy assignment  ------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
mbt_base<Key,Base,Compare,Allocator>&
mbt_base<Key,Base,Compare,Allocator>::
operator=(const mbt_base<Key,Base,Compare,Allocator>& x)
{
  clear();

  for (const_iterator it = x.begin(); it != x.end(); ++it)
    m_insert(*it, uniqueness());

  return *this;
}

//-------------------------------  move assignment  ------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
mbt_base<Key,Base,Compare,Allocator>&
mbt_base<Key,Base,Compare,Allocator>::
operator=(mbt_base<Key,Base,Compare,Allocator>&& x)
{
  swap(x);
  return *this;
}

//-----------------------------------  m_init()  ---------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
void
mbt_base<Key,Base,Compare,Allocator>::
m_init()
{
  m_size = 0;
  m_max_leaf_size = node_size() / sizeof(leaf_value);
  m_max_branch_size = node_size() / sizeof(branch_value);
  m_root = m_new_node<leaf_node>(0U, m_max_leaf_size);
  m_root->owner(this);
}

//------------------------------------  swap()  ----------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
void
mbt_base<Key,Base,Compare,Allocator>::
swap(mbt_base<Key,Base,Compare,Allocator>&x)
{
  std::swap(m_key_compare, x.m_key_compare);
  std::swap(m_value_compare, x.m_value_compare);
  std::swap(m_branch_value_compare, x.m_branch_value_compare);
  std::swap(m_alloc, x.m_alloc);
  std::swap(m_node_size, x.m_node_size);
  std::swap(m_size, x.m_size);
  std::swap(m_max_leaf_size, x.m_max_leaf_size);
  std::swap(m_max_branch_size, x.m_max_branch_size);
  std::swap(m_root, x.m_root);
  std::swap(m_root->_owner, x.m_root->_owner);
}

//---------------------------------  m_free_all()  -------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
void
mbt_base<Key,Base,Compare,Allocator>::
m_free_all(node* np)
{
  if (np->is_leaf())
    m_free_node<leaf_node>(node_cast<leaf_node>(np));
  else
  {
    branch_node* bp = node_cast<branch_node>(np);
    branch_value* it;
    for (it = bp->begin(); it <= bp->end(); ++it)
    {
      m_free_all(it->first);
    }
    m_free_node<branch_node>(bp);
  }
}

//-----------------------------------  clear()  ----------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
void
mbt_base<Key,Base,Compare,Allocator>::
clear() BOOST_NOEXCEPT
{
  m_free_all(m_root);
  m_size = 0;
  m_root = m_new_node<leaf_node>(0U, m_max_leaf_size);
  m_root->owner(this);
}

//----------------------------------  m_new_node  --------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
template <class Node>
Node*
mbt_base<Key,Base,Compare,Allocator>::
m_new_node(uint16_t height_, size_type max_elements)
{
  std::size_t node_size = sizeof(Node) + Node::extra_space()
    + max_elements * sizeof(typename Node::value_type);

  Node* np = reinterpret_cast<Node*>(new char[node_size]);
#ifndef NDEBUG
  std::memset(np, 0, node_size);
#endif
  np->height(height_);
  np->size(0);
  np->parent_node(0);
  np->parent_element(0);
  return np;
}

//---------------------------------  m_free_node  --------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
template <class Node>
void
mbt_base<Key,Base,Compare,Allocator>::
m_free_node(Node* np)
{
  typedef typename Node::value_type value_type;
  for (value_type* it = np->begin(); it != np->end(); ++it)
  {
    it->~value_type();
  }
  delete [] reinterpret_cast<char*>(np);
}

//----------------------------------  m_begin()  ---------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::iterator
mbt_base<Key,Base,Compare,Allocator>::
m_begin() BOOST_NOEXCEPT
{
  if (empty())
    return end();

  branch_node* bp = node_cast<branch_node>(m_root);

  // work down the tree until a leaf is reached
  while (bp->is_branch())
  {
    // create the child->parent list
    node* child = bp->begin()->first;
    child->parent_node(bp);
    child->parent_element(bp->begin());
    bp = node_cast<branch_node>(child);
  }

  leaf_node* lp = node_cast<leaf_node>(bp);
  return iterator(lp, lp->begin());
}

//------------------------------------ m_last() ----------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::iterator
mbt_base<Key,Base,Compare,Allocator>::
m_last()
{
  if (empty())
    return end();

  branch_node* bp = node_cast<branch_node>(m_root);

  // work down the tree until a leaf is reached
  while (bp->is_branch())
  {
    // create the child->parent list
    node* child = bp->end()->first;
    child->parent_node(bp);
    child->parent_element(bp->end());
    bp = node_cast<branch_node>(child);
  }

  leaf_node* lp = node_cast<leaf_node>(bp);
  BOOST_ASSERT(lp->size());
  return iterator(lp, lp->begin()+(lp->size()-1));
}

//-----------------------------  m_op_square_brackets()  -------------------------------//

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::mapped_type&
mbt_base<Key,Base,Compare,Allocator>::
m_op_square_brackets(const key_type& k)
{
  iterator it = m_special_lower_bound(k);

  bool not_found = it.m_element == it.m_node->end()
         || key_comp()(k, it->first)
         || key_comp()(it->first, k);

  if (not_found)
  {
    m_leaf_insert(make_value(k), it.m_node, it.m_element);
  }

  return it->second;
}

//-------------------------  m_op_square_brackets() r-value ----------------------------//

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::mapped_type&
mbt_base<Key,Base,Compare,Allocator>::
m_op_square_brackets(key_type&& k)
{
  iterator it = m_special_lower_bound(k);

  bool not_found = it.m_element == it.m_node->end()
         || key_comp()(k, it->first)
         || key_comp()(it->first, k);

  if (not_found)
  {
    m_leaf_insert(make_value(k), it.m_node, it.m_element);
  }

  return it->second;
}

//----------------------------------- m_new_root() -------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
void
mbt_base<Key,Base,Compare,Allocator>::
m_new_root()
{
  //std::cout << "***adding new root\n";
  // create a new root containing only the end pseudo-element
  node* old_root = m_root;
  branch_node* new_root
    = m_new_node<branch_node>(old_root->height()+1, m_max_branch_size);
  new_root->begin()->first = old_root;
  new_root->owner(this);
  old_root->parent_node(new_root);
  old_root->parent_element(new_root->begin());
  m_root = new_root;
}

//-------------------------------  m_insert_unique()  ----------------------------------//

template <class Key, class Base, class Compare, class Allocator>
std::pair<typename mbt_base<Key,Base,Compare,Allocator>::iterator, bool>
mbt_base<Key,Base,Compare,Allocator>::
m_insert_unique(const value_type& x)
{
  iterator insert_point = m_special_lower_bound(key(x));

  bool unique = insert_point.m_element == insert_point.m_node->end()
         || key_comp()(key(x), key(*insert_point))
         || key_comp()(key(*insert_point), key(x));

  if (!unique)
    return std::pair<iterator, bool>(insert_point, false);

  value_type v = x;
  m_leaf_insert(std::move(v), insert_point.m_node, insert_point.m_element);
  return std::pair<iterator, bool>(insert_point, true);
}

//---------------------------  m_insert_unique() r-value  ------------------------------//

template <class Key, class Base, class Compare, class Allocator>
std::pair<typename mbt_base<Key,Base,Compare,Allocator>::iterator, bool>
mbt_base<Key,Base,Compare,Allocator>::
m_insert_unique(value_type&& x)
{
  iterator insert_point = m_special_lower_bound(key(x));

  bool unique = insert_point.m_element == insert_point.m_node->end()
         || key_comp()(key(x), key(*insert_point))
         || key_comp()(key(*insert_point), key(x));

  if (!unique)
    return std::pair<iterator, bool>(insert_point, false);

  m_leaf_insert(x, insert_point.m_node, insert_point.m_element);
  return std::pair<iterator, bool>(insert_point, true);
}

//-----------------------------  m_insert_non_unique()  --------------------------------//

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::iterator
mbt_base<Key,Base,Compare,Allocator>::
m_insert_non_unique(const value_type& x)
{
  iterator insert_point = m_special_upper_bound(key(x));

  value_type v = x;
  m_leaf_insert(std::move(v), insert_point.m_node, insert_point.m_element);
  return insert_point;
}

//-------------------------  m_insert_non_unique() r-value  ----------------------------//

template <class Key, class Base, class Compare, class Allocator>
template <class P>
typename mbt_base<Key,Base,Compare,Allocator>::iterator
mbt_base<Key,Base,Compare,Allocator>::
m_insert_non_unique(P&& x)
{
  iterator insert_point = m_special_lower_bound(key(x));

  m_leaf_insert(x, insert_point.m_node, insert_point.m_element);
  return insert_point;
}

//-------------------------------  m_leaf_insert()  ------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
void
mbt_base<Key,Base,Compare,Allocator>::
m_leaf_insert(value_type&& x,
              leaf_node*& old_node, leaf_value*& ep)
    // Requires: old_node points to the node where insertion is to occur
    //           ep points to the element where insertion is to occur
    // Effects:  Inserts k and mv at *ep. If the insertion causes a node to be split,
    //           and the ep falls on the newly split node, old_node and ep are set to
    //           point to the new node and appropriate element
{
  leaf_node*   np = old_node;
  leaf_value*  insert_begin = ep;
  leaf_node*   new_node = 0;

  BOOST_ASSERT_MSG(np->size() <= m_max_leaf_size, "internal error");


  if (np->size() == m_max_leaf_size)  // if no room on node, node must be split
  {
    //std::cout << "***splitting a leaf\n";
    if (np->is_root()) // splitting the root?
      m_new_root();  // create a new root

    new_node = m_new_node<leaf_node>(np->height(), m_max_leaf_size);  // create the new node

//    // ck pack conditions now, since leaf seq list update may chg header().last_node_id()
//    if (m_ok_to_pack
//        && (insert_begin != np->leaf().end() || np->node_id() != header().last_node_id()))
//      m_ok_to_pack = false;  // conditions for pack optimization not met

//    // apply pack optimization if applicable
//    if (m_ok_to_pack)  // have all inserts been ordered and no erases occurred?
//    {
//      // pack optimization: instead of splitting np, just put value alone on new_node
//      m_memcpy_value(&*new_node->leaf().begin(), &key_, key_size, &mapped_value_, mapped_size);  // insert value
//      new_node->size(value_size);
//      BOOST_ASSERT(np->parent()->node_id() == np->parent_node_id()); // max_cache_size logic OK?
//      m_branch_insert(np->parent(), np->parent_element(),
//        key(*new_node->leaf().begin()), new_node->node_id());
//      ++m_size;
//      return const_iterator(new_node, new_node->leaf().begin());
//    }

    // split node np by moving half the elements to node new_node
    new_node->size(np->size() / 2);  // round down to minimize move size
    detail::placement_move(np->end() - new_node->size(), np->end(), new_node->begin());
    np->size(np->size() - new_node->size());

    // TODO: if the insert point will fall on the new node, it would be faster to
    // copy the portion before the insert point, copy the value being inserted, and
    // finally copy the portion after the insert point. However, that's a fair amount of
    // additional code for something that only happens on half of all leaf splits on average.

    // adjust np and insert_begin if they now fall on the new node due to the split
    if (insert_begin > np->end())
    {
      BOOST_ASSERT((insert_begin-np->end()) >= 0);  // ck offset validity
      BOOST_ASSERT((insert_begin-np->end()) <= static_cast<std::ptrdiff_t>(new_node->size()));

      insert_begin = new_node->begin() + (insert_begin - np->end());
      np = new_node;
    }
  }

  BOOST_ASSERT(insert_begin >= np->begin());
  BOOST_ASSERT(insert_begin <= np->end());

  // prep memory at end() for use
  ::new (np->end()) leaf_value;

  // make room for insert
  std::move_backward(insert_begin, np->end(), np->end()+1);

  //  insert x at insert_begin
  *insert_begin = x;
  ++np->_size;
  ++m_size;

  // if there is a new node, its initial key and leaf_node* are inserted into parent
  if (new_node)
  {
    key_type first_key = key(*new_node->begin());  // avoid unwanted move
    m_branch_insert(std::move(first_key), old_node, new_node);

    // if the insert point changed, update the caller's pointers
    if (ep != insert_begin)
    {
      old_node = np;
      ep = insert_begin;
    }
  }

//std::cout << "***insert done" << std::endl;
}

//-------------------------------  m_branch_insert()  ----------------------------------//

template <class Key, class Base, class Compare, class Allocator>
void
mbt_base<Key,Base,Compare,Allocator>::
m_branch_insert(key_type&& k, node* old_np, node* new_np)
    // Effects:  inserts k and new_np at old_np->parent_element()->second and
    //           (old_np->parent_element()+1)->first, respectively
    // Postcondition: For the nodes pointed to by old_np and new_np, parent_node() and
    //           parent_element() are valid. i.e. updated if needed
{
  branch_node*   old_node = old_np->parent_node();
  branch_node*   insert_node = old_node;
  branch_value*  insert_begin = old_np->parent_element();
  branch_node*   new_node = 0;

  //std::cout << "*****branch insert, height=" << old_node->height() << "\n";

  BOOST_ASSERT_MSG(old_node->size() <= m_max_branch_size, "internal error");


  if (old_node->size() == m_max_branch_size)  // if no room on node, node must be split
  {
    if (old_node->is_root()) // splitting the root?
      m_new_root();  // create a new root

    new_node = m_new_node<branch_node>(old_node->height(), m_max_branch_size);

//    // ck pack conditions now, since leaf seq list update may chg header().last_node_id()
//    if (m_ok_to_pack
//        && (insert_begin != old_node->leaf().end() || old_node->node_id() != header().last_node_id()))
//      m_ok_to_pack = false;  // conditions for pack optimization not met
//
//    // apply pack optimization if applicable
//    if (m_ok_to_pack)  // have all inserts been ordered and no erases occurred?
//    {
//      // pack optimization: instead of splitting old_node, just put value alone on old_node2
//      m_memcpy_value(&*old_node2->leaf().begin(), &key_, key_size, &mapped_value_, mapped_size);  // insert value
//      old_node2->size(value_size);
//      BOOST_ASSERT(old_node->parent()->node_id() == old_node->parent_node_id()); // max_cache_size logic OK?
//      m_branch_insert(old_node->parent(), old_node->parent_element(),
//        key(*old_node2->leaf().begin()), old_node2->node_id());
//      ++m_size;
//      return const_iterator(old_node2, old_node2->leaf().begin());
//    }

    // split node old_node by moving half the elements to node new_node
    new_node->size(old_node->size() / 2);  // round down to minimize move size
    detail::placement_move(old_node->end() - new_node->size(), old_node->end(),
      new_node->begin());
    new_node->end()->first = old_node->end()->first; // copy the end pseudo-element
    old_node->size(old_node->size() - (new_node->size()+1));
//    cout << "\nupdated old_node->size()=" << old_node->size() << '\n';

    // Do the promotion now, since old_node->end().second is the key that needs to be
    // promoted regardless of which node the insert occurs on.
    m_branch_insert(std::move(old_node->end()->second), old_node, new_node);
    old_node->end()->second.~key_type();  // prep for insert expects uninitialized memory

    // TODO: if the insert point will fall on the new node, it would be faster to
    // move the portion before the insert point, copy the value being inserted, and
    // finally move the portion after the insert point. However, that's a fair amount of
    // additional code for something that only happens on half of branch splits on average.

    // adjust old_node and insert_begin if they now fall on the new node due to the split
    if (insert_begin > old_node->end())
    {
//      cout << "\ninsert_begin offset=" << insert_begin - old_node->end() - 1 << endl;
      BOOST_ASSERT((insert_begin-old_node->end()-1) >= 0);  // ck offset validity
      BOOST_ASSERT((insert_begin-old_node->end()-1) <= static_cast<std::ptrdiff_t>(new_node->size()));

      insert_begin = new_node->begin() + (insert_begin - old_node->end() - 1);
      insert_node = new_node;

      // update old_np's parent pointers
      old_np->parent_node(insert_node);
      old_np->parent_element(insert_begin);
    }
  }

  BOOST_ASSERT(insert_begin >= insert_node->begin());
  BOOST_ASSERT(insert_begin <= insert_node->end());

  // prep memory at end()->second for use
  ::new (&insert_node->end()->second) key_type;

  // make room for insert
  if (insert_begin != insert_node->end())
  {
    (insert_node->end()+1)->first = insert_node->end()->first;  // end pseudo-element
    std::move_backward(insert_begin+1, insert_node->end(), insert_node->end()+1);
    (insert_begin+1)->second = std::move(insert_begin->second);  // key
  }

  //  insert x
  insert_begin->second = k;
  (insert_begin+1)->first = new_np;
  ++insert_node->_size;

  // update new_np's parent pointers
  new_np->parent_node(insert_node);
  new_np->parent_element(insert_begin+1);

  //std::cout << "*****branch insert done" << std::endl;
}

//------------------------------------- erase() ----------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::iterator
mbt_base<Key,Base,Compare,Allocator>::
erase(const_iterator pos)
{
  BOOST_ASSERT_MSG(pos != end(), "erase() on end iterator");
  BOOST_ASSERT(pos.m_node);
  BOOST_ASSERT(pos.m_node->is_leaf());
  BOOST_ASSERT(pos.m_element < pos.m_node->end());
  BOOST_ASSERT(pos.m_element >= pos.m_node->begin());

  //m_ok_to_pack = false;  // TODO: is this too conservative?

  --m_size;

  if (!pos.m_node->is_root()  // not root?
    && (pos.m_node->size() == 1))  // only 1 element on node?
  {
    // erase a single value leaf node that is not the root
    leaf_node* nxt (pos.m_node->next_node(pos.m_node));
    iterator nxt_it (nxt->is_root() ? end() : iterator(nxt, nxt->begin()));  // [note 1]
    m_erase_from_parent(pos.m_node);  // unlink from tree
    m_free_node(pos.m_node);
    return nxt_it;
  }
  else
  {
    // erase an element from a leaf with multiple elements or erase the only element
    // on a leaf that is also the root; these use the same logic because they do not remove
    // the node from the tree.
    std::move(pos.m_element+1, pos.m_node->end(), pos.m_element);
    pos.m_node->size(pos.m_node->size()-1);
    pos.m_node->end()->~leaf_value();
    if (pos.m_element != pos.m_node->end())
      return iterator(pos.m_node, pos.m_element);

    leaf_node* nxt (pos.m_node->next_node(pos.m_node));
    return nxt->is_root() ? end() : iterator(nxt, nxt->begin());
  }
}
// [note 1] the call to m_erase_parent() may change the root, so build the next iterator
//          before the call to m_erase_parent()

//------------------------------ m_erase_branch_value() --------------------------------//

template <class Key, class Base, class Compare, class Allocator>
void
mbt_base<Key,Base,Compare,Allocator>::
m_erase_from_parent(node* child)
{
  branch_node* np (child->parent_node());
  branch_value* ep (child->parent_element());

  BOOST_ASSERT(np->is_branch());
  BOOST_ASSERT(ep >= np->begin());
  BOOST_ASSERT(ep <= np->end());  // may be end pseudo-element

  //  Example 1:
  //
  //                        * C *          root branch, height 2
  //                       /     \
  //                      *       *        branches, height 1
  //                     /         \
  //                    A...        C...   leaves, height 0

  if (np->is_empty())
  // If the node is empty, except for the end pseudo-element, then just unlink from
  // parent and free node. This will happen for a height 1 branch in the example
  // if the last element its child leaf is erased.
  {
    BOOST_ASSERT(!np->is_root());  // a branch node that is the root should not be empty
    m_erase_from_parent(np); // unlink from tree
    m_free_node(np);
    return;
  }

  if (ep != np->end())
  {
    if (ep != np->begin())
    // Process erase of element other than the first element (ie preserve branch invariants)
    // Example 2: erase with ep pointing to element 2:B below
    // Branch:        1:A 2:B 3:C 4:D 5
    // Postcondition: 1:B 3:C 4:D 5
    {
      (ep-1)->second = std::move(ep->second);
    }

    std::move(ep+1, np->end(), ep);
    (np->end()-1)->first = np->end()->first;
  }

  // common processing for all non-empty nodes
  (np->end()-1)->second.~key_type();
  np->size(np->size()-1);

  // Trim the tree if applicable. In Example 1 above, if the last element on either of
  // the leaves is erased, then all the branch nodes must be removed and the remaining
  // leaf node becomes the new root.
  while (np->is_empty()      // node is empty except for the end pseudo-element
         && np->is_root()    // node is the root
         && !np->is_leaf())  // node isn't a leaf (happens if iteration reaches a leaf)
  {
    // make the end pseudo-element the new root and then free this node
    m_root = np->end()->first;
    m_root->parent_node(0);
    m_root->owner(this);
    m_free_node(np);
    np = node_cast<branch_node>(m_root);
  }
}

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::size_type
mbt_base<Key,Base,Compare,Allocator>::
erase(const key_type& k)
{
  size_type count = 0;
  const_iterator it = lower_bound(k);

  while (it != end() && !key_comp()(k, key(*it)))
  {
    ++count;
    it = erase(it);
  }
  return count;
}

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::iterator
mbt_base<Key,Base,Compare,Allocator>::
erase(const_iterator first, const_iterator last)
{
  // caution: last must be revalidated when on the same node as first
  while (first != last)
  {
    if (last != end() && first.m_node == last.m_node)
    {
      BOOST_ASSERT(first.m_element < last.m_element);
      --last;  // revalidate in anticipation of erasing a prior element on same node
    }
    first = erase(first);
  }
  return last;
}

////----------------------------------- m_sub_tree_begin() -------------------------------//
//
//template <class Key, class Base, class Traits, class Comp>
//typename btree_base<Key,Base,Traits,Comp>::iterator
//btree_base<Key,Base,Traits,Comp>::m_sub_tree_begin(node_id_type id)
//{
//  if (empty())
//    return end();
//  btree_node_ptr np =  m_mgr.read(id);
//
//  // work down the tree until a leaf is reached
//  while (np->is_branch())
//  {
//    // create the child->parent list
//    btree_node_ptr child_np = m_mgr.read(np->branch().begin()->node_id());
//    child_np->parent(np);
//    child_np->parent_element(np->branch().begin());
//#   ifndef NDEBUG
//    child_np->parent_node_id(np->node_id());
//#   endif
//
//    np = child_np;
//  }
//
//  return iterator(np, np->leaf().begin());
//}

//-----------------------------  m_special_lower_bound()  ------------------------------//

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::iterator
mbt_base<Key,Base,Compare,Allocator>::
m_special_lower_bound(const key_type& k) const
{
  branch_node* bp = node_cast<branch_node>(m_root);

  // search branches down the tree until a leaf is reached
  while (bp->is_branch())
  {
    branch_value* low
      = std::lower_bound(bp->begin(), bp->end(), k, branch_comp());

    if ( /*(header().flags() & btree::flags::unique)
      &&*/ low != bp->end()
      && !key_comp()(k, low->second)) // if k isn't less that low key, low is equal
      ++low;                         // and so must be incremented; this follows from
                                     // the branch node invariant for unique containers

    // create the child->parent list
    node* child = low->first;
    child->parent_node(bp);
    child->parent_element(low);

    bp = node_cast<branch_node>(child);
  }

  //  search leaf
  leaf_node* lp = node_cast<leaf_node>(bp);
  leaf_value* low
    = std::lower_bound(lp->begin(), lp->end(), k, value_comp());

  return iterator(lp, low);
}

//---------------------------------- lower_bound() -------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::iterator
mbt_base<Key,Base,Compare,Allocator>::
lower_bound(const key_type& k)
{
  iterator low = m_special_lower_bound(k);

  if (low.m_element != low.m_node->end())
    return low;

  if (low.m_node->begin() == low.m_node->end())
  {
    BOOST_ASSERT(empty());
    return end();
  }

  // lower bound is first element on next node
  leaf_node* np = low.m_node->next_node(low.m_node);
  return !np->is_root() ? iterator(np, np->begin()) : end();
}

//-----------------------------  m_special_upper_bound()  ------------------------------//

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::iterator
mbt_base<Key,Base,Compare,Allocator>::
m_special_upper_bound(const key_type& k) const
{
  branch_node* bp = node_cast<branch_node>(m_root);

  // search branches down the tree until a leaf is reached
  while (bp->is_branch())
  {
    branch_value* up
      = std::upper_bound(bp->begin(), bp->end(), k, branch_comp());

    // create the child->parent list
    node* child = up->first;
    child->parent_node(bp);
    child->parent_element(up);

    bp = node_cast<branch_node>(child);
  }

  //  search leaf
  leaf_node* lp = node_cast<leaf_node>(bp);
  leaf_value* up
    = std::upper_bound(lp->begin(), lp->end(), k, value_comp());

  return iterator(lp, up);
}

//---------------------------------- upper_bound() -------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::iterator
mbt_base<Key,Base,Compare,Allocator>::
upper_bound(const key_type& k)
{
  iterator up = m_special_upper_bound(k);

  if (up.m_element != up.m_node->end())
    return up;

  // upper bound is first element on next node
  leaf_node* np = up.m_node->next_node(up.m_node);
  return !np->is_root() ? iterator(np, np->begin()) : end();
}

//------------------------------------- find() -----------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
typename mbt_base<Key,Base,Compare,Allocator>::iterator
mbt_base<Key,Base,Compare,Allocator>::
find(const key_type& k)
{
  iterator low = lower_bound(k);
  return (low != end() && !key_comp()(k, key(*low)))
    ? low
    : end();
}

//----------------------------------- dump_dot -----------------------------------------//

template <class Key, class Base, class Compare, class Allocator>
void
mbt_base<Key,Base,Compare,Allocator>::
m_dump_node(std::ostream& os, node* np) const
{
  if (np->is_leaf())
  {
    leaf_node* lp = node_cast<leaf_node>(np);
    os << "node_" << lp << "[label = \"<f0> ";
    for (leaf_value* it = lp->begin(); it != lp->end(); ++it)
    {
      if (it != lp->begin())
        os << '|';
      os << it->first << ':' << it->second;
    }
    os << "\",fillcolor=\"palegreen\"];\n";
  }
  else
  {
    os << "node_" << np << "[label = \"";
    branch_node* bp = node_cast<branch_node>(np);
    int f = 0;
    branch_value* it;
    for (it = bp->begin(); it != bp->end(); ++it)
    {
      os << "<f" << f << ">|" << it->second << "|";
      ++f;
    }
    os << "<f" << f << ">\",fillcolor=\"lightblue\"];\n";
    f = 0;
    for (it = bp->begin(); it != bp->end(); ++it)
    {
      os << "\"node_" << bp << "\":f" << f << " -> \"node_" << it->first << "\":f0;\n";
      m_dump_node(os, it->first);
      ++f;
    }
    os << "\"node_" << bp << "\":f" << f << " -> \"node_" << it->first << "\":f0;\n";
    m_dump_node(os, it->first);
  }
}

template <class Key, class Base, class Compare, class Allocator>
void
mbt_base<Key,Base,Compare,Allocator>::
dump_dot(std::ostream& os) const
{
  os << "digraph btree {\nrankdir=LR;\nfontname=Courier;\n"
    "node [shape = record,margin=.1,width=.1,height=.1,fontname=Courier,style=\"filled\"];\n";

  m_dump_node(os, m_root);

  os << "}" << std::endl;
}

//--------------------------------  node::next_node()  ---------------------------------//

template <class Key, class Base, class Compare, class Allocator>
template <class Node>
Node*
mbt_base<Key,Base,Compare,Allocator>::node::
next_node(Node*)  // return next node at same height, root_node if end
{
  if (this->is_root())
    return node_cast<Node>(this);

  branch_node*   parent_np = this->parent_node();
  branch_value*  parent_ep = this->parent_element();

  if (parent_ep != parent_np->end())
    ++parent_ep;
  else
  {
    parent_np = this->parent_node()->next_node(parent_np);
    if (parent_np->is_root())
      return node_cast<Node>(parent_np);
    parent_ep = parent_np->begin();
  }

  Node* np = node_cast<Node>(parent_ep->first);
  np->parent_node(parent_np);
  np->parent_element(parent_ep);
  return np;
}

//--------------------------  iterator::increment()  -----------------------------------//

template <class Key, class Base, class Compare, class Allocator>
template <class VT>
void
mbt_base<Key,Base,Compare,Allocator>::iterator_type<VT>::
increment()
{
  BOOST_ASSERT_MSG(m_element, "attempt to increment uninitialized iterator");
  BOOST_ASSERT_MSG(m_node, "attempt to increment end iterator");
  BOOST_ASSERT(m_node->is_leaf());
  BOOST_ASSERT(m_element >= m_node->begin());
  BOOST_ASSERT(m_element < m_node->end());

  if (++m_element != m_node->end())
    return;

  m_node = m_node->next_node(m_node);  // next leaf node, or root node if end

  if (!m_node->is_root())
  {
    m_element = m_node->begin();
    BOOST_ASSERT(m_element != m_node->end());
  }
  else // end() reached
  {
    m_owner = m_node->owner();
    m_node = 0;
  }
}

//-------------------------------  node::prior_node()  ---------------------------------//

template <class Key, class Base, class Compare, class Allocator>
template <class Node>
Node*
mbt_base<Key,Base,Compare,Allocator>::node::
prior_node(Node*)  // return prior node at same height, root_node if end
{
  if (this->is_root())
    return node_cast<Node>(this);

  branch_node*   parent_np = this->parent_node();
  branch_value*  parent_ep = this->parent_element();

  if (parent_ep != parent_np->begin())
    --parent_ep;
  else
  {
    parent_np = this->parent_node()->prior_node(parent_np);
    if (parent_np->is_root())
      return node_cast<Node>(parent_np);
    parent_ep = parent_np->end();
  }

  Node* np = node_cast<Node>(parent_ep->first);
  np->parent_node(parent_np);
  np->parent_element(parent_ep);
  return np;
}

//--------------------------  iterator::decrement()  -----------------------------------//

template <class Key, class Base, class Compare, class Allocator>
template <class VT>
void
mbt_base<Key,Base,Compare,Allocator>::iterator_type<VT>::
decrement()
{
  BOOST_ASSERT_MSG(m_element, "attempt to increment uninitialized iterator");

  if (!m_node)  // end iterator
    *this = m_owner->m_last();
  else if (m_element != m_node->begin())  // not first element
    --m_element;
  else  // not on this node
  {
    m_node = m_node->prior_node(m_node);

    if (m_node->is_root())  // precondition violation, so all bets are off
    {
      BOOST_ASSERT_MSG(!m_node->is_root(), "attempt to decrement begin() iterator");
      throw std::runtime_error("attempt to decrement begin() iterator");
    }
    else
    {
      m_element = m_node->end();
      BOOST_ASSERT(m_element != m_node->begin());
      --m_element;
    }
  }
}

}  // btree
}  // boost

#ifdef BOOST_MSVC
#  pragma warning(pop)
#endif

#endif  // BOOST_MBT_MAP_BASE_HPP
