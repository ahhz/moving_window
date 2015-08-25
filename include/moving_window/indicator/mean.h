
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Not for public distribution
//=======================================================================

#ifndef INDICATOR_MEAN_H_AHZ
#define INDICATOR_MEAN_H_AHZ

#include <moving_window/traits.h>
#include <moving_window/default_construction_functor.h>

namespace moving_window {

  struct mean_tag
  {};

  template<typename T>
  struct mean
  {
    mean() :weight(0), sum(0)
    {}

    typedef mean<T> this_type;
    typedef mean_tag my_tag;
    typedef T value_type;

    template<typename Weight>
    void add_sample(const value_type& v, const Weight& w)
    {
      weight += w;
      sum += w * static_cast<double>(v);
    }

    void add_sample(const value_type& v)
    {
      weight++;
      sum += static_cast<double>(v);
    }

    template<typename Weight>
    void subtract_sample(const value_type& v, const Weight& w)
    {
      weight -= w;
      sum -= w * static_cast<double>(v);
    }

    void subtract_sample(const value_type& v)
    {
      weight--;
      sum -= static_cast<double>(v);
    }

    template<typename Weight>
    void add_subtotal(const this_type& subtotal, const Weight& w)
    {
      weight += w * subtotal.weight;
      sum += w * subtotal.sum;
    }

    void add_subtotal(const this_type& subtotal)
    {
      weight++;
      sum += subtotal.sum;
    }

    template<typename Weight>
    void subtract_subtotal(const this_type& subtotal, const Weight& w)
    {
      weight -= w * subtotal.weight;
      sum -= w * subtotal.sum;
    }

    void subtract_subtotal(const this_type& subtotal)
    {
      weight--;
      sum -= subtotal.sum;
    }

    double extract() const
    {
      return weight > 0 ? sum / weight : 1.0;
    }

    double weight;
    double sum;
  };

  template<>
  struct indicator_traits<mean_tag>
  {
    typedef pixel_element_tag element_type_tag;

    template<typename T>
    struct indicator
    {
      typedef mean<T> indicator_type;
      typedef default_construction_functor<indicator_type> initializer;
      static initializer make_initializer()
      {
        return initializer();
      }
    };
  };

}//namespace moving_window 
#endif //INDICATOR_MEAN_H_AHZ