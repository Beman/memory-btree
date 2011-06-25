//  history_tracker_test.cpp  ----------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  This library is experimental and has not been accepted as a boost.org library

#include <boost/config/warning_disable.hpp>

#include <boost/btree/support/history_tracker.hpp>

#include <iostream>
#include <boost/detail/lightweight_test.hpp>

#include <boost/test/included/prg_exec_monitor.hpp>

using namespace boost;
using std::cout; using std::endl;

namespace
{
  struct Int
  {
    int value;

    Int() : value(-1) {}
    Int(int v) : value(v) {}
  };
  typedef boost::btree::history_tracker<Int> kiss;
}

int cpp_main(int, char*[])
{
  kiss::log(&cout);

  kiss x;
  BOOST_TEST_EQ(x.default_construction(), 1);
  BOOST_TEST_EQ(x.construction(), 0);
  BOOST_TEST_EQ(x.copy_construction(), 0);
  BOOST_TEST_EQ(x.copy_assignment(), 0);
  BOOST_TEST_EQ(x.move_construction(), 0);
  BOOST_TEST_EQ(x.move_assignment(), 0);
  BOOST_TEST_EQ(x.destruction(), 0);

  kiss y(1);
  BOOST_TEST_EQ(y.default_construction(), 0);
  BOOST_TEST_EQ(y.construction(), 1);
  BOOST_TEST_EQ(y.copy_construction(), 0);
  BOOST_TEST_EQ(y.copy_assignment(), 0);
  BOOST_TEST_EQ(y.move_construction(), 0);
  BOOST_TEST_EQ(y.move_assignment(), 0);
  BOOST_TEST_EQ(y.destruction(), 0);

  kiss z(x);
  BOOST_TEST_EQ(z.default_construction(), 1);
  BOOST_TEST_EQ(z.construction(), 0);
  BOOST_TEST_EQ(z.copy_construction(), 1);
  BOOST_TEST_EQ(z.copy_assignment(), 0);
  BOOST_TEST_EQ(z.move_construction(), 0);
  BOOST_TEST_EQ(z.move_assignment(), 0);
  BOOST_TEST_EQ(z.destruction(), 0);

  
  return report_errors();
}
