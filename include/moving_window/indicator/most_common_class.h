
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef INDICATOR_MOST_COMMON_CLASS_H_AHZ
#define INDICATOR_MOST_COMMON_CLASS_H_AHZ

#include <moving_window/traits.h>
#include <moving_window/default_construction_functor.h>
// #include <boost/heap/fibonacci_heap.hpp> TODO investigate using boost heap instead of makeshift sorted_vector below.
#include <utility>
#include <vector>

namespace moving_window {

  struct most_common_class_tag
  {};

  struct sorted_vector
  {
    void increment(int index, double w)
    {
      for (int i = static_cast<int>(lookup.size()); i <= index; ++i)  {
        lookup.push_back(i);
        sorted.push_back(std::make_pair(i, 0.0));
      }
      int old_i = lookup[index];
      sorted[old_i].second += w;
      double new_w = sorted[old_i].second;
      int new_i = old_i;
      while (new_i > 0 && sorted[new_i - 1].second < new_w) {
        --new_i;
      }
      auto temp = sorted[old_i];
      for (int i = old_i; i > new_i; --i)  {
        sorted[i] = sorted[i - 1];
        lookup[sorted[i].first] = i;
      }
      sorted[new_i] = temp;
      lookup[sorted[new_i].first] = new_i;
    }

    void decrement(int index, double w)
    {
      int old_i = lookup[index];
      sorted[old_i].second -= w;
      double new_w = sorted[old_i].second;
      int new_i = old_i;
      while (new_i < lookup.size() - 1 && sorted[new_i + 1].second > new_w)  {
        ++new_i;
      }
      auto temp = sorted[old_i];
      for (int i = old_i; i < new_i; ++i) {
        sorted[i] = sorted[i + 1];
        lookup[sorted[i].first] = i;
      }
      sorted[new_i] = temp;
      lookup[sorted[new_i].first] = new_i;
    }

    std::vector<int> lookup;
    std::vector<std::pair<int, double> > sorted;
  };

  template<typename T>
  struct most_common_class
  {
    most_common_class()
    {}

    typedef most_common_class this_type;
    typedef most_common_class_tag my_tag;
    typedef T value_type;

    template<typename Weight>
    void add_sample(const value_type& v, const Weight& w)
    {
      int category = static_cast<int>(v + 0.5);
      sv.increment(category, w);
    }

    void add_sample(const value_type& v)
    {
      int category = static_cast<int>(v + 0.5);
      sv.increment(category, 1.0);
    }

    template<typename Weight>
    void subtract_sample(const value_type& v, const Weight& w)
    {
      int category = static_cast<int>(v + 0.5);
      sv.decrement(category, w);
    }

    void subtract_sample(const value_type& v)
    {
      int category = static_cast<int>(v + 0.5);
      sv.decrement(category, 1.0);
    }

    template<typename Weight>
    void add_subtotal(const this_type& subtotal, const Weight& w)
    {
      for (auto i : subtotal.sv.sorted) {
        add_sample(i.first, i.second * w);
      }
    }

    void add_subtotal(const this_type& subtotal)
    {
      for (auto i : subtotal.sv.sorted) {
        add_sample(i.first, i.second);
      }
    }

    template<typename Weight>
    void subtract_subtotal(const this_type& subtotal, const Weight& w)
    {
      for (auto i : subtotal.sv.sorted) {
        subtract_sample(i.first, i.second * w);
      }
    }

    void subtract_subtotal(const this_type& subtotal)
    {
      for (auto i : subtotal.sv.sorted) {
        subtract_sample(i.first, i.second);
      }
    }

    boost::optional<int> extract() const
    {
      return boost::make_optional(true, sv.sorted.front().first);
    }
    sorted_vector sv;
  };

  template<>
  struct indicator_traits<most_common_class_tag>
  {
    typedef pixel_element_tag element_type_tag;

    template<typename T>
    struct indicator
    {
      typedef most_common_class<T> indicator_type;
      typedef default_construction_functor<indicator_type> initializer;
      static initializer make_initializer()
      {
        return initializer();
      }
    };
  };

} //namespace moving_window 
#endif //INDICATOR_MOST_COMMON_CLASS_H_AHZ