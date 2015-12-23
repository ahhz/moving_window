//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// The moving_window_edge_iterator provides a generalized interface for both
// circular window iterators of pixel (or patch) based indicators

#ifndef BLINK_MOVING_WINDOW_CIRCULAR_WINDOW_ITERATOR_H_AHZ
#define BLINK_MOVING_WINDOW_CIRCULAR_WINDOW_ITERATOR_H_AHZ

#include <blink/moving_window/indicator_input_raster.h>
#include <blink/moving_window/default_construction_functor.h>
#include <blink/raster/raster_view.h>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/none_t.hpp>
#include <boost/optional.hpp>

#include <algorithm> // min max
#include <vector>
#include <utility> // pair

namespace blink {
  namespace moving_window {
  
    template<typename Indicator, typename Raster, typename Window>
    class circular_window_iterator
      : public boost::iterator_facade
      < circular_window_iterator<Indicator, Raster, Window>
      , const Indicator, boost::forward_traversal_tag >
    {
    public:
      using coordinate_type = typename Raster::coordinate_type;
      using index_type = typename coordinate_type::index_type;
      using is_weighted = typename Raster::is_weighted;

    private:
     using  input_data_type = Raster;
     using trans_view = blink::raster::raster_view <
       blink::raster::orientation::col_major, blink::raster::element::pixel,
       blink::raster::access::read_only, input_data_type >;

     using input_iterator =  typename Raster::iterator;
     using input_trans_iterator = typename trans_view::iterator;
    
     using optional_input_iterator = boost::optional<input_iterator>;
     using optional_input_trans_iterator = boost::optional<input_trans_iterator>;

      // .first gives the offset relative to the current position of the iterator
      // .second gives the iterator at the offset position (if on raster). 

     using iterator_vector = std::vector < std::pair<coordinate_type, 
       optional_input_iterator> > ;
     using trans_iterator_vector = std::vector < std::pair<coordinate_type, 
       optional_input_trans_iterator> >;
   
    public:
      template<typename Initializer = default_construction_functor<Indicator> >
      circular_window_iterator(Window& window, Raster* data
        , Initializer initializer = Initializer()) : m_data(data)
        , m_data_trans(data), m_initializer(initializer)
        , m_indicator(initializer()), m_indicator_start_of_row(initializer())
        , m_coordinates(0, 0), m_radius(window.get_radius())
      {
        int int_radius = static_cast<int>(m_radius);
        for (int a = -int_radius; a <= int_radius; ++a) {
          int b = static_cast<int>(sqrt(int_radius * int_radius - a * a) ); // rounding problem do on the basis of square
          m_left.emplace_back(coordinate_type(a, -b), boost::none);
          m_right.emplace_back(coordinate_type(a, b), boost::none);
          m_top.emplace_back(coordinate_type(-b, a), boost::none);
          m_bottom.emplace_back(coordinate_type(b, a), boost::none);
        }
      }

      void find_begin()
      {
        m_coordinates = coordinate_type(0, 0);

        nullify_iterator_vector(m_bottom);
        nullify_iterator_vector(m_top);

        m_indicator_start_of_row = m_initializer();

        for (auto&& bottom : m_bottom) {
          auto nb = bottom.first;
          if (nb.col >= 0) {
            bottom.second = m_data_trans.begin() + m_data->size1() * nb.col;
            for (int r = 0; r <= nb.row; ++r){
              if (r == m_data->size1()) {
                bottom.second = boost::none;
              }
              else {
                typename input_data_type::value_type v = **(bottom.second);
                add_sample_to_indicator(is_weighted{}, m_indicator_start_of_row, v);
                ++(*bottom.second);
              }
            }
          }
        }
        prepare_first_pixel_of_row();
      }

      void find_end()
      {
        m_coordinates = coordinate_type(m_data->size1(), 0);
     }

    private:
      friend class boost::iterator_core_access;

      void increment()
      {
        if (++m_coordinates.col != m_data->size2()) {
          return moved_right();
        }
        m_coordinates.col = 0;
        if (++m_coordinates.row != m_data->size1()) {
          return moved_down();
        }
      }

      bool equal(const circular_window_iterator& that) const
      {
        return m_coordinates == that.m_coordinates;
      }

      const Indicator& dereference() const
      {
        return get_indicator();
      }

    private:
      void moved_right()
      {
        for (auto&& right : m_right) {
          if (right.second) {
            if (m_coordinates.col + right.first.col >= m_data->size2()) {
              right.second = boost::none;
            }
            else {
              typename input_data_type::value_type v = **(right.second);
              add_sample_to_indicator(is_weighted{}, m_indicator, v);
              ++(*right.second);
            }
          }
        }
        for (auto&& left : m_left) {
          if (left.second) {
            typename input_data_type::value_type v = **(left.second);
            subtract_sample_from_indicator(is_weighted{}, m_indicator, v);
            ++(*left.second);
          }
          else {
            coordinate_type nb = m_coordinates + left.first + coordinate_type(0, -1);
            if (is_in_range(nb.row, nb.col) ) {
              left.second = m_data->begin() + m_data->size2() * nb.row;
              typename input_data_type::value_type v = **(left.second);
              subtract_sample_from_indicator(is_weighted{}, m_indicator, v);
              ++(*left.second);
            }
          }
        }
      }

      void moved_down()
      {
        for (auto&& bottom : m_bottom) {
          if (bottom.second) {
            if (m_coordinates.row + bottom.first.row == m_data->size1()) {
              bottom.second = boost::none;
            }
            else {
              typename input_data_type::value_type v = **(bottom.second);
              auto temp = bottom.second->get_coordinates();
              assert(bottom.second->get_coordinates() == m_coordinates + bottom.first);
              add_sample_to_indicator(is_weighted{}, m_indicator_start_of_row, v);
              ++(*bottom.second);
            }
          }
        }
        for (auto&& top : m_top) {
          if (top.second) {
         
            assert(top.second->get_coordinates() == m_coordinates + top.first + coordinate_type(-1, 0));

            typename input_data_type::value_type v = **(top.second);
            subtract_sample_from_indicator(is_weighted{}, m_indicator_start_of_row, v);
            ++(*top.second);
          }
          else
          {
            coordinate_type nb = m_coordinates + top.first + coordinate_type(-1, 0);
            if (is_in_range(nb.row, nb.col) ) {
              top.second = m_data_trans.begin() + m_data->size1() * nb.col;
              
              assert(top.second->get_coordinates() == nb);
              
              typename input_data_type::value_type v = **(top.second);
              subtract_sample_from_indicator(is_weighted{}, m_indicator_start_of_row, v);
              ++(*top.second);
            }
          }
        }
        prepare_first_pixel_of_row();
      }

      void prepare_first_pixel_of_row()
      {
        m_indicator = m_indicator_start_of_row;

        nullify_iterator_vector(m_right);
        nullify_iterator_vector(m_left);

        for (auto&&r : m_right) {
          coordinate_type nb = m_coordinates + r.first + coordinate_type(0, 1);
          if (is_in_range(nb.row, nb.col)) {
            r.second = m_data->begin() + m_data->size2() * nb.row + nb.col;
          }
        }
      }

      template<class Range>
      void nullify_iterator_vector(Range& r)
      {
        for (auto&& i : r) {
          i.second = boost::none;
        }
      }

      const coordinate_type& get_coordinates() const
      {
        return m_coordinates;
      }

      const Indicator& get_indicator() const
      {
        return m_indicator;
      }

      bool is_in_range(const index_type& row, const index_type& col) const
      {
        return row >= 0 && row < m_data->size1() && col >= 0 && col < m_data->size2();
      }

      Raster* m_data;
      trans_view m_data_trans;
      double m_radius;

      iterator_vector m_left;
      iterator_vector m_right;
      trans_iterator_vector m_top;
      trans_iterator_vector m_bottom;

      coordinate_type m_coordinates;
      construction_functor<Indicator> m_initializer;

      Indicator m_indicator;
      Indicator m_indicator_start_of_row;
    };
  } // namespace moving_window
}
#endif