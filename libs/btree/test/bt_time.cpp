//  bt_time.cpp  -----------------------------------------------------------------------//

//  Copyright Beman Dawes 1994, 2010, 2011

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  This library is experimental and has not been accepted as a boost.org library

#define BOOST_NO_CONSTEXPR

#include <boost/btree/mbt_map.hpp>
#include <boost/btree/detail/config.hpp>
#include <boost/random.hpp>
#include <boost/btree/support/timer.hpp>
#include <boost/btree/support/random_string.hpp>
#include <boost/btree/support/indirect_less.hpp>

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>  // for atol()
#include <map>

#include <boost/test/included/prg_exec_monitor.hpp>

using namespace boost;
using std::cout;
using std::cerr;
using std::endl;

namespace
{
  std::string command_args;
  long n;
  long initial_n;
  long seed = 1;
  long lg = 0;
  int node_sz = boost::btree::default_node_size;
  bool do_create (true);
  bool do_preload (false);
  bool do_insert (true);
  bool do_pack (false);
  bool do_find (true);
  bool do_iterate (true);
  bool do_erase (true);
  bool verbose (false);
  bool stl_tests (false);
  bool ratio_btree_to_stl(true);
  bool html (false);
  const int places = 2;

  typedef std::vector<char> char_vector_type;

  btree::times_t insert_tm;
  btree::times_t find_tm;
  btree::times_t iterate_tm;
  btree::times_t erase_tm;
  const long double sec = 1000000.0L;

  double ratio_of(btree::microsecond_t btree, btree::microsecond_t stl)
  {
    return ratio_btree_to_stl ? (btree*1.0L)/(stl*1.0L) : (stl*1.0L)/(btree*1.0L);
  }

  const char* ratio_type()
  {
    return ratio_btree_to_stl ? "ratio btree/stl" : "ratio stl/btree";
  }

  class indirect_factory
  {
  public:
    indirect_factory(char_vector_type& chars) : _chars(chars), _index(0) {}

    void seed(char_vector_type::size_type) { _index = 0; }

   char* operator()()
    {
      char* p = &_chars[_index];
      _index += std::strlen(p) + 1;
      return p;
    }
  private:
    char_vector_type& _chars;
    char_vector_type::size_type _index;
  };

  template <class BT, class RNG, class KeyGen>
  void test(BT& bt, RNG& rng, KeyGen& key)
  {
    typename BT::key_compare key_compare = bt.key_comp();
    btree::run_timer t(3);
    {

      if (do_insert)
      {
        cout << "\ninserting " << n << " btree elements..." << endl;
        rng.seed(seed);
        t.start();
        for (long i = 1; i <= n; ++i)
        {
          typename BT::value_type element(key(), i);
          if (lg && i % lg == 0)
            std::cout << "  insert " << i << " key = " << element.first << std::endl;
          bt.insert(element);
        }
        insert_tm = t.stop();
        cout << "  inserts complete" << endl;
        t.report();
      }

//      if (do_pack)
//      {
//        cout << "\npacking btree..." << endl;
//        bt.close();
//        fs::remove(path_org);
//        fs::rename(path, path_org);
//        t.start();
//        BT bt_old(path_org);
//        bt_old.max_cache_size(cache_sz);
//        BT bt_new(path, btree::flags::truncate, node_sz);
//        bt_new.max_cache_size(cache_sz);
//        for (typename BT::iterator it = bt_old.begin(); it != bt_old.end(); ++it)
//        {
//          bt_new.emplace(it->key(), it->mapped_value());
//        }
//        cout << "  bt_old.size() " << bt_old.size() << std::endl;
//        cout << "  bt_new.size() " << bt_new.size() << std::endl;
//        BOOST_ASSERT(bt_new.size() == bt_old.size());
//        bt_old.close();
//        bt_new.close();
//        t.report();
//        cout << "  " << path_org << " file size: " << fs::file_size(path_org) << '\n';
//        cout << "  " << path << "     file size: " << fs::file_size(path) << '\n';
//        bt.open(path, btree::flags::read_write);
//        bt.max_cache_size(cache_sz);
//      }

      if (do_iterate)
      {
        cout << "\niterating over " << bt.size() << " btree elements..." << endl;
        unsigned long count = 0;
        typename BT::key_type prior_key;
        t.start();
        for (typename BT::const_iterator itr = bt.begin();
          itr != bt.end();
          ++itr)
        {
          if (count && !key_compare(prior_key, itr->first))
            throw std::runtime_error("btree iteration sequence error");
          ++count;
          prior_key = itr->first;
        }
        iterate_tm = t.stop();
        cout << "  iteration complete" << endl;
        t.report();
        if (count != bt.size())
          throw std::runtime_error("btree iteration count error");
      }

      if (do_find)
      {
        cout << "\nfinding " << n << " btree elements..." << endl;
        rng.seed(seed);
        typename BT::const_iterator itr;
        t.start();
        for (long i = 1; i <= n; ++i)
        {
          typename BT::key_type k (key());
          if (lg && i % lg == 0)
            std::cout << "  find " << i << " key = " << k << std::endl;
          itr = bt.find(k);
#       if !defined(NDEBUG)
          if (itr == bt.end())
            throw std::runtime_error("btree find() returned end()");
          if (itr->first != k)
          {
            cout << "*** first is " << itr->first << ", k is " << k << endl;
            throw std::runtime_error("btree find() returned wrong iterator");
          }
#       endif
        }
        find_tm = t.stop();
        cout << "  finds complete" << endl;
        t.report();
      }

//      if (verbose)
//      {
//        bt.flush();
//        cout << '\n' << bt << endl;
//        cout << bt.manager() << endl;
//      }

      if (do_erase)
      {
        cout << "\nerasing " << n << " btree elements..." << endl;
        rng.seed(seed);
        t.start();
        for (long i = 1; i <= n; ++i)
        {
          if (lg && i % lg == 0)
            std::cout << i << std::endl;
          //long k = key();
          //if (i >= n - 5)
          //{
          //  std::cout << i << ' ' << k << ' ' << bt.size() << std::endl;
          //  std::cout << "erase(k) returns " << bt.erase(k) << std::endl;
          //  std::cout << "and size() returns " << bt.size() << std::endl;
          //}
          //else
          //  bt.erase(k);
          bt.erase(key());
        }
        erase_tm = t.stop();
        t.report();
      }

      cout << "B-tree timing complete" << endl;

    }

    typedef std::map<typename BT::key_type, long,  typename BT::key_compare>  stl_type;
    stl_type stl;

    if (stl_tests)
    {
      cout << "\ninserting " << n << " std::map elements..." << endl;
      rng.seed(seed);
      btree::times_t this_tm;
      t.start();
      for (long i = 1; i <= n; ++i)
      {
        typename stl_type::value_type element(key(), i);
        if (lg && i % lg == 0)
          std::cout << "  insert " << i << " key = " << element.first << std::endl;
        stl.insert(element);
      }
      this_tm = t.stop();
      cout << "  inserts complete" << endl;
      t.report();
      if (html)
      {
        cerr << "<tr>\n  <td><code>" << command_args << "</code></td>\n";
        cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);
        cerr.precision(places);
        if (this_tm.wall)
        {
          double ratio = ratio_of(insert_tm.wall, this_tm.wall);
          if (ratio < 1.0)
            cerr << "  <td align=\"right\" bgcolor=\"#99FF66\">"
                 << insert_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n";
          else
            cerr << "  <td align=\"right\">"
                 << insert_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n";
        }
        else
          cerr << "  <td align=\"right\">N/A</td>\n";
      }
      if (insert_tm.wall && this_tm.wall)
        cout << "  " << ratio_type() << " wall clock time: "
             << ratio_of(insert_tm.wall, this_tm.wall) << '\n';
      //if (verbose && this_tm.system + this_tm.user)
      //  cout << "  " << ratio_type() << " cpu time: "
      //       << ((insert_tm.system + insert_tm.user) * 1.0)
      //          / (this_tm.system + this_tm.user) << '\n';

      cout << "\niterating over " << stl.size() << " stl elements..." << endl;
      unsigned long count = 0;
      typename BT::key_type prior_key;
      t.start();
      for (typename stl_type::const_iterator itr = stl.begin();
        itr != stl.end();
        ++itr)
      {
        if (count && !key_compare(prior_key, itr->first))
          throw std::runtime_error("stl iteration sequence error");
        ++count;
        prior_key = itr->first;
      }
      this_tm = t.stop();
      cout << "  iteration complete" << endl;
      t.report();
      if (html)
      {
        if (this_tm.wall)
        {
          double ratio = ratio_of(iterate_tm.wall, this_tm.wall);
          if (ratio < 1.0)
            cerr << "  <td align=\"right\" bgcolor=\"#99FF66\">"
                 << iterate_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n";
          else
            cerr << "  <td align=\"right\">"
                 << iterate_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n";
        }
        else
          cerr << "  <td align=\"right\">N/A</td>\n";
      }
      if (iterate_tm.wall && this_tm.wall)
        cout << "  " << ratio_type() << " wall clock time: "
             << ratio_of(iterate_tm.wall, this_tm.wall) << '\n';
      //if (verbose && this_tm.system + this_tm.user)
      //  cout << "  " << ratio_type() << " cpu time: "
      //       << ((iterate_tm.system + iterate_tm.user) * 1.0)
      //          / (this_tm.system + this_tm.user) << '\n';
      if (count != stl.size())
        throw std::runtime_error("stl iteration count error");

      cout << "\nfinding " << n << " std::map elements..." << endl;
      typename stl_type::const_iterator itr;
      typename BT::key_type k;
      rng.seed(seed);
      t.start();
      for (long i = 1; i <= n; ++i)
      {
        k = key();
        if (lg && i % lg == 0)
          std::cout << "  find " << i << " key = " << k << std::endl;
        itr = stl.find(k);
#       if !defined(NDEBUG)
          if (itr == stl.end())
            throw std::runtime_error("stl find() returned end()");
          if (itr->first != k)
          {
            cout << "*** first is " << itr->first << ", k is " << k << endl;
            throw std::runtime_error("stl find() returned wrong iterator");
          }
#       endif
      }
      this_tm = t.stop();
      cout << "  finds complete" << endl;
      t.report();
      if (html)
      {
        if (this_tm.wall)
        {
          double ratio = ratio_of(find_tm.wall, this_tm.wall);
          if (ratio < 1.0)
            cerr << "  <td align=\"right\" bgcolor=\"#99FF66\">"
                 << find_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n";
          else
            cerr << "  <td align=\"right\">"
                 << find_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n";
        }
        else
          cerr << "  <td align=\"right\">N/A</td>\n";
      }
      if (find_tm.wall && this_tm.wall)
        cout << "  " << ratio_type() << " wall clock time: "
             << ratio_of(find_tm.wall, this_tm.wall) << '\n';
      //if (verbose && this_tm.system + this_tm.user)
      //  cout << "  " << ratio_type() << " cpu time: "
      //       << ((find_tm.system + find_tm.user) * 1.0)
      //          / (this_tm.system + this_tm.user) << '\n';

      cout << "\nerasing " << n << " std::map elements..." << endl;
      rng.seed(seed);
      t.start();
      for (long i = 1; i <= n; ++i)
      {
        if (lg && i % lg == 0)
          std::cout << i << std::endl;
        stl.erase(key());
      }
      this_tm = t.stop();
      t.report();
      if (html)
      {
        if (this_tm.wall)
        {
          double ratio = ratio_of(erase_tm.wall, this_tm.wall);
          if (ratio < 1.0)
            cerr << "  <td align=\"right\" bgcolor=\"#99FF66\">"
                 << erase_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n</tr>\n";
          else
            cerr << "  <td align=\"right\">"
                 << erase_tm.wall / sec << " sec<br>"
                 << ratio << " ratio</td>\n</tr>\n";
        }
        else
          cerr << "  <td align=\"right\">N/A</td>\n</tr>  <td align=\"right\">N/A</td>\n</tr>\n";
      }
      if (find_tm.wall && this_tm.wall)
        cout << "  " << ratio_type() << " wall clock time: "
             << ratio_of(erase_tm.wall, this_tm.wall) << '\n';
//      if (verbose && this_tm.system + this_tm.user)
//        cout << "  " << ratio_type() << " cpu time: "
//             << ((erase_tm.system + erase_tm.user) * 1.0)
//                / (this_tm.system + this_tm.user) << '\n';
      cout << "STL timing complete" << endl;
    }
  }

}

//-------------------------------------- main()  ---------------------------------------//

int cpp_main(int argc, char * argv[])
{
  for (int a = 0; a < argc; ++a)
  {
    command_args += argv[a];
    if (a != argc-1)
      command_args += ' ';
  }

  cout << command_args << '\n';;

  if (argc >=2)
    n = std::atol(argv[1]);

  for (; argc > 2; ++argv, --argc)
  {
    if ( std::strncmp( argv[2]+1, "xe", 2 )==0 )
      do_erase = false;
    else if ( std::strncmp( argv[2]+1, "xf", 2 )==0 )
      do_find = false;
    else if ( std::strncmp( argv[2]+1, "xw", 2 )==0 )
      do_iterate = false;
    else if ( std::strncmp( argv[2]+1, "xc", 2 )==0 )
      do_create = false;
    else if ( std::strncmp( argv[2]+1, "xi", 2 )==0 )
    {
      do_create = false;
      do_insert = false;
    }
    else if ( std::strncmp( argv[2]+1, "rx", 2 )==0 )
      ratio_btree_to_stl = false;
    else if ( std::strncmp( argv[2]+1, "stl", 3 )==0 )
      stl_tests = true;
    else if ( std::strncmp( argv[2]+1, "html", 4 )==0 )
      html = true;
    else if ( *(argv[2]+1) == 's' )
      seed = atol( argv[2]+2 );
    else if ( *(argv[2]+1) == 'n' )
      node_sz = atoi( argv[2]+2 );
     else if ( *(argv[2]+1) == 'i' )
      initial_n = atol( argv[2]+2 );
    else if ( *(argv[2]+1) == 'l' )
      lg = atol( argv[2]+2 );
    else if ( *(argv[2]+1) == 'k' )
      do_pack = true;
    else if ( *(argv[2]+1) == 'r' )
      do_preload = true;
    else if ( *(argv[2]+1) == 'v' )
      verbose = true;
    else
    {
      cout << "Error - unknown option: " << argv[2] << "\n\n";
      argc = -1;
      break;
    }
  }

  if (argc < 2)
  {
    cout << "Usage: bt_time n [Options]\n"
      " The argument n specifies the number of test cases to run\n"
      " Options:\n"
      "   path     Specifies the test file path; default test.btree\n"
      "   -s#      Seed for random number generator; default 1\n"
      "   -n#      Node size (>=128); default " << "N/A" /*btree::default_node_size*/ << "\n"
      "              Small node sizes are useful for stress testing\n"
      "   -l#      log progress every # actions; default is to not log\n"
      "   -xc      No create; use file from prior -xe run\n"
      "   -xi      No insert test; forces -xc and doesn't do inserts\n"
      "   -xf      No find test\n"
      "   -xw      No iterate test\n"
      "   -xe      No erase test; use to save file intact\n"
      "   -k       Pack tree after insert test\n"
      "   -v       Verbose output statistics\n"
      "   -stl     Also run the tests against std::map\n"
      "   -rx      Report ratio as stl/btree instead of btree/stl\n"
      "   -html    Output html table of results to cerr\n"
      ;
    return 1;
  }

  cout << "sizeof(std::string) is " << sizeof(std::string) << "\n";
  cout << "starting tests with node size " << node_sz << "\n";

  {
    cout << "\n*************************  key_type long tests  ****************************\n";
    rand48  rng;
    uniform_int<long> n_dist(0, n-1);
    variate_generator<rand48&, uniform_int<long> > key(rng, n_dist);

    typedef boost::btree::mbt_map<boost::int32_t, boost::int32_t> map_type;
    map_type bt(node_sz);

    test(bt, rng, key);
  }
  //{
  //  cout << "\n*************************  key_type string tests  ****************************\n";
  //  boost::random_string  rng(4, 30, 'a', 'z');

  //  typedef boost::btree::mbt_map<std::string, boost::int32_t> map_type;
  //  map_type bt(node_sz);

  //  test(bt, rng, rng);
  //}
  {
    cout << "\n*******************  key_type indirect string tests  *************************\n";

    // generate the strings
    boost::random_string  rng(4, 50, 'a', 'z');
    char_vector_type chars;
    for (long i = 0; i < n; ++i)
    {
      std::string str(rng());
      for (const char* p = str.c_str(); *p; ++p)
        chars.push_back(*p);
      chars.push_back('\0');
    }

    // now set up the test
    typedef boost::btree::mbt_map<char*, boost::int32_t,
      boost::btree::indirect_less<char*> > map_type;

    map_type bt(node_sz);

    indirect_factory factory(chars);

    test(bt, factory, factory);
  }

  return 0;
}
