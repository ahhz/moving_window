
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef INDICATOR_INTERSPERSION_H_AHZ
#define INDICATOR_INTERSPERSION_H_AHZ

#include <boost/optional.hpp>

#include <cmath>
#include <set>
#include <map>
#include <utility>

namespace moving_window {

  template<typename T> // value type of the edge sides 
  struct interspersion
  {
    interspersion() :m_total(0)
    {}

    typedef interspersion<T> this_type;
    typedef std::pair<boost::optional<T>, boost::optional<T> > edge_type;
    typedef std::pair<T, T> clean_edge_type;
    typedef std::map<std::pair<T, T>, int > sparse_matrix_type;

    std::pair<T, T> make_pair_sorted(const T& a, const T& b)
    {
      return (a > b) ? std::make_pair(a, b) : std::make_pair(b, a);
    }

    void add_sample(const edge_type& edge)
    {

      if (edge.first && edge.second && (*(edge.first) != *(edge.second))) {
        clean_edge_type clean_edge = make_pair_sorted(*(edge.first), *(edge.second));
        ++m_edges[clean_edge];
        ++m_total;
      }
    }

    void subtract_sample(const edge_type& edge)
    {

      if (edge.first && edge.second && (*(edge.first) != *(edge.second))) {
        clean_edge_type clean_edge = make_pair_sorted(*(edge.first), *(edge.second));

        sparse_matrix_type::iterator i = m_edges.find(clean_edge);

        assert(i != m_edges.end());
        --(i->second);
        --m_total;
        if (i->second == 0) {
          m_edges.erase(i);
        }
      }
    }

    void add_subtotal(const this_type& subtotal)
    {
      sparse_matrix_type::const_iterator i = subtotal.m_edges.begin();
      sparse_matrix_type::const_iterator i_end = subtotal.m_edges.end();

      for (; i != i_end; ++i) {
        m_edges[i->first] += i->second;
        m_total += i->second;
      }
    };

    void subtract_subtotal(const this_type& subtotal)
    {

      sparse_matrix_type::const_iterator i = subtotal.m_edges.begin();
      sparse_matrix_type::const_iterator i_end = subtotal.m_edges.end();

      for (; i != i_end; ++i) {
        sparse_matrix_type::iterator j = m_edges.find(i->first);
        j->second -= i->second;
        m_total -= i->second;
        if (j->second == 0)
        {
          m_edges.erase(j);
        }
      }
    };


    double extract() const
    {
      sparse_matrix_type::const_iterator i = m_edges.begin();
      sparse_matrix_type::const_iterator i_end = m_edges.end();
      double numerator = 0;
      std::set<T> categories;
      int total = 0;
      for (; i != i_end; ++i)
      {
        categories.insert(i->first.first);
        categories.insert(i->first.second);
        total += i->second;
        const double edge_fraction = static_cast<double>(i->second) / static_cast<double>(m_total);
        numerator += edge_fraction * log(edge_fraction);
      }
      assert(total == m_total);
      const std::size_t m = categories.size();
      const double nominator = m > 1 ? log(0.5 * m * (m - 1.0)) : 0.0;
      return nominator > 0 ? -numerator / nominator : 0;
    };

    sparse_matrix_type m_edges;
    int m_total;
  };

} // namespace moving_window 
#endif //INDICATOR_INTERSPERSION_H_AHZ