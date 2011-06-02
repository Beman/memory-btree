#include <iostream>
#include <boost/config.hpp>
#include <boost/detail/lightweight_main.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/btree/mbt_map.hpp>
#include <boost/type_traits/is_same.hpp>
#include <utility>

using namespace boost;
using std::cout; using std::endl;

int cpp_main(int, char*[])
{
  typedef btree::mbt_map<int, long> map;

  BOOST_TEST((is_same< map::key_type, int>::value));
  BOOST_TEST((is_same< map::mapped_type, long>::value));
  BOOST_TEST((is_same< map::value_type, std::pair<const int, long> >::value));
  BOOST_TEST((is_same<map::iterator::reference,
    std::pair<const int, long>& >::value));
  BOOST_TEST((is_same<map::const_iterator::reference,
    const std::pair<const int, long>& >::value));

  cout << "construction test" << endl;

  map bt;

  BOOST_TEST_EQ(bt.size(), 0U);

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

  return report_errors();
}
