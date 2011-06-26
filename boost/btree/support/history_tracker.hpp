//  history_tracker.hpp  ---------------------------------------------------------------//

//  Copyright Beman Dawes 2011

//  Distributed under the Boost Software License(0), Version 1.0.
//  http://www.boost.org/LICENSE_1_0.txt

//  This library is experimental and has not been accepted as a boost.org library

#ifndef BOOST_BTREE_HISTORY_TRACKER_HPP
#define BOOST_BTREE_HISTORY_TRACKER_HPP

#include <boost/assert.hpp>
#include <utility>   // for std::forward
#include <ostream>

namespace boost {
namespace btree {

//--------------------------------------------------------------------------------------//
//                                class history_tracker                                 //
//  This wrapper class keeps a count of how many time each special member function of   //
//  the wrapped class isinvoked. The primary envisioned use case is testing and tuning  //
//  container clases to avoid unnecessary copies or moves.                              //
//--------------------------------------------------------------------------------------//

template <class T>
class history_tracker : public T
{
public:

  typedef T history_tracker_base_type;

  history_tracker()
    : _default_ctor(1), _ctor(0), _copy_ctor(0), _copy_assign(0),
      _move_ctor(0), _move_assign(0), _dtor(0), T()
  {
    if (log())
      *log() << "  object " << this << " default constructor\n";
  }

  template <class U1>
  explicit history_tracker(const U1& u1)
    : _default_ctor(0), _ctor(1), _copy_ctor(0), _copy_assign(0),
      _move_ctor(0), _move_assign(0), _dtor(0), T(u1)
  {
    if (log())
      *log() << "  object " << this << " 1-arg constructor\n";
  }

  template <class U1, class U2>
  history_tracker(const U1& u1, const U2& u2)
    : _default_ctor(0), _ctor(1), _copy_ctor(0), _copy_assign(0),
      _move_ctor(0), _move_assign(0), _dtor(0), T(u1, u2)
  {
    if (log())
      *log() << "  object " << this << " 2-arg constructor\n";
  }

  template <class U1, class U2, class U3>
  history_tracker(const U1& u1, const U2& u2, const U3& u3)
    : _default_ctor(0), _ctor(1), _copy_ctor(0), _copy_assign(0),
      _move_ctor(0), _move_assign(0), _dtor(0), T(u1, u2, u3)
  {
    if (log())
      *log() << "  object " << this << " 3-arg constructor\n";
  }

  history_tracker(const history_tracker& x)
    : _default_ctor(x._default_ctor), _ctor(x._ctor), _copy_ctor(x._copy_ctor+1),
      _copy_assign(x._copy_assign), _move_ctor(x._move_ctor),
      _move_assign(x._move_assign), _dtor(x._dtor), T(x)
  {
    if (log())
      *log() << "  object " << this << " copy constructor from object " << &x << "\n";
  }

  history_tracker(history_tracker&& x)
    : _default_ctor(x._default_ctor), _ctor(x._ctor), _copy_ctor(x._copy_ctor),
      _copy_assign(x._copy_assign), _move_ctor(x._move_ctor+1),
      _move_assign(x._move_assign), _dtor(x._dtor), T(std::move(std::forward<T>(x)))
  {
    if (log())
      *log() << "  object " << this << " move constructor from object " << &x << "\n";
  }


 ~history_tracker()
  {
    BOOST_ASSERT(destruction() == 0);  // assert: object not previously destroyed
    ++_dtor;
    if (log())
      *log() << "  object " << this << " destructor\n";
  }

  history_tracker&  operator=(const history_tracker& x)
  {
    _default_ctor = x._default_ctor;
    _ctor         = x._ctor;
    _copy_ctor    = x._copy_ctor;
    _copy_assign  = x._copy_assign + 1;
    _move_ctor    = x._move_ctor;
    _move_assign  = x._move_assign;
    _dtor         = x._dtor;
    this->T::operator=(x);
    if (log())
      *log() << "  object " << this << " copy assignment from object " << &x << "\n";
    return *this;
  }

  history_tracker&  operator=(history_tracker&& x)
  {
    _default_ctor = x._default_ctor;
    _ctor         = x._ctor;
    _copy_ctor    = x._copy_ctor;
    _copy_assign  = x._copy_assign;
    _move_ctor    = x._move_ctor;
    _move_assign  = x._move_assign + 1;
    _dtor         = x._dtor;
    std::swap(*static_cast<T*>(this), *static_cast<T*>(&x));
    if (log())
      *log() << "  object " << this << " move assignment from object " << &x << "\n";
    return *this;
  }

  int  default_construction() const            { return _default_ctor; }
  int  construction() const                    { return _ctor; }
  int  copy_construction() const               { return _copy_ctor; }
  int  copy_assignment() const                 { return _copy_assign; }
  int  move_construction() const               { return _move_ctor; }
  int  move_assignment() const                 { return _move_assign; }
  int  destruction() const                     { return _dtor; }

  template <class OS>
  void history_tracker_dump(OS& os)
  {
    os << default_construction() << " default_construction\n";
    os << construction()         << " construction\n";
    os << copy_construction()    << " copy_construction\n";
    os << copy_assignment()      << " copy_assignment\n";
    os << move_construction()    << " move_construction\n";
    os << move_assignment()      << " move_assignment\n";
    os << destruction()          << " destruction\n";
  }
  
  static std::ostream* log()
  {
    return _stream();
  }
  
  static void log(std::ostream* os)  // 0 turns of logging
  {
    _stream() = os;
  }

private:
  int _default_ctor, _ctor, _copy_ctor, _copy_assign, _move_ctor, _move_assign, _dtor;

  static std::ostream*& _stream()
  {
    static std::ostream* os = 0;
    return os;
  }
};

}  // namespace btree
}  // namespace boost

#endif  // BOOST_BTREE_HISTORY_TRACKER_HPP
