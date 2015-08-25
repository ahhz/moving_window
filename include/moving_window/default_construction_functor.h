//
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Not for public distribution
//=======================================================================
//
// This class is used as a default for functions that allow specifying a 
// construction functor.

#ifndef DEFAULT_CONSTRUCTION_FUNCTOR_H_AHZ
#define DEFAULT_CONSTRUCTION_FUNCTOR_H_AHZ

#include <memory>

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
} // namespace moving_window
#endif
