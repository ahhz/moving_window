
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef BLINK_MOVING_WINDOW_INDICATOR_EDGE_LIST_H_AHZ
#define BLINK_MOVING_WINDOW_INDICATOR_EDGE_LIST_H_AHZ

#include <blink/moving_window/traits.h>

#include <boost/optional.hpp>

#include <cmath>
#include <map>
#include <set>
#include <sstream>
#include <utility>

namespace blink {
  namespace moving_window {

    struct edge_list_tag
    {};

    template<typename T> // value type of the edge sides 
    struct edge_list
    {
      edge_list()
      {}

      typedef edge_list<T> this_type;
      typedef std::pair<boost::optional<T>, boost::optional<T> > edge_type;
      typedef std::pair<std::pair<T, T>, std::pair<T, T> > clean_edge_type;
      typedef std::map<clean_edge_type, int > sparse_matrix_type;


      void add_sample(const edge_type& edge)
      {
        T ar = edge.first ? *(edge.first) / 1000 : 999;
        T ac = edge.first ? *(edge.first) % 1000 : 999;
        T br = edge.second ? *(edge.second) / 1000 : 999;
        T bc = edge.second ? *(edge.second) % 1000 : 999;

        clean_edge_type clean_edge = std::make_pair(std::make_pair(ar, ac), std::make_pair(br, bc));

        ++m_edges[clean_edge];
      }

      void subtract_sample(const edge_type& edge)
      {
        T ar = edge.first ? *(edge.first) / 1000 : 999;
        T ac = edge.first ? *(edge.first) % 1000 : 999;
        T br = edge.second ? *(edge.second) / 1000 : 999;
        T bc = edge.second ? *(edge.second) % 1000 : 999;

        clean_edge_type clean_edge = std::make_pair(std::make_pair(ar, ac), std::make_pair(br, bc));

        sparse_matrix_type::iterator i = m_edges.find(clean_edge);

        assert(i != m_edges.end());
        --(i->second);
        if (i->second == 0) {
          m_edges.erase(i);
        }
      }

      void add_subtotal(const this_type& subtotal)
      {
        sparse_matrix_type::const_iterator i = subtotal.m_edges.begin();
        sparse_matrix_type::const_iterator i_end = subtotal.m_edges.end();

        for (; i != i_end; ++i) {
          m_edges[i->first] += i->second;
        }
      };

      void subtract_subtotal(const this_type& subtotal)
      {
        sparse_matrix_type::const_iterator i = subtotal.m_edges.begin();
        sparse_matrix_type::const_iterator i_end = subtotal.m_edges.end();

        for (; i != i_end; ++i) {
          sparse_matrix_type::iterator j = m_edges.find(i->first);
          assert(j != m_edges.end());
          j->second -= i->second;
          if (j->second == 0)
          {
            m_edges.erase(j);
          }
        }
      };


      double extract() const
      {
        return static_cast<double>(m_edges.size());
      };

      sparse_matrix_type m_edges;
    };



    template<>
    struct indicator_traits < edge_list_tag >
    {
      typedef edge_element_tag element_type_tag;

      template<typename T>
      struct indicator
      {
        typedef edge_list<T> indicator_type;
        typedef default_construction_functor<indicator_type> initializer;
        static initializer make_initializer()
        {
          return initializer();
        }
      };
    };
  }
} // namespace moving_window 
#endif //INDICATOR_EDGE_LIST_H_AHZ