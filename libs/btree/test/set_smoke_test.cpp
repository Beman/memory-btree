//  set_smoke_test.cpp  ----------------------------------------------------------------//

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
#include <boost/btree/mbt_set.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/btree/detail/archetype.hpp>
#include <boost/type_traits/alignment_of.hpp>
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

    typedef btree::mbt_set<archetype, long, archetype_compare> set;

    cout << "  alignment_of<archetype> is " << boost::alignment_of<archetype>::value << endl;
    cout << "  alignment_of<set::value_type> is " << boost::alignment_of<set::value_type>::value << endl;

    {
      set empty_bt;
    }

    BOOST_TEST_EQ(archetype_count::sum(), 0);
    archetype_count::clear();

    {
      set bt(64);

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
} // unnamed namespace

int cpp_main(int, char*[])
{
  archetype_test();

//  typedef btree::mbt_set<int, long> set;
//
//  cout << "type test" << endl;
//
//  BOOST_TEST((is_same< set::key_type, int>::value));
//  BOOST_TEST((is_same< set::setped_type, long>::value));
//  BOOST_TEST((is_same< set::value_type, std::pair<const int, long> >::value));
//  BOOST_TEST((is_same<set::iterator::reference,
//    std::pair<const int, long>& >::value));
//  BOOST_TEST((is_same<set::const_iterator::reference,
//    const std::pair<const int, long>& >::value));
//
//  cout << "construction test" << endl;
//
//  const std::size_t node_sz = 48;
//
//  set bt(node_sz);
//
//  const set* const_bt = &bt;
//
//  BOOST_TEST_EQ(bt.size(), 0U);
//  BOOST_TEST_EQ(bt.node_size(), node_sz);
//  BOOST_TEST(bt.empty());
//  BOOST_TEST_EQ(const_bt->size(), 0U);
//  BOOST_TEST(const_bt->empty());
//  BOOST_TEST(bt.begin() == bt.end());
//  BOOST_TEST(const_bt->begin() == const_bt->end());
//  BOOST_TEST(bt.end() == const_bt->end());
//
//  cout << "insertion test" << endl;
//
//  std::pair<const int, long> v1(1, 1*100);
//  bt.insert(v1);
//  BOOST_TEST_EQ(bt.size(), 1U);
//
//  bt.insert(std::make_pair(3, 3*100));
//  BOOST_TEST_EQ(bt.size(), 2U);
//
//  bt.insert(std::make_pair(2, 2*100));
//  BOOST_TEST_EQ(bt.size(), 3U);
//
//  std::pair<set::iterator, bool> result = bt.insert(v1);
//  BOOST_TEST(!result.second);
//  BOOST_TEST_EQ(bt.size(), 3U);
//
//  cout << "iterator test" << endl;
//
//  set::iterator it = bt.begin();
//  BOOST_TEST_EQ(it->first, 1);
//  BOOST_TEST_EQ(it->second, 100);
//
//  cout << "const_iterator test" << endl;
//
//  set::const_iterator const_it = bt.begin();
//  BOOST_TEST_EQ(const_it->first, 1);
//  BOOST_TEST_EQ(const_it->second, 100);
//
//  cout << "iterator conversion test" << endl;
//
//  it = bt.begin();
//  const_it = bt.end();
//  BOOST_TEST(const_it != it);
//  const_it = it;
//  BOOST_TEST(const_it == it);
//
//  cout << "iteration test" << endl;
//
//  it = bt.begin();
//  BOOST_TEST_EQ(it->first, 1);
//  BOOST_TEST_EQ(it->second, 100);
//  ++it;
//  BOOST_TEST_EQ(it->first, 2);
//  BOOST_TEST_EQ(it->second, 200);
//  ++it;
//  BOOST_TEST_EQ(it->first, 3);
//  BOOST_TEST_EQ(it->second, 300);
//  ++it;
//  BOOST_TEST(it == bt.end());
//
//  cout << "insert more test" << endl;
//
//  for (int i = 100; i > 3; --i)
//    bt.insert(std::make_pair(i, i*100));
//  BOOST_TEST_EQ(bt.size(), 100U);
//  cout << "  complete, height() is " << bt.height() << '\n';
//
//  cout << "forward iteration checksum test" << endl;
//
//  int itr_checksum = 0, loop_checksum = 0;
//  std::size_t loop_counter = 0;
//  set::const_iterator itr;
//  for (itr = bt.begin(); itr != bt.end(); ++itr)
//  {
////    std::cout << "  " << itr->first << ", " << itr->second << std::endl;
//    ++loop_counter;
//    itr_checksum += itr->first;
//    loop_checksum += loop_counter;
//  }
//  BOOST_TEST_EQ(loop_counter, bt.size());
//  BOOST_TEST_EQ(itr_checksum, loop_checksum);
//
//  cout << "backward iteration checksum test" << endl;
//  itr_checksum = 0;
//  for (itr = bt.end(); itr != bt.begin();)
//  {
//    --itr;
//    itr_checksum += itr->first;
//  }
//  BOOST_TEST_EQ(itr_checksum, loop_checksum);
//
//  cout << "lower_bound test" << endl;
//
//  BOOST_TEST_EQ(bt.lower_bound(0)->first, 1);
//  BOOST_TEST_EQ(bt.lower_bound(1)->first, 1);
//  BOOST_TEST_EQ(bt.lower_bound(20)->first, 20);
//  BOOST_TEST_EQ(bt.lower_bound(40)->first, 40);
//  BOOST_TEST(bt.lower_bound(bt.size()+1)==bt.end());
//
//  cout << "const lower_bound test" << endl;
//
//  BOOST_TEST_EQ(const_bt->lower_bound(0)->first, 1);
//  BOOST_TEST_EQ(const_bt->lower_bound(1)->first, 1);
//  BOOST_TEST_EQ(const_bt->lower_bound(20)->first, 20);
//  BOOST_TEST_EQ(const_bt->lower_bound(bt.size())->first, set::key_type(bt.size()));
//  BOOST_TEST(const_bt->lower_bound(bt.size()+1)==const_bt->end());
//
//  cout << "find test" << endl;
//
//  BOOST_TEST(bt.find(0)==bt.end());
//  for (int i = 1; i <= int(bt.size()); ++i)
//    BOOST_TEST_EQ(bt.find(i)->first, i);
//  BOOST_TEST(bt.find(bt.size()+1)==bt.end());
//
//  cout << "const find test" << endl;
//
//  BOOST_TEST(const_bt->find(0)==const_bt->end());
//  for (int i = 1; i <= int(bt.size()); ++i)
//    BOOST_TEST_EQ(const_bt->find(i)->first, i);
//  BOOST_TEST(const_bt->find(bt.size()+1)==const_bt->end());
//
//  cout << "upper_bound test" << endl;
//
//  BOOST_TEST_EQ(bt.upper_bound(0)->first, 1);
//  BOOST_TEST_EQ(bt.upper_bound(1)->first, 2);
//  BOOST_TEST_EQ(bt.upper_bound(20)->first, 21);
//  BOOST_TEST(bt.upper_bound(bt.size())==bt.end());
//  BOOST_TEST(bt.upper_bound(bt.size()+1)==bt.end());
//
//  cout << "const upper_bound test" << endl;
//
//  BOOST_TEST_EQ(const_bt->upper_bound(0)->first, 1);
//  BOOST_TEST_EQ(const_bt->upper_bound(1)->first, 2);
//  BOOST_TEST_EQ(const_bt->upper_bound(20)->first, 21);
//  BOOST_TEST(const_bt->upper_bound(bt.size())==const_bt->end());
//  BOOST_TEST(const_bt->upper_bound(bt.size()+1)==const_bt->end());
//
//  cout << "equal_range test" << endl;
//
//  std::pair<set::iterator, set::iterator> eq = bt.equal_range(0);
//  BOOST_TEST(eq.first->first == 1);
//  BOOST_TEST(eq.second->first == 1);
//  eq = bt.equal_range(1);
//  BOOST_TEST(eq.first->first == 1);
//  BOOST_TEST(eq.second->first == 2);
//  eq = bt.equal_range(20);
//  BOOST_TEST(eq.first->first == 20);
//  BOOST_TEST(eq.second->first == 21);
//  eq = bt.equal_range(bt.size());
//  BOOST_TEST(eq.first->first == set::key_type(bt.size()));
//  BOOST_TEST(eq.second == bt.end());
//  eq = bt.equal_range(bt.size()+1);
//  BOOST_TEST(eq.first == bt.end());
//  BOOST_TEST(eq.second == bt.end());
//
//  cout << "const equal_range test" << endl;
//
//  std::pair<set::const_iterator, set::const_iterator> const_eq = const_bt->equal_range(0);
//  BOOST_TEST(const_eq.first->first == 1);
//  BOOST_TEST(const_eq.second->first == 1);
//  const_eq = const_bt->equal_range(1);
//  BOOST_TEST(const_eq.first->first == 1);
//  BOOST_TEST(const_eq.second->first == 2);
//  const_eq = const_bt->equal_range(20);
//  BOOST_TEST(const_eq.first->first == 20);
//  BOOST_TEST(const_eq.second->first == 21);
//  const_eq = const_bt->equal_range(bt.size());
//  BOOST_TEST(const_eq.first->first == set::key_type(bt.size()));
//  BOOST_TEST(const_eq.second == const_bt->end());
//  const_eq = const_bt->equal_range(bt.size()+1);
//  BOOST_TEST(const_eq.first == const_bt->end());
//  BOOST_TEST(const_eq.second == const_bt->end());
//
//  cout << "operator[] l-value test" << endl;
//
//  set::size_type sz = bt.size();
//  set::key_type key = 20;
//  BOOST_TEST_EQ(bt.find(key)->second, key*100);
//  bt[key] = key*1000;
//  BOOST_TEST_EQ(bt.size(), sz);
//  BOOST_TEST_EQ(bt.find(key)->second, key*1000);
//  key = bt.size()+1;
//  loop_checksum += key;
//  BOOST_TEST(bt.find(key) == bt.end());
//  bt[key] = key*1000;
//  BOOST_TEST_EQ(bt.size(), sz+1);
//  BOOST_TEST_EQ(bt.find(key)->second, key*1000);
//
//  cout << "copy construction test" << endl;
//
//  set bt2(bt);
//  BOOST_TEST_EQ(bt.size(), bt2.size());
//  BOOST_TEST(bt == bt2);
//  BOOST_TEST(!(bt != bt2));
//  BOOST_TEST(!(bt < bt2));
//  BOOST_TEST( bt <= bt2);
//  BOOST_TEST(!(bt > bt2));
//  BOOST_TEST(bt >= bt2);
//  itr_checksum = 0;
//  for (itr = bt2.begin(); itr != bt2.end(); ++itr)
//  {
////    std::cout << "  " << itr->first << ", " << itr->second << std::endl;
//    itr_checksum += itr->first;
//  }
//  BOOST_TEST_EQ(itr_checksum, loop_checksum);
//
//  cout << "relational test" << endl;
//
//  bt2.erase(--bt2.end());
//  BOOST_TEST_EQ(bt.size()-1, bt2.size());
//  BOOST_TEST(bt != bt2);
//  BOOST_TEST(!(bt == bt2));
//  BOOST_TEST(!(bt < bt2));
//  BOOST_TEST(!(bt <= bt2));
//  BOOST_TEST(bt > bt2);
//  BOOST_TEST(bt >= bt2);
//
//  cout << "range construction test" << endl;
//
//  set bt3(bt.begin(), bt.end());
//  BOOST_TEST_EQ(bt.size(), bt3.size());
//  BOOST_TEST(bt == bt3);
// 
//  cout << "move construction test" << endl;
//
//  set bt4a(bt);
//  set bt4b(std::move(bt4a));
//  BOOST_TEST_EQ(bt4a.size(), 0U);
//  BOOST_TEST_EQ(bt4b.size(), bt.size());
//  BOOST_TEST(bt4b == bt);
//
//  cout << "copy assignment test" << endl;
//  
//  set bt5;
//  bt5 = bt;
//  BOOST_TEST_EQ(bt.size(), bt5.size());
//  BOOST_TEST(bt == bt5);
//
//  cout << "move assignment test" << endl;
//
//  set bt6a(bt);
//  set bt6b;
//  bt6b.insert(v1);
//  bt6b = std::move(bt6a);
//  BOOST_TEST_EQ(bt6a.size(), 1U);  // required by implementation rather than specs
//  BOOST_TEST_EQ(bt6b.size(), bt.size());
//  BOOST_TEST(bt6b == bt);
//
//  cout << "range insert test" << endl;
//  
//  set bt7;
//  bt7.insert(bt.begin(), bt.end());
//  BOOST_TEST_EQ(bt7.size(), bt.size());
//  BOOST_TEST(bt7 == bt);
//
//  cout << "erase test" << endl;
//
//  set::size_type old_sz = bt.size();
//  set::size_type ct = bt.erase(20);
//  BOOST_TEST_EQ(ct, 1U);
//  --old_sz;
//  BOOST_TEST_EQ(bt.size(), old_sz);
//
//  BOOST_TEST(bt.height() > 1);  // make sure test covers several branch levels
//  for (int i = 1; i <= int(old_sz); ++i)
//  {
////    cout << i << endl;
//    bt.erase(i);
//  }
//  //for (itr = bt.begin(); itr != bt.end(); ++itr)
//  //{
//  //  std::cout << "  " << itr->first << ", " << itr->second << std::endl;
//  //}
//  BOOST_TEST_EQ(bt.size(), 1U);
//  BOOST_TEST_EQ(bt.height(), 0);
//
//  cout << "clear test" << endl;
//
//  bt.clear();
//  BOOST_TEST_EQ(bt.size(), 0U);
//  BOOST_TEST_EQ(bt.height(), 0);
//
  return report_errors();
}

#ifdef BOOST_MSVC
#  pragma warning(pop)
#endif
