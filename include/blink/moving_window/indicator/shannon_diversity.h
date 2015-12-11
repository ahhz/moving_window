
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef BLINK_MOVING_WINDOW_INDICATOR_SHANNON_DIVERSITY_H_AHZ
#define BLINK_MOVING_WINDOW_INDICATOR_SHANNON_DIVERSITY_H_AHZ

#include <blink/moving_window/traits.h>
#include <blink/moving_window/default_construction_functor.h>
#include <numeric>
#include <utility>
#include <vector>

namespace blink {
  namespace moving_window {

    struct shannon_diversity_tag
    {};

    struct counted_vector
    {
      void increment(int index, double w)
      {
        for (int i = static_cast<int>(counts.size()); i <= index; ++i)  {
          counts.push_back(0);
        }
        counts[index] += w;
      }

      void decrement(int index, double w)
      {
        counts[index] -= w;
      }

      std::vector<double> counts;
    };
    template<typename T>
    struct shannon_diversity
    {
      shannon_diversity()
      {}

      typedef shannon_diversity this_type;
      typedef most_common_class_tag my_tag;
      typedef T value_type;

      template<typename Weight>
      void add_sample(const value_type& v, const Weight& w)
      {
        int category = static_cast<int>(v + 0.5);
        cv.increment(category, w);
      }

      void add_sample(const value_type& v)
      {
        int category = static_cast<int>(v + 0.5);
        cv.increment(category, 1.0);
      }

      template<typename Weight>
      void subtract_sample(const value_type& v, const Weight& w)
      {
        int category = static_cast<int>(v + 0.5);
        cv.decrement(category, w);
      }

      void subtract_sample(const value_type& v)
      {
        int category = static_cast<int>(v + 0.5);
        cv.decrement(category, 1.0);
      }

      template<typename Weight>
      void add_subtotal(const this_type& subtotal, const Weight& w)
      {
        const int n = static_cast<int>(subtotal.cv.counts.size());
        for (int i = 0; i < n; ++i) {
          add_sample(i, subtotal.cv.counts[i] * w);
        }
      }

      void add_subtotal(const this_type& subtotal)
      {
        const int n = static_cast<int>(subtotal.cv.counts.size());
        for (int i = 0; i < n; ++i) {
          add_sample(i, subtotal.cv.counts[i]);
        }
      }

      template<typename Weight>
      void subtract_subtotal(const this_type& subtotal, const Weight& w)
      {
        const int n = subtotal.cv.counts.size();
        for (int i = 0; i < n; ++i) {
          subtract_sample(i, subtotal.cv.counts[i] * w);
        }
      }

      void subtract_subtotal(const this_type& subtotal)
      {
        const int n = static_cast<int>(subtotal.cv.counts.size());
        for (int i = 0; i < n; ++i) {
          subtract_sample(i, subtotal.cv.counts[i]);
        }
      }

      boost::optional<double> extract() const
      {
        double sum = 0;
        sum = std::accumulate(cv.counts.begin(), cv.counts.end(), sum);
        double h = 0;
        //auto lambda = [](double total, double elem) return  (elem > 0)  ? total + elem * log(elem) : total;
        //std::accumulate(cv.counts.begin(), cv.counts.end(), h, lambda);
        for (auto i : cv.counts) {
          if (i > 0) {
            const double pi = i / sum;
            h -= pi * log(pi);
          }
        }
        return boost::make_optional(true, h);
      }
      counted_vector cv;
    };

    template<>
    struct indicator_traits < shannon_diversity_tag >
    {
      typedef pixel_element_tag element_type_tag;

      template<typename T>
      struct indicator
      {
        typedef shannon_diversity<T> indicator_type;
        typedef default_construction_functor<indicator_type> initializer;
        static initializer make_initializer()
        {
          return initializer();
        }
      };
    };
  }//namespace moving_window 
}
#endif