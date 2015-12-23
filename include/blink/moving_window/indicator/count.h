
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef BLINK_MOVING_WINDOW_INDICATOR_COUNT_H_AHZ
#define BLINK_MOVING_WINDOW_INDICATOR_COUNT_H_AHZ

#include <blink/moving_window/traits.h>
#include <blink/moving_window/default_construction_functor.h>

namespace blink{
  namespace moving_window {

    struct count_tag
    {};

    template<typename T>
    struct count
    {
      count() : m_count(0)
      {}

      typedef count<T> this_type;
      typedef count_tag my_tag;
      typedef double value_type;

      template<typename Weight>
      void add_sample(const value_type& v, const Weight& w)
      {
        ++m_count;
      }

      void add_sample(const value_type& v)
      {
        ++m_count;
      }

      template<typename Weight>
      void subtract_sample(const value_type& v, const Weight& w)
      {
        --m_count;
      }

      void subtract_sample(const value_type& v)
      {
        --m_count;
      }

      template<typename Weight>
      void add_subtotal(const this_type& subtotal, const Weight& w)
      {
        m_count += subtotal.m_count;
      }

      void add_subtotal(const this_type& subtotal)
      {
        m_count += subtotal.m_count;
      }

      template<typename Weight>
      void subtract_subtotal(const this_type& subtotal, const Weight& w)
      {
        m_count -= subtotal.m_count;
      }

      void subtract_subtotal(const this_type& subtotal)
      {
        m_count -= subtotal.m_count;
      }

      int extract() const
      {
        return m_count;
      }

      int m_count;
    };

    template<>
    struct indicator_traits < count_tag >
    {
      typedef pixel_element_tag element_type_tag;

      template<typename T>
      struct indicator
      {
        typedef count<T> indicator_type;
        typedef default_construction_functor<indicator_type> initializer;
        static initializer make_initializer()
        {
          return initializer();
        }
      };
    };
  }//namespace moving_window 
}
#endif //INDICATOR_MEAN_H_AHZ