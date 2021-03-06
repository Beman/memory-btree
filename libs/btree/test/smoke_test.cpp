//  smoke_test.cpp  --------------------------------------------------------------------//

//  Copyright Beman Dawes 2010, 2011

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  This library is experimental and has not been accepted as a boost.org library

#include <boost/config/warning_disable.hpp>
#include <boost/config.hpp>

#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable: 4996)  // ... Function call with parameters that may be unsafe
#endif

#include <iostream>
#include <boost/detail/lightweight_test.hpp>
#include <boost/btree/mbt_map.hpp>
#include <boost/btree/mbt_set.hpp>
#include <map>
#include <set>
#include <boost/type_traits.hpp>
#include <boost/btree/detail/archetype.hpp>
#include <utility>

#include <boost/test/included/prg_exec_monitor.hpp>

using namespace boost;
using std::cout; using std::endl;

namespace
{
  class archetype_compare
  {
  public:
    bool operator()(const archetype& x, const archetype& y) const
    {
      return x.value() < y.value();
    }
  };

  void archetype_test()
  {
    cout << "archetype test" << endl;

    typedef btree::mbt_map<archetype, long, archetype_compare> map;

    cout << "  alignment_of<archetype> is " << boost::alignment_of<archetype>::value << endl;
    cout << "  alignment_of<map::value_type> is " << boost::alignment_of<map::value_type>::value << endl;

    {
      map empty_bt;
    }

    BOOST_TEST_EQ(archetype_count::sum(), 0);
    archetype_count::clear();

    {
      map bt(64);

      std::pair<const archetype, long> v1(archetype(1), 1*100);
      bt.insert(v1);
  //    BOOST_TEST_EQ(archetype_count::copy_construct, 1);
  //    BOOST_TEST_EQ(archetype_count::sum(), 1);
      cout << '\n';
      archetype_count::dump(cout);

      std::pair<const archetype, long> v3(archetype(3), 3*100);
      bt.insert(v3);
      cout << '\n';
      archetype_count::dump(cout);

      std::pair<const archetype, long> v2(archetype(2), 2*100);
      bt.insert(v2);
      cout << '\n';
      archetype_count::dump(cout);
      cout << endl;

      // insert enough elements to verify archetype asserts don't fire
      // on branch inserts and splits
      for (int i = 1000; i > 3; --i)
      {
        std::pair<const archetype, long> x(archetype(i*1973345679), i);
        bt.insert(x);
      }
    }
    BOOST_TEST_EQ(archetype_count::all_constructors(), archetype_count::destruct);
    archetype_count::dump(cout);
    cout << endl;
  }

  template <class BT>
  void insert_unique_test(BT& bt, const typename BT::value_type& v, true_type)
  {
    std::pair<typename BT::iterator, bool> result = bt.insert(v);
    BOOST_TEST(!result.second);
    BOOST_TEST_EQ(bt.size(), 3U);
  }

  template <class BT>
  void insert_unique_test(BT&, const typename BT::value_type&, false_type) {}

  template <class BT>
  void uniqueness_count_and_erase_test(BT& bt,
    const typename BT::value_type& v, std::size_t sz, true_type) {}

  template <class BT>
  void uniqueness_count_and_erase_test(BT& bt,
    const typename BT::value_type& v, std::size_t sz, false_type)
  {
    BOOST_TEST_EQ(bt.size(), sz+1);
    BOOST_TEST_EQ(bt.count(BT::key(v)), 2U);
    typename BT::const_iterator it = bt.find(BT::key(v));
    bt.erase(it);
  }

  template <class BT>
  void operator_sq_bracket_test(BT& bt, true_type, true_type)
  {
    cout << "operator[] l-value test" << endl;

    typename BT::size_type sz = bt.size();
    typename BT::key_type key = 20;
    BOOST_TEST_EQ(bt.find(key)->second, key);
    bt[key] = key*1000;
    BOOST_TEST_EQ(bt.size(), sz);
    BOOST_TEST_EQ(bt.find(key)->second, key*1000);
    key = bt.size()+1;
    BOOST_TEST(bt.find(key) == bt.end());
    bt[key] = key*1000;
    BOOST_TEST_EQ(bt.size(), sz+1);
    BOOST_TEST_EQ(bt.find(key)->second, key*1000);
    bt.erase(bt.find(key));  // return bt to original state
  }
  template <class BT>
  void operator_sq_bracket_test(BT& bt, false_type, true_type) {}
  template <class BT>
  void operator_sq_bracket_test(BT& bt, true_type, false_type) {}
  template <class BT>
  void operator_sq_bracket_test(BT& bt, false_type, false_type) {}

  //----------------------------------- test() -----------------------------------------//

  template <class BT, class STL, class IsUnique, class IsMapped>
  //  Requires: IsUnique, IsMapped, be boost::true_type or boost::false_type
  void test()
  {
    cout << "type test" << endl;

    BOOST_TEST((is_same<typename BT::key_type, typename STL::key_type>::value));
//    BOOST_TEST((is_same<typename BT::mapped_type, typename STL::mapped_type>::value));
    BOOST_TEST((is_same<typename BT::value_type, typename STL::value_type >::value));
    BOOST_TEST((is_same<typename BT::iterator::reference,
      typename STL::iterator::reference>::value));
    BOOST_TEST((is_same<typename BT::const_iterator::reference,
      typename STL::const_iterator::reference >::value));

    cout << "construction test" << endl;

    const std::size_t node_sz = 48;

    BT bt(node_sz);

    const BT* const_bt = &bt;

    BOOST_TEST_EQ(bt.size(), 0U);
    BOOST_TEST_EQ(bt.node_size(), node_sz);
    BOOST_TEST(bt.empty());
    BOOST_TEST_EQ(const_bt->size(), 0U);
    BOOST_TEST(const_bt->empty());
    BOOST_TEST(bt.begin() == bt.end());
    BOOST_TEST(const_bt->begin() == const_bt->end());
    BOOST_TEST(bt.end() == const_bt->end());

    cout << "insertion test" << endl;

    const typename BT::value_type v1(BT::make_value(1, 1));
    bt.insert(v1);
    BOOST_TEST_EQ(bt.size(), 1U);

    const typename BT::value_type v3(BT::make_value(3, 3));
    bt.insert(v3);
    BOOST_TEST_EQ(bt.size(), 2U);

    const typename BT::value_type v2(BT::make_value(2, 2));
    bt.insert(v2);
    BOOST_TEST_EQ(bt.size(), 3U);

    insert_unique_test<BT>(bt, v1, IsUnique());

    cout << "iterator test" << endl;

    typename BT::iterator it = bt.begin();
    BOOST_TEST_EQ(BT::key(*it), 1);
    BOOST_TEST_EQ(BT::mapped_value(*it), 1);

    cout << "const_iterator test" << endl;

    typename BT::const_iterator const_it = bt.begin();
    BOOST_TEST_EQ(BT::key(*const_it), 1);
    BOOST_TEST_EQ(BT::mapped_value(*const_it), 1);

    cout << "iterator conversion test" << endl;

    it = bt.begin();
    const_it = bt.end();
    BOOST_TEST(const_it != it);
    const_it = it;
    BOOST_TEST(const_it == it);

    cout << "iteration test" << endl;

    it = bt.begin();
    BOOST_TEST_EQ(BT::key(*it), 1);
    BOOST_TEST_EQ(BT::mapped_value(*it), 1);
    ++it;
    BOOST_TEST_EQ(BT::key(*it), 2);
    BOOST_TEST_EQ(BT::mapped_value(*it), 2);
    ++it;
    BOOST_TEST_EQ(BT::key(*it), 3);
    BOOST_TEST_EQ(BT::mapped_value(*it), 3);
    ++it;
    BOOST_TEST(it == bt.end());

    cout << "insert more test" << endl;

    for (int i = 100; i > 3; --i)
    {
      bt.insert(BT::make_value(i, i));
    }
    BOOST_TEST_EQ(bt.size(), 100U);
    cout << "  complete, height() is " << bt.height() << '\n';

    cout << "forward iteration checksum test" << endl;

    int itr_checksum = 0, loop_checksum = 0;
    std::size_t loop_counter = 0;
    typename BT::const_iterator itr;
    for (itr = bt.begin(); itr != bt.end(); ++itr)
    {
  //    std::cout << "  " << itr->first << ", " << itr->second << std::endl;
      ++loop_counter;
      itr_checksum += BT::key(*itr);
      loop_checksum += loop_counter;
    }
    BOOST_TEST_EQ(loop_counter, bt.size());
    BOOST_TEST_EQ(itr_checksum, loop_checksum);

    cout << "backward iteration checksum test" << endl;
    itr_checksum = 0;
    for (itr = bt.end(); itr != bt.begin();)
    {
      --itr;
      itr_checksum += BT::key(*itr);
    }
    BOOST_TEST_EQ(itr_checksum, loop_checksum);

    cout << "lower_bound test" << endl;

    BOOST_TEST_EQ(BT::key(*bt.lower_bound(0)), 1);
    BOOST_TEST_EQ(BT::key(*bt.lower_bound(1)), 1);
    BOOST_TEST_EQ(BT::key(*bt.lower_bound(20)), 20);
    BOOST_TEST_EQ(BT::key(*bt.lower_bound(40)), 40);
    BOOST_TEST(bt.lower_bound(bt.size()+1)==bt.end());

    cout << "const lower_bound test" << endl;

    BOOST_TEST_EQ(BT::key(*const_bt->lower_bound(0)), 1);
    BOOST_TEST_EQ(BT::key(*const_bt->lower_bound(1)), 1);
    BOOST_TEST_EQ(BT::key(*const_bt->lower_bound(20)), 20);
    BOOST_TEST_EQ(BT::key(*const_bt->lower_bound(bt.size())), typename BT::key_type(bt.size()));
    BOOST_TEST(const_bt->lower_bound(bt.size()+1)==const_bt->end());

    cout << "find test" << endl;

    BOOST_TEST(bt.find(0)==bt.end());
    for (int i = 1; i <= int(bt.size()); ++i)
      BOOST_TEST_EQ(BT::key(*bt.find(i)), i);
    BOOST_TEST(bt.find(bt.size()+1)==bt.end());

    cout << "const find test" << endl;

    BOOST_TEST(const_bt->find(0)==const_bt->end());
    for (int i = 1; i <= int(bt.size()); ++i)
      BOOST_TEST_EQ(BT::key(*const_bt->find(i)), i);
    BOOST_TEST(const_bt->find(bt.size()+1)==const_bt->end());

    cout << "upper_bound test" << endl;

    BOOST_TEST_EQ(BT::key(*bt.upper_bound(0)), 1);
    BOOST_TEST_EQ(BT::key(*bt.upper_bound(1)), 2);
    BOOST_TEST_EQ(BT::key(*bt.upper_bound(20)), 21);
    BOOST_TEST(bt.upper_bound(bt.size())==bt.end());
    BOOST_TEST(bt.upper_bound(bt.size()+1)==bt.end());

    cout << "const upper_bound test" << endl;

    BOOST_TEST_EQ(BT::key(*const_bt->upper_bound(0)), 1);
    BOOST_TEST_EQ(BT::key(*const_bt->upper_bound(1)), 2);
    BOOST_TEST_EQ(BT::key(*const_bt->upper_bound(20)), 21);
    BOOST_TEST(const_bt->upper_bound(bt.size())==const_bt->end());
    BOOST_TEST(const_bt->upper_bound(bt.size()+1)==const_bt->end());

    cout << "equal_range test" << endl;

    std::pair<typename BT::iterator, typename BT::iterator> eq = bt.equal_range(0);
    BOOST_TEST(BT::key(*eq.first) == 1);
    BOOST_TEST(BT::key(*eq.second) == 1);
    eq = bt.equal_range(1);
    BOOST_TEST(BT::key(*eq.first) == 1);
    BOOST_TEST(BT::key(*eq.second) == 2);
    eq = bt.equal_range(20);
    BOOST_TEST(BT::key(*eq.first) == 20);
    BOOST_TEST(BT::key(*eq.second) == 21);
    eq = bt.equal_range(bt.size());
    BOOST_TEST(BT::key(*eq.first) == typename BT::key_type(bt.size()));
    BOOST_TEST(eq.second == bt.end());
    eq = bt.equal_range(bt.size()+1);
    BOOST_TEST(eq.first == bt.end());
    BOOST_TEST(eq.second == bt.end());

    cout << "const equal_range test" << endl;

    std::pair<typename BT::const_iterator, typename BT::const_iterator> const_eq = const_bt->equal_range(0);
    BOOST_TEST(BT::key(*const_eq.first) == 1);
    BOOST_TEST(BT::key(*const_eq.second) == 1);
    const_eq = const_bt->equal_range(1);
    BOOST_TEST(BT::key(*const_eq.first) == 1);
    BOOST_TEST(BT::key(*const_eq.second) == 2);
    const_eq = const_bt->equal_range(20);
    BOOST_TEST(BT::key(*const_eq.first) == 20);
    BOOST_TEST(BT::key(*const_eq.second) == 21);
    const_eq = const_bt->equal_range(bt.size());
    BOOST_TEST(BT::key(*const_eq.first) == typename BT::key_type(bt.size()));
    BOOST_TEST(const_eq.second == const_bt->end());
    const_eq = const_bt->equal_range(bt.size()+1);
    BOOST_TEST(const_eq.first == const_bt->end());
    BOOST_TEST(const_eq.second == const_bt->end());

    cout << "unique/non-unique insert, count, and erase test" << endl;

    typename BT::size_type sz = bt.size();
    bt.insert(v1);

    uniqueness_count_and_erase_test(bt, v1, sz, IsUnique());
    BOOST_TEST_EQ(bt.size(), sz);  // test should have preserved size
    BOOST_TEST_EQ(bt.count(BT::key(v1)), 1U);

    operator_sq_bracket_test(bt, IsUnique(), IsMapped());
    BOOST_TEST_EQ(bt.size(), sz);  // test should have preserved size

    cout << "copy construction test" << endl;

    BT bt2(bt);
    BOOST_TEST_EQ(bt.size(), bt2.size());
    BOOST_TEST(bt == bt2);
    BOOST_TEST(!(bt != bt2));
    BOOST_TEST(!(bt < bt2));
    BOOST_TEST( bt <= bt2);
    BOOST_TEST(!(bt > bt2));
    BOOST_TEST(bt >= bt2);
    itr_checksum = 0;
    for (itr = bt2.begin(); itr != bt2.end(); ++itr)
    {
  //    std::cout << "  " << itr->first << ", " << itr->second << std::endl;
      itr_checksum += BT::key(*itr);
    }
    BOOST_TEST_EQ(itr_checksum, loop_checksum);

    cout << "relational test" << endl;

    bt2.erase(--bt2.end());
    BOOST_TEST_EQ(bt.size()-1, bt2.size());
    BOOST_TEST(bt != bt2);
    BOOST_TEST(!(bt == bt2));
    BOOST_TEST(!(bt < bt2));
    BOOST_TEST(!(bt <= bt2));
    BOOST_TEST(bt > bt2);
    BOOST_TEST(bt >= bt2);

    cout << "range construction test" << endl;

    BT bt3(bt.begin(), bt.end());
    BOOST_TEST_EQ(bt.size(), bt3.size());
    BOOST_TEST(bt == bt3);

    cout << "move construction test" << endl;

    BT bt4a(bt);
    BT bt4b(std::move(bt4a));
    BOOST_TEST_EQ(bt4a.size(), 0U);
    BOOST_TEST_EQ(bt4b.size(), bt.size());
    BOOST_TEST(bt4b == bt);

    cout << "copy assignment test" << endl;

    BT bt5;
    bt5 = bt;
    BOOST_TEST_EQ(bt.size(), bt5.size());
    BOOST_TEST(bt == bt5);

    cout << "move assignment test" << endl;

    BT bt6a(bt);
    BT bt6b;
    bt6b.insert(v1);
    bt6b = std::move(bt6a);
    BOOST_TEST_EQ(bt6a.size(), 1U);  // required by implementation rather than specs
    BOOST_TEST_EQ(bt6b.size(), bt.size());
    BOOST_TEST(bt6b == bt);

    cout << "range insert test" << endl;

    BT bt7;
    bt7.insert(bt.begin(), bt.end());
    BOOST_TEST_EQ(bt7.size(), bt.size());
    BOOST_TEST(bt7 == bt);

    cout << "erase test" << endl;

    typename BT::size_type old_sz = bt.size();
    typename BT::size_type ct = bt.erase(20);
    BOOST_TEST_EQ(ct, 1U);
    --old_sz;
    BOOST_TEST_EQ(bt.size(), old_sz);

    BOOST_TEST(bt.height() > 1);  // make sure test covers several branch levels
    for (int i = 1; i <= int(old_sz); ++i)
    {
  //    cout << i << endl;
      bt.erase(i);
    }
    //for (itr = bt.begin(); itr != bt.end(); ++itr)
    //{
    //  std::cout << "  " << itr->first << ", " << itr->second << std::endl;
    //}
    BOOST_TEST_EQ(bt.size(), 1U);
    BOOST_TEST_EQ(bt.height(), 0);

    cout << "clear test" << endl;

    bt.clear();
    BOOST_TEST_EQ(bt.size(), 0U);
    BOOST_TEST_EQ(bt.height(), 0);
  }

} // unnamed namespace

//------------------------------------ cpp_main() -------------------------------------//

int cpp_main(int, char*[])
{
  archetype_test();

  cout << "----------------- mbt_map test -----------------\n\n";
  test<btree::mbt_map<int, long>, std::map<int, long>, true_type, true_type>();

  cout << "\n----------------- mbt_multimap test -----------------\n\n";
  test<btree::mbt_multimap<int, long>, std::multimap<int, long>, false_type, true_type>();

  cout << "\n----------------- mbt_set test -----------------\n\n";
  test<btree::mbt_set<int>, std::set<int>, true_type, false_type>();

  cout << "\n----------------- mbt_multiset test -----------------\n\n";
  test< btree::mbt_multiset<int>, std::multiset<int>, false_type, false_type>();

  return report_errors();
}

#ifdef BOOST_MSVC
#  pragma warning(pop)
#endif
