
#include <boost/config/warning_disable.hpp>
#include <boost/config.hpp>

#ifdef BOOST_MSVC
#  pragma warning(push)
#  pragma warning(disable: 4996)  // ... Function call with parameters that may be unsafe
#endif

#include <iostream>
#include <boost/detail/lightweight_test.hpp>
#include <boost/btree/mbt_map.hpp>
#include <boost/type_traits/is_same.hpp>
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
    map bt(64);

    std::pair<const archetype, long> v1(archetype(1), 1*100);
    archetype_count::clear();
    bt.insert(v1);
//    BOOST_TEST_EQ(archetype_count::copy_construct, 1);
//    BOOST_TEST_EQ(archetype_count::sum(), 1);
    //cout << '\n';
    //archetype_count::dump(cout);

    std::pair<const archetype, long> v3(archetype(3), 3*100);
    //archetype_count::clear();
    bt.insert(v3);
    //cout << '\n';
    //archetype_count::dump(cout);

    std::pair<const archetype, long> v2(archetype(2), 2*100);
    //archetype_count::clear();
    bt.insert(v2);
    cout << '\n';
    //archetype_count::dump(cout);
    //cout << endl;

    // insert enough elements to verify archetype asserts don't fire
    // on branch inserts and splits
    for (int i = 40; i > 3; --i)
    {
      std::pair<const archetype, long> x(archetype(i), i*100);
      bt.insert(x);
    }
    archetype_count::dump(cout);
    cout << endl;

  }
} // unnamed namespace

int cpp_main(int, char*[])
{
  archetype_test();

  typedef btree::mbt_map<int, long> map;

  cout << "type test" << endl;

  BOOST_TEST((is_same< map::key_type, int>::value));
  BOOST_TEST((is_same< map::mapped_type, long>::value));
  BOOST_TEST((is_same< map::value_type, std::pair<const int, long> >::value));
  BOOST_TEST((is_same<map::iterator::reference,
    std::pair<const int, long>& >::value));
  BOOST_TEST((is_same<map::const_iterator::reference,
    const std::pair<const int, long>& >::value));

  cout << "construction test" << endl;

  map bt(48);
  const map* const_bt = &bt;

  BOOST_TEST_EQ(bt.size(), 0U);
  BOOST_TEST(bt.empty());
  BOOST_TEST_EQ(const_bt->size(), 0U);
  BOOST_TEST(const_bt->empty());
  BOOST_TEST(bt.begin() == bt.end());
  BOOST_TEST(const_bt->begin() == const_bt->end());
  BOOST_TEST(bt.end() == const_bt->end());

  cout << "insertion test" << endl;

  std::pair<const int, long> v1(1, 1*100);
  bt.insert(v1);
  BOOST_TEST_EQ(bt.size(), 1U);

  bt.insert(std::make_pair(3, 3*100));
  BOOST_TEST_EQ(bt.size(), 2U);

  bt.insert(std::make_pair(2, 2*100));
  BOOST_TEST_EQ(bt.size(), 3U);

  std::pair<map::iterator, bool> result = bt.insert(v1);
  BOOST_TEST(!result.second);
  BOOST_TEST_EQ(bt.size(), 3U);

  cout << "iterator test" << endl;

  map::iterator it = bt.begin();
  BOOST_TEST_EQ(it->first, 1);
  BOOST_TEST_EQ(it->second, 100);

  cout << "const_iterator test" << endl;

  map::const_iterator const_it = bt.begin();
  BOOST_TEST_EQ(const_it->first, 1);
  BOOST_TEST_EQ(const_it->second, 100);

  cout << "iterator conversion test" << endl;

  it = bt.begin();
  const_it = bt.end();
  BOOST_TEST(const_it != it);
  const_it = it;
  BOOST_TEST(const_it == it);

  cout << "iteration test" << endl;

  it = bt.begin();
  BOOST_TEST_EQ(it->first, 1);
  BOOST_TEST_EQ(it->second, 100);
  ++it;
  BOOST_TEST_EQ(it->first, 2);
  BOOST_TEST_EQ(it->second, 200);
  ++it;
  BOOST_TEST_EQ(it->first, 3);
  BOOST_TEST_EQ(it->second, 300);
  ++it;
  BOOST_TEST(it == bt.end());

  cout << "insert more test" << endl;

  for (int i = 100; i > 3; --i)
    bt.insert(std::make_pair(i, i*100));
  BOOST_TEST_EQ(bt.size(), 100U);
  cout << "  complete, height() is " << bt.height() << '\n';

  cout << "forward iteration checksum test" << endl;

  int itr_checksum = 0, loop_checksum = 0;
  std::size_t loop_counter = 0;
  map::const_iterator itr;
  for (itr = bt.begin(); itr != bt.end(); ++itr)
  {
//    std::cout << "  " << itr->first << ", " << itr->second << std::endl;
    ++loop_counter;
    itr_checksum += itr->first;
    loop_checksum += loop_counter;
  }
  BOOST_TEST_EQ(loop_counter, bt.size());
  BOOST_TEST_EQ(itr_checksum, loop_checksum);

  cout << "backward iteration checksum test" << endl;
  itr_checksum = 0;
  for (itr = bt.end(); itr != bt.begin();)
  {
    --itr;
    itr_checksum += itr->first;
  }
  BOOST_TEST_EQ(itr_checksum, loop_checksum);

  cout << "lower_bound test" << endl;

  BOOST_TEST_EQ(bt.lower_bound(0)->first, 1);
  BOOST_TEST_EQ(bt.lower_bound(1)->first, 1);
  BOOST_TEST_EQ(bt.lower_bound(20)->first, 20);
  BOOST_TEST_EQ(bt.lower_bound(40)->first, 40);
  BOOST_TEST(bt.lower_bound(bt.size()+1)==bt.end());

  cout << "const lower_bound test" << endl;

  BOOST_TEST_EQ(const_bt->lower_bound(0)->first, 1);
  BOOST_TEST_EQ(const_bt->lower_bound(1)->first, 1);
  BOOST_TEST_EQ(const_bt->lower_bound(20)->first, 20);
  BOOST_TEST_EQ(const_bt->lower_bound(bt.size())->first, bt.size());
  BOOST_TEST(const_bt->lower_bound(bt.size()+1)==const_bt->end());

  cout << "find test" << endl;

  BOOST_TEST(bt.find(0)==bt.end());
  for (int i = 1; i <= int(bt.size()); ++i)
    BOOST_TEST_EQ(bt.find(i)->first, i);
  BOOST_TEST(bt.find(bt.size()+1)==bt.end());

  cout << "const find test" << endl;

  BOOST_TEST(const_bt->find(0)==const_bt->end());
  for (int i = 1; i <= int(bt.size()); ++i)
    BOOST_TEST_EQ(const_bt->find(i)->first, i);
  BOOST_TEST(const_bt->find(bt.size()+1)==const_bt->end());

  cout << "upper_bound test" << endl;

  BOOST_TEST_EQ(bt.upper_bound(0)->first, 1);
  BOOST_TEST_EQ(bt.upper_bound(1)->first, 2);
  BOOST_TEST_EQ(bt.upper_bound(20)->first, 21);
  BOOST_TEST(bt.upper_bound(bt.size())==bt.end());
  BOOST_TEST(bt.upper_bound(bt.size()+1)==bt.end());

  cout << "const upper_bound test" << endl;

  BOOST_TEST_EQ(const_bt->upper_bound(0)->first, 1);
  BOOST_TEST_EQ(const_bt->upper_bound(1)->first, 2);
  BOOST_TEST_EQ(const_bt->upper_bound(20)->first, 21);
  BOOST_TEST(const_bt->upper_bound(bt.size())==const_bt->end());
  BOOST_TEST(const_bt->upper_bound(bt.size()+1)==const_bt->end());

  cout << "equal_range test" << endl;

  std::pair<map::iterator, map::iterator> eq = bt.equal_range(0);
  BOOST_TEST(eq.first->first == 1);
  BOOST_TEST(eq.second->first == 1);
  eq = bt.equal_range(1);
  BOOST_TEST(eq.first->first == 1);
  BOOST_TEST(eq.second->first == 2);
  eq = bt.equal_range(20);
  BOOST_TEST(eq.first->first == 20);
  BOOST_TEST(eq.second->first == 21);
  eq = bt.equal_range(bt.size());
  BOOST_TEST(eq.first->first == bt.size());
  BOOST_TEST(eq.second == bt.end());
  eq = bt.equal_range(bt.size()+1);
  BOOST_TEST(eq.first == bt.end());
  BOOST_TEST(eq.second == bt.end());

  cout << "const equal_range test" << endl;

  std::pair<map::const_iterator, map::const_iterator> const_eq = const_bt->equal_range(0);
  BOOST_TEST(const_eq.first->first == 1);
  BOOST_TEST(const_eq.second->first == 1);
  const_eq = const_bt->equal_range(1);
  BOOST_TEST(const_eq.first->first == 1);
  BOOST_TEST(const_eq.second->first == 2);
  const_eq = const_bt->equal_range(20);
  BOOST_TEST(const_eq.first->first == 20);
  BOOST_TEST(const_eq.second->first == 21);
  const_eq = const_bt->equal_range(bt.size());
  BOOST_TEST(const_eq.first->first == bt.size());
  BOOST_TEST(const_eq.second == const_bt->end());
  const_eq = const_bt->equal_range(bt.size()+1);
  BOOST_TEST(const_eq.first == const_bt->end());
  BOOST_TEST(const_eq.second == const_bt->end());

  cout << "operator[] l-value test" << endl;

  map::size_type sz = bt.size();
  map::key_type key = 20;
  BOOST_TEST_EQ(bt.find(key)->second, key*100);
  bt[key] = key*1000;
  BOOST_TEST_EQ(bt.size(), sz);
  BOOST_TEST_EQ(bt.find(key)->second, key*1000);
  key = bt.size()+1;
  BOOST_TEST(bt.find(key) == bt.end());
  bt[key] = key*1000;
  BOOST_TEST_EQ(bt.size(), sz+1);
  BOOST_TEST_EQ(bt.find(key)->second, key*1000);

  cout << "erase by key test" << endl;

  map::size_type old_sz = bt.size();
  map::size_type ct = bt.erase(20);
  BOOST_TEST_EQ(ct, 1U);
  BOOST_TEST_EQ(bt.size(), old_sz-1);

//  for (int i = 1; i < 41; ++i)
//    BOOST_TEST_EQ(const_bt->find(i)->first, i);
//  BOOST_TEST(const_bt->find(41)==const_bt->end());


  return report_errors();
}

#ifdef BOOST_MSVC
#  pragma warning(pop)
#endif
