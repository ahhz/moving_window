//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// This class is used as a default for functions that allow specifying a 
// construction functor.
// The idea is that a class can contain a constructing object, without the need 
// of adding a template parameter to the class 
//

/* Example

template<class T> struct foo
{
  template<class Maker = default_construction_functor<T> >
  foo(Maker maker = Maker() ) : m_maker(maker) {}

  void bar() {
    T t = m_maker();
  }

private:
  construction_functor<T> m_maker;
};

*/
// 

#ifndef BLINK_MOVING_WINDOW_DEFAULT_CONSTRUCTION_FUNCTOR_H_AHZ
#define BLINK_MOVING_WINDOW_DEFAULT_CONSTRUCTION_FUNCTOR_H_AHZ

#include <memory>
namespace blink {
  namespace moving_window {

    template<typename T>
    struct default_construction_functor
    {
      T operator()() const
      {
        return T();
      }
    };

    template<typename T>
    struct construction_functor_base
    {
      virtual T operator()() = 0;
    };

    template<typename T, typename TMaker>
    struct construction_functor_helper : construction_functor_base < T >
    {
      construction_functor_helper(TMaker& maker) : m_maker(maker)
      {
      }

      T operator()()
      {
        return m_maker();
      }
      TMaker m_maker;
    };

    template<typename T>
    struct construction_functor
    {
      template<typename TMaker = default_construction_functor<T> >
      construction_functor(TMaker maker = TMaker())
        : m_maker(new construction_functor_helper<T, TMaker>(maker))
      {
      }

      T operator()()
      {
        return (*m_maker)();
      }

    private:
      std::shared_ptr<construction_functor_base<T> > m_maker;
    };
  }
} // namespace moving_window
#endif
