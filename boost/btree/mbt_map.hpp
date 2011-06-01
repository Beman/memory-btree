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

* new_leaf_node(), new_branch_note(), should use allocator!

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
    iterator                begin() noexcept;
    const_iterator          begin() const noexcept;
    iterator                end() noexcept;
    const_iterator          end() const noexcept;
    reverse_iterator        rbegin() noexcept;
    const_reverse_iterator  rbegin() const noexcept;
    reverse_iterator        rend() noexcept;
    const_reverse_iterator  rend() const noexcept;
    const_iterator          cbegin() const noexcept;
    const_iterator          cend() const noexcept;
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
    template <class P>
      std::pair<iterator, bool>
                            insert(P&& x);
    iterator                insert(const_iterator position, const value_type& x);
    template <class P>
      iterator              insert(const_iterator position, P&&);
    template <class InputIterator>
      void                  insert(InputIterator first, InputIterator last);
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
    friend class branch_node;
    friend class leaf_node;

    typedef std::pair<Key, T>      leaf_value;
    typedef std::pair<node*, Key>  branch_value;


    //----------------------------------------------------------------------------------//
    //                             private nested classes                               //
    //----------------------------------------------------------------------------------//

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
    };

    class node
    {
    public:
      uint16_t      _height;   // 0 for the leaf node
      uint16_t      _size;
      branch_node*  _parent_node;  // always points to a branch, except 0 for root node
      branch_value* _parent_element; // 0 for root node

      bool          is_leaf() const                 {return _height == 0;}
      bool          is_branch() const               {return _height != 0;}
      bool          is_root() const                 {return _parent_node == 0;}
      std::size_t   size() const                    {return _size;}
      branch_node*  parent_node() const             {return _parent_node;}
      branch_value* parent_element() const          {return _parent_element;}
      void          size(std::size_t n)             {_size = n;}
      void          parent_node(branch_node* p)     {_parent_node = p;}
      void          parent_element(branch_value* p) {_parent_element = p;}
    };

    class leaf_node : public node
    {
    public:
      leaf_value _values[1];

      leaf_value* begin()        {return _values;}
      leaf_value* end()          {return &_values[size()];}
    };

    class branch_node : public node
    {
    public:
      branch_value _values[1]; // end pseudo-element first is dereferenceable & valid

      branch_value* begin()      {return _values;}
      branch_value* end()        {return &_values[size()];}
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
         : m_element(0) {m_union.m_node = 0;}
 #     endif

      iterator_type(typename mbt_map::leaf_node* np,
                    typename mbt_map::leaf_value* ep)
        : m_element(ep) {m_union.m_node = np;}

    private:
      iterator_type(mbt_map* owner)  // construct end iterator
        : m_element(0) {m_union.m_owner = owner;}

      friend class boost::iterator_core_access;
      friend class mbt_map;

      union  // discriminated by m_element
      {
        typename mbt_map::leaf_node*          m_node;   // non-end iterator
        mbt_map*                              m_owner;  // end iterator
      }                                     m_union;
      typename mbt_map::leaf_value*    m_element;  // 0 for end iterator


      T& dereference() const
      {
        BOOST_ASSERT_MSG(m_union.m_node, "dereferencing uninitialized iterator");
        BOOST_ASSERT_MSG(m_element, "dereferencing end iterator");
        return *m_element;
      }

      bool equal(const iterator_type& rhs) const {return m_element == rhs.m_element;}

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
    size_type             m_size;                 // number of elements in container
    size_type             m_max_leaf_node_size;   // maximum number of elements in values
    size_type             m_max_branch_node_size; // maximum number of elements in values
    node*                 m_root;                 // invariant: there is always a root

    //----------------------------------------------------------------------------------//
    //                            private member functions                              //
    //----------------------------------------------------------------------------------//

    branch_value_compare   branch_comp() const {return m_branch_value_compare;}

    void m_init(std::size_t node_sz)
    {
      m_size = 0;
      m_max_leaf_node_size = node_sz / sizeof(leaf_value);
      m_max_branch_node_size = node_sz / sizeof(branch_value);
      m_root = new_leaf_node();
    }

    leaf_node* new_leaf_node()
    {
      std::size_t leaf_node_size = sizeof(leaf_node)
       + (m_max_leaf_node_size-1) * sizeof(leaf_value);
      leaf_node* np = reinterpret_cast<leaf_node*>(new char[leaf_node_size]);
 #  ifdef NDEBUG
      np->_height = 0;
      np->_size = 0;
      np->_parent = 0;
 #  else
      std::memset(np, 0, leaf_node_size);
 #  endif
     return np;
    }

    iterator m_special_lower_bound(const key_type& k) const;

  };  // class mbt_map

//--------------------------------------------------------------------------------------//
//                                  implementation                                      //
//--------------------------------------------------------------------------------------//

//-----------------------------  m_special_lower_bound()  ------------------------------//

template <class Key, class T, class Compare, class Allocator>
typename mbt_map<Key,T,Compare,Allocator>::iterator
mbt_map<Key,T,Compare,Allocator>::
m_special_lower_bound(const key_type& k) const
{
  branch_node* bp = static_cast<branch_node*>(m_root);

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
    branch_node* child_np = low->first;
    child_np->parent(bp);
    child_np->parent_element(low);

    bp = child_np;
  }

  //  search leaf
  leaf_node* lp = static_cast<leaf_node*>(bp);
  leaf_value* low
    = std::lower_bound(lp->begin(), lp->end(), k, value_comp());

  return iterator(lp, low);
}

//-----------------------------------  insert()  ---------------------------------------//

template <class Key, class T, class Compare, class Allocator>
std::pair<typename mbt_map<Key,T,Compare,Allocator>::iterator, bool>
mbt_map<Key,T,Compare,Allocator>::
insert(const value_type& x)
{
  iterator insert_point = m_special_lower_bound(x.first);

  bool unique = insert_point.m_element == insert_point.m_union.m_node->end()
         || key_comp()(x.first, insert_point->first)
         || key_comp()(insert_point->first, x.first);

  if (!unique)
    return std::pair<const_iterator, bool>(insert_point, false);


 return std::pair<const_iterator, bool>(insert_point, true);
}


}  // btree
}  // boost

#endif  // BOOST_MBT_MAP_HPP
