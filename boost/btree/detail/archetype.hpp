//  archetype.hpp  ---------------------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_BTREE_ARCHETYPE_HPP       
#define BOOST_BTREE_ARCHETYPE_HPP

#include <cstddef>
#include <string>
#include <boost/cstdint.hpp>
#include <boost/assert.hpp>
#include <ostream>

namespace boost
{

namespace archetype_count
{
  int default_construct = 0;
  int construct = 0;
  int copy_construct = 0;
  int move_construct = 0;
  int destruct = 0;
  int copy_assign = 0;
  int move_assign = 0;

  int sum()
  {
    return default_construct + construct + copy_construct
      +move_construct + destruct + copy_assign + move_assign;
  }

  int all_constructors()
  {
    return default_construct + construct + copy_construct + move_construct;
  }

  void clear()
  {
    default_construct = 0;
    construct = 0;
    copy_construct = 0;
    move_construct = 0;
    destruct = 0;
    copy_assign = 0;
    move_assign = 0;
  }

  void dump(std::ostream& os)
  {
    os << "  default_construct: " << default_construct << '\n';
    os << "  construct: " << construct << '\n';
    os << "  copy_construct: " << copy_construct << '\n';
    os << "  move_construct: " << move_construct << '\n';
    os << "  destruct: " << destruct << '\n';
    os << "  copy_assign: " << copy_assign << '\n';
    os << "  move_assign: " << move_assign << std::endl;
  }
}


class archetype
{
public:
  archetype() : _value(-1)
  {
    BOOST_ASSERT(_uid != _uid_value);
    _uid = _uid_value;
    ++archetype_count::default_construct;
  }
  explicit archetype(boost::int32_t v) : _value(v)
  {
    BOOST_ASSERT(_uid != _uid_value);
    _uid = _uid_value;
    ++archetype_count::construct;
  }
  archetype(const archetype& x)  : _value(x._value)
  {
    BOOST_ASSERT(_uid != _uid_value);
    _uid = _uid_value;
    ++archetype_count::copy_construct;
  }
  archetype(archetype&& rx)  : _value(rx._value)
  {
    BOOST_ASSERT(_uid != _uid_value);
    rx._value = -2;
    _uid = _uid_value;
    ++archetype_count::move_construct;
  }
 ~archetype()
  {
    BOOST_ASSERT(_uid == _uid_value);
    _value = -3;
    _uid = 0;
    ++archetype_count::destruct;
  }
  archetype& operator=(const archetype& x)
  {
    BOOST_ASSERT(_uid == _uid_value);
    _value = x._value;
    ++archetype_count::copy_assign;
    return *this;
  }
  archetype& operator=(archetype&& rx)
  {
    BOOST_ASSERT(_uid == _uid_value);
    _value = rx._value;
    rx._value = -4;
    ++archetype_count::move_assign;
    return *this;
  }
  boost::int32_t value() const
  {
    BOOST_ASSERT(_uid == _uid_value);
    return _value;
  }

private:
  boost::uint64_t  _uid;  // unique marker indicates memory initialized
  boost::int32_t   _value;

  static const boost::uint64_t _uid_value = 0x0123456789abcdefULL;
};

} // namespace boost

#endif  // BOOST_BTREE_ARCHETYPE_HPP
