
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef INDICATOR_EDGE_DENSITY_H_AHZ
#define INDICATOR_EDGE_DENSITY_H_AHZ

#include <moving_window/traits.h>
#include <moving_window/default_construction_functor.h>
#include <boost/optional.hpp>
#include <utility>

namespace moving_window {

  struct edge_density_tag{};

  // the weight either consists of the weight for both sides of the edge
  // or it can be a scalar

  template<typename T>
  double mean_weight(const std::pair<boost::optional<T>, boost::optional<T> >& w)
  {
    if (w.first && w.second)	return (*w.first + *w.second) / 2;
    return 0;
  }

  double mean_weight(const double& w)
  {
    return w;
  }

  template<typename T> // value type of the edge sides 
  struct edge_density
  {
    edge_density() :count_all(0), count_edge(0)
    {}

    typedef edge_density<T> this_type;
    typedef edge_density_tag my_tag;

    template<typename T>
    struct optional_pair
    {
      typedef std::pair<boost::optional<T>, boost::optional<T> > type;
    };

    typedef typename optional_pair<T>::type edge_type;

    void add_sample(const edge_type& edge)
    {
      if (edge.first && edge.second) {
        count_all++;
        if (*(edge.first) != *(edge.second)) {
          count_edge++;
        }
      }
    }

    template<typename Weight>
    void add_sample(const edge_type& edge, const Weight& weight)
    {
      const double w = mean_weight(weight);
      if (edge.first && edge.second) {
        count_all += w;
        if (*(edge.first) != *(edge.second)) {
          count_edge += w;
        }
      }
    }
    template<typename Weight>
    void subtract_sample(const edge_type& edge, const Weight& weight)
    {
      const double w = mean_weight(weight);
      if (edge.first && edge.second) {
        count_all -= w;
        if (*(edge.first) != *(edge.second)) {
          count_edge -= w;
        }
      }
    }

    void subtract_sample(const edge_type& edge)
    {
      if (edge.first && edge.second) {
        count_all--;
        if (*(edge.first) != *(edge.second)) {
          count_edge--;
        }
      }
    }

    void add_subtotal(const this_type& subtotal)
    {
      count_all += subtotal.count_all;
      count_edge += subtotal.count_edge;
    }

    template<typename Weight>
    void add_subtotal(const this_type& subtotal, const Weight& w)
    {
      count_all += w * subtotal.count_all;
      count_edge += w * subtotal.count_edge;
    }

    void subtract_subtotal(const this_type& subtotal)
    {
      count_all -= subtotal.count_all;
      count_edge -= subtotal.count_edge;
    }

    template<typename Weight>
    void subtract_subtotal(const this_type& subtotal, const Weight& w)
    {
      count_all -= w * subtotal.count_all;
      count_edge -= w * subtotal.count_edge;
    }

    boost::optional<double> extract() const
    {
      return count_all > 0 ? count_edge / count_all : 1.0;
    }

    double count_all;
    double count_edge;
  };

  template<>
  struct indicator_traits<edge_density_tag>
  {
    typedef edge_element_tag element_type_tag;

    template<typename T>
    struct indicator
    {
      typedef edge_density<T> indicator_type;
      typedef default_construction_functor<indicator_type> initializer;
      static initializer make_initializer()
      {
        return initializer();
      }
    };
  };
} // namespace moving_window 
#endif //INDICATOR_EDGE_DENSITY_H_AHZ