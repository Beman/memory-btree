#ifndef BOOST_MBT_MAP_HPP
#define BOOST_MBT_MAP_HPP

#define BOOST_NOEXCEPT

#include <boost/config/warning_disable.hpp>
#include <boost/config.hpp>

#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable: 4996)  // ... Function call with parameters that may be unsafe
#endif

#include <cstddef>
#include <functional>
#include <utility>
#include <memory>
#include <new>
#include <iterator>
#include <algorithm>
#include <boost/cstdint.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/assert.hpp>
#include <boost/btree/detail/placement_move.hpp>
#include <cstring> // for memset


/*
TODO:

  * Implement ~mbt_map() and clear()

  * new_node() should use allocator!

*/

namespace boost {
namespace btree {

  template <class Key, class T, class Compare = std::less<Key>,
    class Allocator = std::allocator<std::pair<const Key, T> > >
  class mbt_map   // short for memory_btree_map
  {
    class node;
    class leaf_node;
    class branch_node;
    template <class VT>
      class iterator_type;
  public:

    // types:
    typedef Key                                     key_type;
    typedef T                                       mapped_type;
    typedef std::pair<const Key, T>                 value_type;
    typedef Compare                                 key_compare;
    typedef Allocator                               allocator_type;
    typedef value_type&                             reference;
    typedef const value_type&                       const_reference;
    typedef iterator_type<value_type>               iterator; // see 23.2
    typedef iterator_type<const value_type>         const_iterator; // see 23.2
    typedef std::size_t                             size_type; // see 23.2
    typedef std::ptrdiff_t                          difference_type;// see 23.2
//    typedef
//      typename std::allocator_traits<Allocator>::pointer pointer;
//    typedef
//      typename std::allocator_traits<Allocator>::const_pointer
//                                                    const_pointer;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

    static const size_type                          default_node_size = 4096;

    class value_compare
    {
      friend class mbt_map;
    protected:
      Compare comp;
      value_compare(Compare c) : comp(c) {}
    public:
      typedef bool                                  result_type;
      typedef value_type                            first_argument_type;
      typedef value_type                            second_argument_type;
      bool operator()(const value_type& x, const value_type& y) const
      {
        return comp(x.first, y.first);
      }
      bool operator()(const Key& k, const value_type& y) const
      {
        return comp(k, y.first);
      }
      bool operator()(const value_type& x, const Key& k) const
      {
        return comp(x.first, k);
      }
    };

    // 23.4.4.2, construct/copy/destroy:
    explicit mbt_map(size_type node_sz = default_node_size,
      const Compare& comp = Compare(), const Allocator& alloc = Allocator())
      : m_key_compare(comp), m_value_compare(comp), m_branch_value_compare(comp),
        m_alloc(alloc) {m_init(node_sz);}

    template <class InputIterator>
      mbt_map(InputIterator first, InputIterator last,
      const Compare& comp = Compare(), const Allocator& = Allocator());
    mbt_map(const mbt_map<Key,T,Compare,Allocator>& x);
    mbt_map(mbt_map<Key,T,Compare,Allocator>&& x);
    explicit mbt_map(const Allocator&);
    mbt_map(const mbt_map&, const Allocator&);
    mbt_map(mbt_map&&, const Allocator&);
//    mbt_map(initializer_list<value_type>, const Compare& = Compare(),
//      const Allocator& = Allocator());
   ~mbt_map();
    mbt_map<Key,T,Compare,Allocator>&
      operator=(const mbt_map<Key,T,Compare,Allocator>& x);
    mbt_map<Key,T,Compare,Allocator>&
      operator=(mbt_map<Key,T,Compare,Allocator>&& x);
//    mbt_map& operator=(initializer_list<value_type>);
    allocator_type get_allocator() const BOOST_NOEXCEPT;

    // iterators:
    iterator                begin() BOOST_NOEXCEPT           {return m_begin();}
    const_iterator          begin() const BOOST_NOEXCEPT     {return const_cast<mbt_map*>(this)->m_begin();}
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

    // 23.4.4.3, element access:
    T&                      operator[](const key_type& x);
    T&                      operator[](key_type&& x);
    T&                      at(const key_type& x);
    const T&                at(const key_type& x) const;

    // 23.4.4.4, modifiers:
    //template <class... Args>
    //  std::pair<iterator, bool>
    //                        emplace(Args&&... args);
    //template <class... Args>
    //  iterator              emplace_hint(const_iterator position, Args&&... args);

    std::pair<iterator, bool>
                            insert(const value_type& x);
    std::pair<iterator, bool>
                            insert(value_type&& x);

//    template <class P>
//      std::pair<iterator, bool>
//                            insert(P&& x);
//    iterator                insert(const_iterator position, const value_type& x);
//    template <class P>
//      iterator              insert(const_iterator position, P&&);
//    template <class InputIterator>
//      void                  insert(InputIterator first, InputIterator last);
//    void                    insert(initializer_list<value_type>);

    iterator                erase(const_iterator position);
    size_type               erase(const key_type& x);
    iterator                erase(const_iterator first, const_iterator last);
    void                    swap(mbt_map<Key,T,Compare,Allocator>&);
    void                    clear() BOOST_NOEXCEPT;

    // observers:
    key_compare             key_comp() const   {return m_key_compare;}
    value_compare           value_comp() const {return m_value_compare;}

    // 23.4.4.5, map operations:
    iterator                find(const key_type& x);
    const_iterator          find(const key_type& x) const {return const_cast<mbt_map*>(this)->find(x);}
    size_type               count(const key_type& x) const;
    iterator                lower_bound(const key_type& x);
    const_iterator          lower_bound(const key_type& x) const {return const_cast<mbt_map*>(this)->lower_bound(x);}
    iterator                upper_bound(const key_type& x);
    const_iterator          upper_bound(const key_type& x) const {return const_cast<mbt_map*>(this)->upper_bound(x);}
    std::pair<iterator, iterator>
                            equal_range(const key_type& x) {return std::make_pair(lower_bound(x), upper_bound(x));}
    std::pair<const_iterator, const_iterator>
                            equal_range(const key_type& x) const {return std::make_pair(lower_bound(x), upper_bound(x));}

  private:

    friend class node;

    typedef std::pair<Key, T>      leaf_value;
    typedef std::pair<node*, Key>  branch_value;  // first is pointer to child node

    //----------------------------------------------------------------------------------//
    //                             private nested classes                               //
    //----------------------------------------------------------------------------------//

    //-------------------------  class branch_value_compare  ---------------------------//

    class branch_value_compare
    {
      friend class mbt_map;
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
    public:
      uint16_t        _height;          // 0 for a leaf node
      uint16_t        _size;
      branch_node*    _parent_node;     // 0 for the root node
      union
      {
        branch_value* _parent_element;  // non-root node
        mbt_map*      _owner;           // root node
      };

      uint16_t      height() const                  {return _height;}
      bool          is_leaf() const                 {return _height == 0;}
      bool          is_branch() const               {return _height != 0;}
      bool          is_root() const                 {return _parent_node == 0;}
      std::size_t   size() const                    {return _size;}
      branch_node*  parent_node() const             {return _parent_node;}
      branch_value* parent_element() const          {return _parent_element;}
      mbt_map*      owner() const                   {return _owner;}

      void          height(uint16_t h)              {_height = h;}
      void          size(std::size_t n)             {_size = n;}
      void          parent_node(branch_node* p)     {_parent_node = p;}
      void          parent_element(branch_value* p) {_parent_element = p;}
      void          owner(mbt_map* o)               {_owner = o;}
    };

    //------------------------------  class leaf_node  ---------------------------------//

    class leaf_node : public node
    {
    public:
      typedef typename mbt_map::leaf_value   value_type;
      typedef typename mbt_map::mapped_type  mapped_type;

      leaf_value     _leaf_values[1];               // actual size determined at runtime

      leaf_value*    begin()                        {return _leaf_values;}
      leaf_value*    end()                          {return _leaf_values + node::_size;}

      leaf_node*     next_node();  // returns next leaf node; root node if end
     };

    //-----------------------------  class branch_node  --------------------------------//

    class branch_node : public node
    {
    public:
      typedef typename mbt_map::branch_value  value_type;
      typedef node*                  mapped_type;

      branch_value   _branch_values[1];             // actual size determined at runtime

      branch_value*  begin()                        {return _branch_values;}
      branch_value*  end()                          {return _branch_values + node::_size;}
      // pseudo-element end()->first is valid; b-tree branches have size() + 1
      // child pointers - see your favorite computer science textbook.

      branch_node*   next_node();  // returns next node at same height; root node if end
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

      iterator_type(typename mbt_map::leaf_node* np,
                    typename mbt_map::leaf_value* ep)
        : m_node(np), m_element(ep) {}

      template <class VU>
      iterator_type(iterator_type<VU> const& other)
        : m_node(other.m_node), m_element(other.m_element) {}

    private:
      iterator_type(mbt_map* owner)  // construct end iterator
        : m_node(0), m_owner(owner) {}
      iterator_type(const mbt_map* owner)  // construct end iterator
        : m_node(0), m_owner(const_cast<mbt_map*>(owner)) {}

      friend class boost::iterator_core_access;
      friend class mbt_map;

      typename mbt_map::leaf_node*    m_node;      // 0 indicates end iterator

      union  // discriminated by m_node
      {
       typename mbt_map::leaf_value*  m_element;  // not end iterator
       mbt_map*                       m_owner;    // end iterator
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

    key_compare           m_key_compare;
    value_compare         m_value_compare;
    branch_value_compare  m_branch_value_compare;
    const Allocator&      m_alloc;
    size_type             m_size;             // number of elements in container
    size_type             m_max_leaf_size;    // maximum number of elements
    size_type             m_max_branch_size;  // maximum number of elements
    node*                 m_root;             // invariant: there is always a root

    //----------------------------------------------------------------------------------//
    //                            private member functions                              //
    //----------------------------------------------------------------------------------//

    iterator               m_begin() BOOST_NOEXCEPT;

    branch_value_compare   branch_comp() const {return m_branch_value_compare;}

    void m_init(std::size_t node_sz)
    {
      m_size = 0;
      m_max_leaf_size = node_sz / sizeof(leaf_value);
      m_max_branch_size = node_sz / sizeof(branch_value);
      m_root = m_new_node<leaf_node>(0U, m_max_leaf_size);
      m_root->owner(this);
    }

    template <class Node>
    Node* m_new_node(uint16_t height_, size_type max_elements);

    void  m_new_root();

    iterator m_special_lower_bound(const key_type& k) const;
    iterator m_special_upper_bound(const key_type& k) const;

    void m_leaf_insert(key_type&& k, mapped_type&& mv,
                  leaf_node*& np, leaf_value*& ep);
    // Remarks:  np points to the node where insertion is to occur
    //           ep points to the element where insertion is to occur
    // Effects:  Inserts k and mv at *ep. If the insertion causes a node to be split,
    //           and the ep falls on the newly split node, np and ep are set to point to
    //           the new node and appropriate element

    void m_branch_insert(key_type&& k, node* old_np, node* new_np);
    // Effects:  inserts k and new_np at old_np->parent_element()->second and
    //           (old_np->parent_element()+1)->first, respectively
    // Postcondition: For the nodes pointed to by old_np and new_np, parent_node() and
    //           parent_element() are valid. i.e. updated if needed

    static leaf_node* leaf_cast(node* np)     {return reinterpret_cast<leaf_node*>(np);}
    static branch_node* branch_cast(node* np) {return reinterpret_cast<branch_node*>(np);}

  };  // class mbt_map

//--------------------------------------------------------------------------------------//
//                                  implementation                                      //
//--------------------------------------------------------------------------------------//

//----------------------------------  ~mbt_map()  --------------------------------------//

template <class Key, class T, class Compare, class Allocator>
mbt_map<Key,T,Compare,Allocator>::
~mbt_map()
{
  // TODO ...
}

//----------------------------------  m_new_node  --------------------------------------//

template <class Key, class T, class Compare, class Allocator>
template <class Node>
Node*
mbt_map<Key,T,Compare,Allocator>::
m_new_node(uint16_t height_, size_type max_elements)
{
  std::size_t node_size = sizeof(Node)
    + (max_elements-1) * sizeof(typename Node::value_type);

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

//----------------------------------  m_begin()  ---------------------------------------//

template <class Key, class T, class Compare, class Allocator>
typename mbt_map<Key,T,Compare,Allocator>::iterator
mbt_map<Key,T,Compare,Allocator>::
m_begin() BOOST_NOEXCEPT
{
  if (empty())
    return end();

  branch_node* bp = branch_cast(m_root);

  // work down the tree until a leaf is reached
  while (bp->is_branch())
  {
    // create the child->parent list
    node* child = bp->begin()->first;
    child->parent_node(bp);
    child->parent_element(bp->begin());
    bp = branch_cast(child);
  }

  leaf_node* lp = leaf_cast(bp);
  return iterator(lp, lp->begin());
}

//---------------------------------  operator[]()  -------------------------------------//

template <class Key, class T, class Compare, class Allocator>
T&
mbt_map<Key,T,Compare,Allocator>::
operator[](const key_type& x)
{
  iterator it = m_special_lower_bound(x);

  bool not_found = it.m_element == it.m_node->end()
         || key_comp()(x, it->first)
         || key_comp()(it->first, x);

  if (not_found)
  {
    key_type k(x); 
    m_leaf_insert(std::move(k), T(), it.m_node, it.m_element);
  }

  return it->second;
}

//-----------------------------  operator[]() r-value ----------------------------------//

template <class Key, class T, class Compare, class Allocator>
T&
mbt_map<Key,T,Compare,Allocator>::
operator[](key_type&& x)
{
  iterator it = m_special_lower_bound(x);

  bool not_found = it.m_element == it.m_node->end()
         || key_comp()(x, it->first)
         || key_comp()(it->first, x);

  if (not_found)
  {
    m_leaf_insert(k, T(), it.m_node, it.m_element);
  }

  return it->second;
}

//----------------------------------- m_new_root() -------------------------------------//

template <class Key, class T, class Compare, class Allocator>
void
mbt_map<Key,T,Compare,Allocator>::
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

//-----------------------------------  insert()  ---------------------------------------//

template <class Key, class T, class Compare, class Allocator>
std::pair<typename mbt_map<Key,T,Compare,Allocator>::iterator, bool>
mbt_map<Key,T,Compare,Allocator>::
insert(const value_type& x)
{
  iterator insert_point = m_special_lower_bound(x.first);

  bool unique = insert_point.m_element == insert_point.m_node->end()
         || key_comp()(x.first, insert_point->first)
         || key_comp()(insert_point->first, x.first);

  if (!unique)
    return std::pair<iterator, bool>(insert_point, false);

  key_type k(x.first);
  mapped_type mv(x.second);
  m_leaf_insert(std::move(k), std::move(mv), insert_point.m_node, insert_point.m_element);
  return std::pair<iterator, bool>(insert_point, true);
}

//-------------------------------  insert() r-value  -----------------------------------//

template <class Key, class T, class Compare, class Allocator>
std::pair<typename mbt_map<Key,T,Compare,Allocator>::iterator, bool>
mbt_map<Key,T,Compare,Allocator>::
insert(value_type&& x)
{
  iterator insert_point = m_special_lower_bound(x.first);

  bool unique = insert_point.m_element == insert_point.m_node->end()
         || key_comp()(x.first, insert_point->first)
         || key_comp()(insert_point->first, x.first);

  if (!unique)
    return std::pair<iterator, bool>(insert_point, false);

  m_leaf_insert(std::move(static_cast<key_type>(x.first)), std::move(x.second),
    insert_point.m_node, insert_point.m_element);
  return std::pair<iterator, bool>(insert_point, true);
}

//-------------------------------  m_leaf_insert()  ------------------------------------//

template <class Key, class T, class Compare, class Allocator>
void
mbt_map<Key,T,Compare,Allocator>::
m_leaf_insert(key_type&& k,mapped_type&& mv,
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
  insert_begin->first = k;
  insert_begin->second = mv;
  ++np->_size;
  ++m_size;

  // if there is a new node, its initial key and leaf_node* are inserted into parent
  if (new_node)
  {
    key_type first_key = new_node->begin()->first;  // avoid unwanted move
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

template <class Key, class T, class Compare, class Allocator>
void
mbt_map<Key,T,Compare,Allocator>::
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

//-----------------------------  m_special_lower_bound()  ------------------------------//

template <class Key, class T, class Compare, class Allocator>
typename mbt_map<Key,T,Compare,Allocator>::iterator
mbt_map<Key,T,Compare,Allocator>::
m_special_lower_bound(const key_type& k) const
{
  branch_node* bp = branch_cast(m_root);

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

    bp = branch_cast(child);
  }

  //  search leaf
  leaf_node* lp = leaf_cast(bp);
  leaf_value* low
    = std::lower_bound(lp->begin(), lp->end(), k, value_comp());

  return iterator(lp, low);
}

//---------------------------------- lower_bound() -------------------------------------//

template <class Key, class T, class Compare, class Allocator>
typename mbt_map<Key,T,Compare,Allocator>::iterator
mbt_map<Key,T,Compare,Allocator>::
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
  leaf_node* np = low.m_node->next_node();
  return !np->is_root() ? iterator(np, np->begin()) : end();
}

//-----------------------------  m_special_upper_bound()  ------------------------------//

template <class Key, class T, class Compare, class Allocator>
typename mbt_map<Key,T,Compare,Allocator>::iterator
mbt_map<Key,T,Compare,Allocator>::
m_special_upper_bound(const key_type& k) const
{
  branch_node* bp = branch_cast(m_root);

  // search branches down the tree until a leaf is reached
  while (bp->is_branch())
  {
    branch_value* up
      = std::upper_bound(bp->begin(), bp->end(), k, branch_comp());

    // create the child->parent list
    node* child = up->first;
    child->parent_node(bp);
    child->parent_element(up);

    bp = branch_cast(child);
  }

  //  search leaf
  leaf_node* lp = leaf_cast(bp);
  leaf_value* up
    = std::upper_bound(lp->begin(), lp->end(), k, value_comp());

  return iterator(lp, up);
}

//---------------------------------- upper_bound() -------------------------------------//

template <class Key, class T, class Compare, class Allocator>
typename mbt_map<Key,T,Compare,Allocator>::iterator
mbt_map<Key,T,Compare,Allocator>::
upper_bound(const key_type& k)
{
  iterator up = m_special_upper_bound(k);

  if (up.m_element != up.m_node->end())
    return up;

  // upper bound is first element on next node
  leaf_node* np = up.m_node->next_node();
  return !np->is_root() ? iterator(np, np->begin()) : end();
}

//------------------------------------- find() -----------------------------------------//

template <class Key, class T, class Compare, class Allocator>
typename mbt_map<Key,T,Compare,Allocator>::iterator
mbt_map<Key,T,Compare,Allocator>::
find(const key_type& k)
{
  iterator low = lower_bound(k);
  return (low != end() && !key_comp()(k, low->first))
    ? low
    : end();
}

//------------------------------  leaf_node::next_node()  ------------------------------//

template <class Key, class T, class Compare, class Allocator>
typename mbt_map<Key,T,Compare,Allocator>::leaf_node*
mbt_map<Key,T,Compare,Allocator>::leaf_node::
next_node()  // return next leaf_node, root_node if end
{
  if (this->is_root())
    return this;

  branch_node*   parent_np = this->parent_node();
  branch_value*  parent_ep = this->parent_element();

  if (parent_ep != parent_np->end())
    ++parent_ep;
  else
  {
    parent_np = this->parent_node()->next_node();
    if (parent_np->is_root())
      return leaf_cast(parent_np);
    parent_ep = parent_np->begin();
  }

  leaf_node* np = leaf_cast(parent_ep->first);
  np->parent_node(parent_np);
  np->parent_element(parent_ep);
  return np;
}

//------------------------------  branch_node::next_node()  ----------------------------//

template <class Key, class T, class Compare, class Allocator>
typename mbt_map<Key,T,Compare,Allocator>::branch_node*
mbt_map<Key,T,Compare,Allocator>::branch_node::
next_node()  // return next branch_node at current height, root_node if end
{
  if (this->is_root())
    return this;

  branch_node*   parent_np = this->parent_node();
  branch_value*  parent_ep = this->parent_element();

  if (parent_ep != parent_np->end())
    ++parent_ep;
  else
  {
    parent_np = this->parent_node()->next_node();
    if (parent_np->is_root())
      return parent_np;
    parent_ep = parent_np->begin();
  }

  branch_node* np = branch_cast(parent_ep->first);
  np->parent_node(parent_np);
  np->parent_element(parent_ep);
  return np;
}

//--------------------------  iterator::increment()  -----------------------------------//

template <class Key, class T, class Compare, class Allocator>
template <class VT>
void
mbt_map<Key,T,Compare,Allocator>::iterator_type<VT>::
increment()
{
  BOOST_ASSERT_MSG(m_element, "attempt to increment uninitialized iterator");
  BOOST_ASSERT_MSG(m_node, "attempt to increment end iterator");
  BOOST_ASSERT(m_node->is_leaf());
  BOOST_ASSERT(m_element >= m_node->begin());
  BOOST_ASSERT(m_element < m_node->end());

  if (++m_element != m_node->end())
    return;

  m_node = m_node->next_node();  // next leaf node, or root node if end

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

}  // btree
}  // boost

#ifdef BOOST_MSVC
#  pragma warning(pop)
#endif

#endif  // BOOST_MBT_MAP_HPP
