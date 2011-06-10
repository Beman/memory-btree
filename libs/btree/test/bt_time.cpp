//  bt_time.cpp  -----------------------------------------------------------------------//

//  Copyright Beman Dawes 1994, 2010

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org/libs/btree for documentation.

#include <boost/btree/map.hpp>
#include <boost/filesystem.hpp>
#include <boost/random.hpp>
#include <boost/btree/support/timer.hpp>
#include <boost/detail/lightweight_main.hpp>

#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>  // for atol()
#include <map>

using namespace boost;
namespace fs = boost::filesystem;
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
  int cache_sz = btree::default_max_cache_nodes;
  int node_sz = btree::default_node_size;
  bool do_create (true);
  bool do_preload (false);
  bool do_insert (true);
  bool do_pack (false);
  bool do_find (true);
  bool do_iterate (true);
  bool do_erase (true);
  bool verbose (false);
  bool stl_tests (false);
  bool html (false);
  const int places = 2;
  std::string path("bt_time.btree");
  std::string path_org("bt_time.btree.org");
  BOOST_SCOPED_ENUM(integer::endianness) whichaway = integer::endianness::native;

  btree::times_t insert_tm;
  btree::times_t find_tm;
  btree::times_t iterate_tm;
  btree::times_t erase_tm;
  const long double sec = 1000000.0L;

  template <class BT>
  void test()
  {
    btree::run_timer t(3);
    rand48  rng;
    uniform_int<long> n_dist(0, n-1);
    variate_generator<rand48&, uniform_int<long> > key(rng, n_dist);

    {
      btree::flags::bitmask flgs =
        do_create ? btree::flags::truncate
                  : btree::flags::read_write;
      if (!do_create && do_preload)
        flgs |= btree::flags::preload;

      cout << "\nopening " << path << endl;
      t.start();
      BT bt(path, flgs, node_sz);
      t.stop();
      t.report();

      bt.max_cache_size(cache_sz);

      if (do_insert)
      {
        cout << "\ninserting " << n << " btree elements..." << endl;
        rng.seed(seed);
        t.start();
        for (long i = 1; i <= n; ++i)
        {
          if (lg && i % lg == 0)
            std::cout << i << std::endl; 
          bt.emplace(key(), i);
        }
        insert_tm = t.stop();
        t.report();
      }

      if (do_pack)
      {
        cout << "\npacking btree..." << endl;
        bt.close();
        fs::remove(path_org);
        fs::rename(path, path_org);
        t.start();
        BT bt_old(path_org);
        bt_old.max_cache_size(cache_sz);
        BT bt_new(path, btree::flags::truncate, node_sz);
        bt_new.max_cache_size(cache_sz);
        for (typename BT::iterator it = bt_old.begin(); it != bt_old.end(); ++it)
        {
          bt_new.emplace(it->key(), it->mapped_value());
        }
        cout << "  bt_old.size() " << bt_old.size() << std::endl;
        cout << "  bt_new.size() " << bt_new.size() << std::endl;
        BOOST_ASSERT(bt_new.size() == bt_old.size());
        bt_old.close();
        bt_new.close();
        t.report();
        cout << "  " << path_org << " file size: " << fs::file_size(path_org) << '\n';
        cout << "  " << path << "     file size: " << fs::file_size(path) << '\n';
        bt.open(path, btree::flags::read_write);
        bt.max_cache_size(cache_sz);
      }

      if (do_find)
      {
        cout << "\nfinding " << n << " btree elements..." << endl;
        rng.seed(seed);
        typename BT::const_iterator itr;
        long k;
        t.start();
        for (long i = 1; i <= n; ++i)
        {
          if (lg && i % lg == 0)
            std::cout << i << std::endl;
          k = key();
          itr = bt.find(k);
#       if !defined(NDEBUG)
          if (itr == bt.end())
            throw std::runtime_error("btree find() returned end()");
          if (itr->key() != k)
            throw std::runtime_error("btree find() returned wrong iterator");
#       endif 
        }
        find_tm = t.stop();
        t.report();
      }

      if (do_iterate)
      {
        cout << "\niterating over " << bt.size() << " btree elements..." << endl;
        unsigned long count = 0;
        long prior_key = -1L;
        t.start();
        for (typename BT::const_iterator itr = bt.begin();
          itr != bt.end();
          ++itr)
        {
          ++count;
          if (itr->key() <= prior_key)
            throw std::runtime_error("btree iteration sequence error");
          prior_key = itr->key();
        }
        iterate_tm = t.stop();
        t.report();
        if (count != bt.size())
          throw std::runtime_error("btree iteration count error");
      }

      if (verbose)
      {
        bt.flush();
        cout << '\n' << bt << endl;
        cout << bt.manager() << endl;
      }

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

      if (verbose)
      {
        bt.flush();
        cout << '\n' << bt << endl;
        cout << bt.manager() << endl;
      }

      bt.close();
    }

    typedef std::map<long, long>  stl_type;
    stl_type stl;

    if (stl_tests)
    {
      cout << "\ninserting " << n << " std::map elements..." << endl;
      rng.seed(seed);
      btree::times_t this_tm;
      t.start();
      for (long i = 1; i <= n; ++i)
      {
        if (lg && i % lg == 0)
          std::cout << i << std::endl; 
        stl_type::value_type element(key(), i);
        stl.insert(element);
      }
      this_tm = t.stop();
      t.report();
      if (html)
      {
        cerr << "<tr>\n  <td><code>" << command_args << "</code></td>\n";  
        cerr.setf(std::ios_base::fixed, std::ios_base::floatfield);
        cerr.precision(places);
        if (this_tm.wall)
        {
          double ratio = (insert_tm.wall * 1.0) / this_tm.wall;
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
      if (this_tm.wall)
        cout << "  ratio of btree to stl wall clock time: "
             << (insert_tm.wall * 1.0) / this_tm.wall << '\n';
      if (verbose && this_tm.system + this_tm.user)
        cout << "  ratio of btree to stl cpu time: "
             << ((insert_tm.system + insert_tm.user) * 1.0)
                / (this_tm.system + this_tm.user) << '\n';

      cout << "\nfinding " << n << " std::map elements..." << endl;
      stl_type::const_iterator itr;
      long k;
      rng.seed(seed);
      t.start();
      for (long i = 1; i <= n; ++i)
      {
        if (lg && i % lg == 0)
          std::cout << i << std::endl; 
          k = key();
          itr = stl.find(k);
#       if !defined(NDEBUG)
          if (itr == stl.end())
            throw std::runtime_error("stl find() returned end()");
          if (itr->first != k)
            throw std::runtime_error("stl find() returned wrong iterator");
#       endif
      }
      this_tm = t.stop();
      t.report();
      if (html)
      {
        if (this_tm.wall)
        {
          double ratio = (find_tm.wall * 1.0) / this_tm.wall;
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
      if (this_tm.wall)
        cout << "  ratio of btree to stl wall clock time: "
             << (find_tm.wall * 1.0) / this_tm.wall << '\n';
      if (verbose && this_tm.system + this_tm.user)
        cout << "  ratio of btree to stl cpu time: "
             << ((find_tm.system + find_tm.user) * 1.0)
                / (this_tm.system + this_tm.user) << '\n';

      cout << "\niterating over " << stl.size() << " stl elements..." << endl;
      unsigned long count = 0;
      long prior_key = -1L;
      t.start();
      for (stl_type::const_iterator itr = stl.begin();
        itr != stl.end();
        ++itr)
      {
        ++count;
        if (itr->first <= prior_key)
          throw std::runtime_error("stl iteration sequence error");
        prior_key = itr->first;
      }
      this_tm = t.stop();
      t.report();
      if (html)
      {
        if (this_tm.wall)
        {
          double ratio = (iterate_tm.wall * 1.0) / this_tm.wall;
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
      if (this_tm.wall)
        cout << "  ratio of btree to stl wall clock time: "
             << (iterate_tm.wall * 1.0) / this_tm.wall << '\n';
      if (verbose && this_tm.system + this_tm.user)
        cout << "  ratio of btree to stl cpu time: "
             << ((iterate_tm.system + iterate_tm.user) * 1.0)
                / (this_tm.system + this_tm.user) << '\n';
      if (count != stl.size())
        throw std::runtime_error("stl iteration count error");

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
          double ratio = (erase_tm.wall * 1.0) / this_tm.wall;
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
      if (this_tm.wall)
        cout << "  ratio of btree to stl wall clock time: "
             << (erase_tm.wall * 1.0) / this_tm.wall << '\n';
      if (verbose && this_tm.system + this_tm.user)
        cout << "  ratio of btree to stl cpu time: "
             << ((erase_tm.system + erase_tm.user) * 1.0)
                / (this_tm.system + this_tm.user) << '\n';
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
    if (*argv[2] != '-')
      path = argv[2];
    else
    {
      if ( std::strncmp( argv[2]+1, "xe", 2 )==0 )
        do_erase = false;
      else if ( std::strncmp( argv[2]+1, "xf", 2 )==0 )
        do_find = false;
      else if ( std::strncmp( argv[2]+1, "xc", 2 )==0 )
        do_create = false;
      else if ( std::strncmp( argv[2]+1, "xi", 2 )==0 )
      {
        do_create = false;
        do_insert = false;
      }
      else if ( std::strncmp( argv[2]+1, "stl", 3 )==0 )
        stl_tests = true;
      else if ( std::strncmp( argv[2]+1, "html", 4 )==0 )
        html = true;
      else if ( std::strncmp( argv[2]+1, "big", 3 )==0 )
        whichaway = integer::endianness::big;
      else if ( std::strncmp( argv[2]+1, "little", 6 )==0 )
        whichaway = integer::endianness::little;
      else if ( std::strncmp( argv[2]+1, "native", 6 )==0 )
        whichaway = integer::endianness::native;
      else if ( *(argv[2]+1) == 's' )
        seed = atol( argv[2]+2 );
      else if ( *(argv[2]+1) == 'n' )
        node_sz = atoi( argv[2]+2 );
      else if ( *(argv[2]+1) == 'c' )
        cache_sz = atoi( argv[2]+2 );
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
  }

  if (argc < 2) 
  {
    cout << "Usage: bt_time n [Options]\n"
      " The argument n specifies the number of test cases to run\n"
      " Options:\n"
      "   path     Specifies the test file path; default test.btree\n"
      "   -s#      Seed for random number generator; default 1\n"
      "   -n#      Node size (>=128); default " << btree::default_node_size << "\n"
      "              Small node sizes are useful for stress testing\n"
      "   -c#      Cache size; default " << btree::default_max_cache_nodes << " nodes\n"
      "   -l#      log progress every # actions; default is to not log\n"
      "   -xc      No create; use file from prior -xe run\n"
      "   -xi      No insert test; forces -xc and doesn't do inserts\n"
      "   -xf      No find test\n"
      "   -xi      No iterate test\n"
      "   -xe      No erase test; use to save file intact\n"
      "   -k       Pack tree after insert test\n"
      "   -v       Verbose output statistics\n"
      "   -stl     Also run the tests against std::map\n"
      "   -r       Read entire file to preload operating system disk cache;\n"
      "            only applicable if -xc option is active\n"
      "   -big     Use btree::default_big_endian_traits\n"
      "   -little  Use btree::default_little_endian_traits\n"
      "   -native  Use btree::default_native_traits; this is the default\n"
      "   -html    Output html table of results to cerr\n"
      ;
    return 1;
  }

  cout << "starting tests with node size " << node_sz
       << ", maximum cache nodes " << cache_sz << ",\n";

  switch (whichaway)
  {
  case integer::endianness::big:
    cout << "and big endianness\n";
    test< btree::btree_map<long, long, btree::default_big_endian_traits> >();
    break;
  case integer::endianness::little:
    cout << "and little endianness\n";
    test< btree::btree_map<long, long, btree::default_little_endian_traits> >();
    break;
  case integer::endianness::native:
    cout << "and native endianness\n";
    test< btree::btree_map<long, long, btree::default_native_traits> >();
    break;
  }

  return 0;
}
