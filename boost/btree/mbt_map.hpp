#ifndef BOOST_MBT_MAP_HPP
#define BOOST_MBT_MAP_HPP

#include <cstddef>
#include <utility>
#include <memory>
#include <iterator>
#include <algorithm>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/assert.hpp>
#include <cstring>  // for memset

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
    explicit mbt_map(size_type   node_sz = default_node_size,
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
    allocator_type get_allocator() const noexcept;

    // iterators:
    iterator                begin() noexcept           {return m_begin();}
    const_iterator          begin() const noexcept     {return m_begin();}
    iterator                end() noexcept             {return iterator(this);}
    const_iterator          end() const noexcept       {return const_iterator(this);}
    reverse_iterator        rbegin() noexcept;
    const_reverse_iterator  rbegin() const noexcept;
    reverse_iterator        rend() noexcept;
    const_reverse_iterator  rend() const noexcept;
    const_iterator          cbegin() const noexcept    {return m_begin();}
    const_iterator          cend() const noexcept      {return const_iterator(this);}
    const_reverse_iterator  crbegin() const noexcept;
    const_reverse_iterator  crend() const noexcept;

    // capacity:
    bool                    empty() const noexcept {return m_size == 0;}
    size_type               size() const noexcept  {return m_size;}
    size_type               max_size() const noexcept;

    // 23.4.4.3, element access:
    T&                      operator[](const key_type& x);
    T&                      operator[](key_type&& x);
    T&                      at(const key_type& x);
    const T&                at(const key_type& x) const;

    // 23.4.4.4, modifiers:
    template <class... Args>
      std::pair<iterator, bool>
                            emplace(Args&&... args);
    template <class... Args>
      iterator              emplace_hint(const_iterator position, Args&&... args);

    std::pair<iterator, bool>
                            insert(const value_type& x);
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
    void                    clear() noexcept;

    // observers:
    key_compare             key_comp() const   {return m_key_compare;}
    value_compare           value_comp() const {return m_value_compare;}

    // 23.4.4.5, map operations:
    iterator                find(const key_type& x);
    const_iterator          find(const key_type& x) const;
    size_type               count(const key_type& x) const;
    iterator                lower_bound(const key_type& x);
    const_iterator          lower_bound(const key_type& x) const;
    iterator                upper_bound(const key_type& x);
    const_iterator          upper_bound(const key_type& x) const;
    std::pair<iterator,iterator>
                            equal_range(const key_type& x);
    std::pair<const_iterator, const_iterator>
                            equal_range(const key_type& x) const;

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
      node*           _parent_node;     // 0 for the root node
      union
      {
        branch_value* _parent_element;  // non-root node
        mbt_map*      _owner;           // root node
      };
      union
      {
        leaf_value    _leaf_values[1];     // actual size determined by constructor
        branch_value  _branch_values[1];   // ditto
      };

      uint16_t      height() const                  {return _height;}
      bool          is_leaf() const                 {return _height == 0;}
      bool          is_branch() const               {return _height != 0;}
      bool          is_root() const                 {return _parent_node == 0;}
      std::size_t   size() const                    {return _size;}
      node*         parent_node() const             {return _parent_node;}
      branch_value* parent_element() const          {return _parent_element;}
      mbt_map*      owner() const                   {return _owner;}

      void          height(uint16_t h)              {_height = h;}
      void          size(std::size_t n)             {_size = n;}
      void          parent_node(node* p)            {_parent_node = p;}
      void          parent_element(branch_value* p) {_parent_element = p;}
      void          owner(mbt_map* o)               {_owner = o;}

      leaf_value*   leaf_begin()                    {return _leaf_values;}
      leaf_value*   leaf_end()                      {return _leaf_values + _size;}

      branch_value* branch_begin()                  {return _branch_values;}
      branch_value* branch_end()                    {return _branch_values + _size;}
      // pseudo-element branch_end()->first is valid; b-tree branches have size() + 1
      // child pointers - see your favorite computer science textbook.

      node*         next_node();  // returns next node at same height; root node if end
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

      iterator_type(typename mbt_map::node* np,
                    typename mbt_map::leaf_value* ep)
        : m_node(np), m_element(ep) {}

      template <class VU>
      iterator_type(iterator_type<VU> const& other)
        : m_node(other.m_node), m_element(other.m_element) {}

    private:
      iterator_type(mbt_map* owner)  // construct end iterator
        : m_node(0), m_owner(owner) {}

      friend class boost::iterator_core_access;
      friend class mbt_map;

      typename mbt_map::node*        m_node;      // 0 indicates end iterator

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

    iterator               m_begin() noexcept;

    branch_value_compare   branch_comp() const {return m_branch_value_compare;}

    void m_init(std::size_t node_sz)
    {
      m_size = 0;
      m_max_leaf_size = node_sz / sizeof(leaf_value);
      m_max_branch_size = node_sz / sizeof(branch_value);
      m_root = m_new_node(0U);
      m_root->owner(this);
    }

    union node_values
    {
      leaf_value    _leaf_values[1];     // actual size determined by tree constructor
      branch_value  _branch_values[1];   // ditto
    };

    node* m_new_node(uint16_t height_);
    void  m_new_root();

    iterator m_special_lower_bound(const key_type& k) const;

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
typename mbt_map<Key,T,Compare,Allocator>::node*
mbt_map<Key,T,Compare,Allocator>::
m_new_node(uint16_t height_)
{
  std::size_t node_size = sizeof(node) - sizeof(node_values) +
   (height_ ? m_max_branch_size * sizeof(branch_value)
            : m_max_leaf_size * sizeof(leaf_value));

  node* np = reinterpret_cast<node*>(new char[node_size]);
#ifdef NDEBUG
  np->height(height_);
  np->size(0);
  np->parent_node(0);
  np->parent_element(0);
#else
  std::memset(np, 0, node_size);
#endif
  return np;
}

//----------------------------------  m_begin()  ---------------------------------------//

template <class Key, class T, class Compare, class Allocator>
typename mbt_map<Key,T,Compare,Allocator>::iterator
mbt_map<Key,T,Compare,Allocator>::
m_begin() noexcept
{
  if (empty())
    return end();

  node* np = m_root;

  // work down the tree until a leaf is reached
  while (np->is_branch())
  {
    // create the child->parent list
    node* child_np = np->branch_begin()->first;
    child_np->parent_node(np);
    child_np->parent_element(np->branch_begin());
    np = child_np;
  }

  return iterator(np, np->leaf_begin());
}

//-----------------------------  m_special_lower_bound()  ------------------------------//

template <class Key, class T, class Compare, class Allocator>
typename mbt_map<Key,T,Compare,Allocator>::iterator
mbt_map<Key,T,Compare,Allocator>::
m_special_lower_bound(const key_type& k) const
{
  node* np = m_root;

  // search branches down the tree until a leaf is reached
  while (np->is_branch())
  {
    branch_value* low
      = std::lower_bound(np->branch_begin(), np->branch_end(), k, branch_comp());

    if ( /*(header().flags() & btree::flags::unique)
      &&*/ low != np->branch_end()
      && !key_comp()(k, low->second)) // if k isn't less that low key, low is equal
      ++low;                         // and so must be incremented; this follows from
                                     // the branch node invariant for unique containers

    // create the child->parent list
    node* child = low->first;
    child->parent_node(np);
    child->parent_element(low);

    np = child;
  }

  //  search leaf
  leaf_value* low
    = std::lower_bound(np->leaf_begin(), np->leaf_end(), k, value_comp());

  return iterator(np, low);
}

//----------------------------------- m_new_root() -------------------------------------//

template <class Key, class T, class Compare, class Allocator>
void
mbt_map<Key,T,Compare,Allocator>::
m_new_root()
{
  // create a new root containing only the end pseudo-element
  node* old_root = m_root;
  m_root = m_new_node(old_root->height()+1);
  m_root->branch_begin()->first = old_root;
  m_root->size(0);  // the end pseudo-element doesn't count as an element
  m_root->parent_node(0);
  m_root->owner(this);
  old_root->parent_node(m_root);
  old_root->parent_element(m_root->branch_begin());
}

//-----------------------------------  insert()  ---------------------------------------//

template <class Key, class T, class Compare, class Allocator>
std::pair<typename mbt_map<Key,T,Compare,Allocator>::iterator, bool>
mbt_map<Key,T,Compare,Allocator>::
insert(const value_type& x)
{
  iterator insert_point = m_special_lower_bound(x.first);

  std::cout << "***" << (insert_point.m_element == insert_point.m_node->leaf_end())
  << (key_comp()(x.first, insert_point->first))
  << (key_comp()(insert_point->first, x.first)) << std::endl;

  bool unique = insert_point.m_element == insert_point.m_node->leaf_end()
         || key_comp()(x.first, insert_point->first)
         || key_comp()(insert_point->first, x.first);

  if (!unique)
    return std::pair<iterator, bool>(insert_point, false);

  //  do the insert

  node*        np = insert_point.m_node;
  leaf_value*  insert_begin = insert_point.m_element;
  node*        np2 = 0;

  BOOST_ASSERT_MSG(np->is_leaf(), "internal error");
  BOOST_ASSERT_MSG(np->size() <= m_max_leaf_size, "internal error");

  if (np->size() == m_max_leaf_size)  // if no room on node, node must be split
  {
    if (np->is_root()) // splitting the root?
      m_new_root();  // create a new root

    np2 = m_new_node(np->height());  // create the new node

//    // ck pack conditions now, since leaf seq list update may chg header().last_node_id()
//    if (m_ok_to_pack
//        && (insert_begin != np->leaf().end() || np->node_id() != header().last_node_id()))
//      m_ok_to_pack = false;  // conditions for pack optimization not met

//    // apply pack optimization if applicable
//    if (m_ok_to_pack)  // have all inserts been ordered and no erases occurred?
//    {
//      // pack optimization: instead of splitting np, just put value alone on np2
//      m_memcpy_value(&*np2->leaf().begin(), &key_, key_size, &mapped_value_, mapped_size);  // insert value
//      np2->size(value_size);
//      BOOST_ASSERT(np->parent()->node_id() == np->parent_node_id()); // max_cache_size logic OK?
//      m_branch_insert(np->parent(), np->parent_element(),
//        key(*np2->leaf().begin()), np2->node_id());
//      ++m_size;
//      return const_iterator(np2, np2->leaf().begin());
//    }

    // split node np by moving half the elements to node np2
    np2->size(np->size() / 2);  // round down to minimize move size
    np->size(np->size() - np2->size());
    std::move(np->leaf_begin() + np->size(), np->leaf_end(), np2->leaf_begin());

    // TODO: if the insert point will fall on the new node, it would be faster to
    // copy the portion before the insert point, copy the value being inserted, and
    // finally copy the portion after the insert point. However, that's a fair amount of
    // additional code for something that only happens on half of all leaf splits on average.

    // adjust np and insert_begin if they now fall on the new node due to the split
    if (np->leaf_end() <= insert_begin)
    {
      insert_begin = np2->leaf_begin() + (insert_begin - np->leaf_end());
      np = np2;
    }
  }

  BOOST_ASSERT(insert_begin >= np->leaf_begin());
  BOOST_ASSERT(insert_begin <= np->leaf_end());

  // make room for insert
  std::move_backward(insert_begin, np->leaf_end(), np->leaf_end()+1);

  //  insert x at insert_begin
  *insert_begin = x;
  ++np->_size;
  ++m_size;

//  // if there is a new node, its initial key and node_id are inserted into parent
//  if (np2)
//  {
//    m_branch_insert(insert_iter.m_node->parent(),
//      insert_iter.m_node->parent_element(),
//      key(*np2->leaf().begin()), np2->node_id());
//  }

//std::cout << "***insert done" << std::endl;
  return std::pair<iterator, bool>(iterator(np, insert_begin), true);
}

//------------------------------  node::next_node()  -----------------------------------//

template <class Key, class T, class Compare, class Allocator>
typename mbt_map<Key,T,Compare,Allocator>::node*
mbt_map<Key,T,Compare,Allocator>::node::
next_node()  // return next node at current height, root_node if end
{
  if (is_root())
    return this;

  node*          parent_np = parent_node();
  branch_value*  parent_ep = parent_element();

  if (parent_ep != parent_np->branch_end())
    ++parent_ep;
  else
  {
    parent_np = parent_node()->next_node();
    if (parent_np->is_root())
      return parent_np;
    parent_ep = parent_np->branch_begin();
  }

  node* np = parent_ep->first;
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
  BOOST_ASSERT(m_element >= m_node->leaf_begin());
  BOOST_ASSERT(m_element < m_node->leaf_end());

  if (++m_element != m_node->leaf_end())
    return;

  m_node = m_node->next_node();  // next leaf node, or root node if end

  if (!m_node->is_root())
  {
    m_element = m_node->leaf_begin();
    BOOST_ASSERT(m_element != m_node->leaf_end());
  }
  else // end() reached
  {
    m_owner = m_node->owner();
    m_node = 0;
  }
}

}  // btree
}  // boost

#endif  // BOOST_MBT_MAP_HPP
