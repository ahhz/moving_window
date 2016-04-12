//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// The moving_window_edge_iterator provides a generalized interface for 
// circular windows of edges

#ifndef BLINK_MOVING_WINDOW_CIRCULAR_WINDOW_EDGE_ITERATOR_H_AHZ
#define BLINK_MOVING_WINDOW_CIRCULAR_WINDOW_EDGE_ITERATOR_H_AHZ

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
    class circular_window_edge_iterator
      : public boost::iterator_facade
      < circular_window_edge_iterator<Indicator, Raster, Window>
      , const Indicator, boost::forward_traversal_tag >
    {
    public:
      using coordinate_type = typename Raster::coordinate_type;
      using index_type = typename coordinate_type::index_type;
      using is_weighted = typename Raster::is_weighted;

    private:
     using  input_data_type = Raster;

     using row_major = blink::raster::orientation::row_major;
     using col_major = blink::raster::orientation::col_major;
     using v_edge = blink::raster::element::v_edge;
     using v1_edge = blink::raster::element::v_edge_first_only;
     using v2_edge = blink::raster::element::v_edge_second_only;
     using h_edge = blink::raster::element::h_edge;
     using h1_edge = blink::raster::element::h_edge_first_only;
     using h2_edge = blink::raster::element::h_edge_second_only;
     using read_only = blink::raster::access::read_only;

     using v_view    = blink::raster::raster_view<row_major, v_edge,  read_only, input_data_type>;
     using v1_view   = blink::raster::raster_view<row_major, v1_edge, read_only, input_data_type>;
     using v2_view   = blink::raster::raster_view<row_major, v2_edge, read_only, input_data_type>;
     using h_view    = blink::raster::raster_view<row_major, h_edge,  read_only, input_data_type>;
     using h1_view   = blink::raster::raster_view<row_major, h1_edge, read_only, input_data_type>;
     using h2_view   = blink::raster::raster_view<row_major, h2_edge, read_only, input_data_type>;
     using v_t_view  = blink::raster::raster_view<col_major, v_edge,  read_only, input_data_type>;
     using v1_t_view = blink::raster::raster_view<col_major, v1_edge, read_only, input_data_type>;
     using v2_t_view = blink::raster::raster_view<col_major, v2_edge, read_only, input_data_type>;
     using h_t_view  = blink::raster::raster_view<col_major, h_edge,  read_only, input_data_type>;
     using h1_t_view = blink::raster::raster_view<col_major, h1_edge, read_only, input_data_type>;
     using h2_t_view = blink::raster::raster_view<col_major, h2_edge, read_only, input_data_type>;

     using v_iterator = typename v_view::iterator;
     using v1_iterator = typename v1_view::iterator;
     using v2_iterator = typename v2_view::iterator;
     using h_iterator = typename  h_view::iterator;
     using h1_iterator = typename h1_view::iterator;
     using h2_iterator = typename h2_view::iterator;

     using v_t_iterator = typename  v_t_view::iterator;
     using v1_t_iterator = typename v1_t_view::iterator;
     using v2_t_iterator = typename v2_t_view::iterator;
     using h_t_iterator = typename  h_t_view::iterator;
     using h1_t_iterator = typename h1_t_view::iterator;
     using h2_t_iterator = typename h2_t_view::iterator;

     using opt_v_iterator = boost::optional<v_iterator>;
     using opt_v1_iterator = boost::optional<v1_iterator>;
     using opt_v2_iterator = boost::optional<v2_iterator>;
     using opt_h_iterator = boost::optional<h_iterator>;
     using opt_h1_iterator = boost::optional<h1_iterator>;
     using opt_h2_iterator = boost::optional<h2_iterator>;
     using opt_v_t_iterator = boost::optional<v_t_iterator>;
     using opt_v1_t_iterator = boost::optional<v1_t_iterator>;
     using opt_v2_t_iterator = boost::optional<v2_t_iterator>;
     using opt_h_t_iterator = boost::optional<h_t_iterator>;
     using opt_h1_t_iterator = boost::optional<h1_t_iterator>;
     using opt_h2_t_iterator = boost::optional<h2_t_iterator>;

     // .first gives the offset relative to the current position of the iterator
     // .second gives the iterator at the offset position. Provided it is within the raster.
     using ct = coordinate_type;
     using v_iterator_vector = std::vector<std::pair<ct, opt_v_iterator> >;
     using v1_iterator_vector = std::vector<std::pair<ct, opt_v1_iterator> >;
     using v2_iterator_vector = std::vector<std::pair<ct, opt_v2_iterator> >;
     using h_iterator_vector = std::vector<std::pair<ct, opt_h_iterator> >;
     using h1_iterator_vector = std::vector<std::pair<ct, opt_h1_iterator> >;
     using h2_iterator_vector = std::vector<std::pair<ct, opt_h2_iterator> >;
     using v_t_iterator_vector = std::vector<std::pair<ct, opt_v_t_iterator> >;
     using v1_t_iterator_vector = std::vector<std::pair<ct, opt_v1_t_iterator>>;
     using v2_t_iterator_vector = std::vector<std::pair<ct, opt_v2_t_iterator>>;
     using h_t_iterator_vector = std::vector<std::pair<ct, opt_h_t_iterator> >;
     using h1_t_iterator_vector = std::vector<std::pair<ct, opt_h1_t_iterator>>;
     using h2_t_iterator_vector = std::vector<std::pair<ct, opt_h2_t_iterator>>;
   
    public:
      template<typename Initializer = default_construction_functor<Indicator> >
      circular_window_edge_iterator(Window& window, Raster* data
        , Initializer initializer = Initializer()) : m_data(data)
        , m_data_trans(data)
        , m_v_view(data), m_v1_view(data), m_v2_view(data), m_h_view(data), m_h1_view(data), m_h2_view(data)
        , m_v_t_view(data), m_v1_t_view(data), m_v2_t_view(data)
        , m_h_t_view(data), m_h1_t_view(data), m_h2_t_view(data)
        , m_initializer(initializer)
        , m_indicator(initializer())
        , m_indicator_start_of_row(initializer())
        , m_coordinates(0, 0), m_radius(window.get_radius())
      {
        int radius_squared = static_cast<int>(m_radius * m_radius);
        for (int a = -int_radius - 1; a <= int_radius + 1; ++a) {
          for (int b = -int_radius - 1; b <= int_radius + 1; ++b) {
            coordinate_type centre(a, b);
            coordinate_type left(a, b - 1);
            coordinate_type topleft(a - 1, b - 1);
            coordinate_type top(a - 1, b);
            coordinate_type topright(a - 1, b + 1);
            coordinate_type right(a, b + 1);
            coordinate_type bottom(a + 1, b);
            coordinate_type bottomleft(a + 1, b - 1);
            coordinate_type left2(a, b - 2);
            coordinate_type top2(a - 2, b);

            bool is_h = (square(centre)) <= radius_squared && (square(top) <= radius_squared);
            bool is_h_right = (square(right)) <= radius_squared && (square(topright) <= radius_squared);
            bool is_h_left = (square(left)) <= radius_squared && (square(topleft) <= radius_squared);
            bool is_h_top = (square(top)) <= radius_squared && (square(top2) <= radius_squared);
            bool is_h_bottom = (square(bottom)) <= radius_squared && (square(centre) <= radius_squared);

            bool is_h1 = !(square(centre)) <= radius_squared && (square(top) <= radius_squared);
            bool is_h1_right = !(square(right)) <= radius_squared && (square(topright) <= radius_squared);
            bool is_h1_left = !(square(left)) <= radius_squared && (square(topleft) <= radius_squared);
            bool is_h1_top = !(square(top)) <= radius_squared && (square(top2) <= radius_squared);
            bool is_h1_bottom = !(square(bottom)) <= radius_squared && (square(centre) <= radius_squared);

            bool is_h2 = (square(centre)) <= radius_squared && !(square(top) <= radius_squared);
            bool is_h2_right = (square(right)) <= radius_squared && !(square(topright) <= radius_squared);
            bool is_h2_left = (square(left)) <= radius_squared && !(square(topleft) <= radius_squared);
            bool is_h2_top = (square(top)) <= radius_squared && !(square(top2) <= radius_squared);
            bool is_h2_bottom = (square(bottom)) <= radius_squared && !(square(centre) <= radius_squared);

            bool is_v = (square(centre)) <= radius_squared && (square(left) <= radius_squared);
            bool is_v_right = (square(right)) <= radius_squared && (square(centre) <= radius_squared);
            bool is_v_left = (square(left)) <= radius_squared && (square(left2) <= radius_squared);
            bool is_v_top = (square(top)) <= radius_squared && (square(topleft) <= radius_squared);
            bool is_v_bottom = (square(bottom)) <= radius_squared && (square(bottomleft) <= radius_squared);

            bool is_v1 = !(square(centre)) <= radius_squared && (square(left) <= radius_squared);
            bool is_v1_right = !(square(right)) <= radius_squared && (square(centre) <= radius_squared);
            bool is_v1_left = !(square(left)) <= radius_squared && (square(left2) <= radius_squared);
            bool is_v1_top = !(square(top)) <= radius_squared && (square(topleft) <= radius_squared);
            bool is_v1_bottom = !(square(bottom)) <= radius_squared && (square(bottomleft) <= radius_squared);

            bool is_v2 = (square(centre)) <= radius_squared && !(square(left) <= radius_squared);
            bool is_v2_right = (square(right)) <= radius_squared && !(square(centre) <= radius_squared);
            bool is_v2_left = (square(left)) <= radius_squared && !(square(left2) <= radius_squared);
            bool is_v2_top = (square(top)) <= radius_squared && !(square(topleft) <= radius_squared);
            bool is_v2_bottom = (square(bottom)) <= radius_squared && !(square(bottomleft) <= radius_squared);

            if (is_h) {
              if (!is_h_left) m_h_left.emplace_back(coordinate_type(a, b));
              if (!is_h_right)m_h_right.emplace_back(coordinate_type(a, b));
              if (!is_h_top) m_h_t_top.emplace_back(coordinate_type(a, b));
              if (!is_h_bottom)m_h_t_bottom.emplace_back(coordinate_type(a, b));
            }
            else if (is_h1) {
              if (!is_h1_left) m_h1_left.emplace_back(coordinate_type(a, b));
              if (!is_h1_right)m_h1_right.emplace_back(coordinate_type(a, b));
              if (!is_h1_top) m_h1_t_top.emplace_back(coordinate_type(a, b));
              if (!is_h1_bottom)m_h1_t_bottom.emplace_back(coordinate_type(a, b));
            }
            else if (is_h2) {
              if (!is_h2_left) m_h2_left.emplace_back(coordinate_type(a, b));
              if (!is_h2_right)m_h2_right.emplace_back(coordinate_type(a, b));
              if (!is_h2_top) m_h2_t_top.emplace_back(coordinate_type(a, b));
              if (!is_h2_bottom)m_h2_t_bottom.emplace_back(coordinate_type(a, b));
            }

            if (is_v) {
              if (!is_v_left) m_v_left.emplace_back(coordinate_type(a, b));
              if (!is_v_right)m_v_right.emplace_back(coordinate_type(a, b));
              if (!is_v_top) m_v_t_top.emplace_back(coordinate_type(a, b));
              if (!is_v_bottom)m_v_t_bottom.emplace_back(coordinate_type(a, b));
            }
            else if (is_v1) {
              if (!is_v1_left) m_v1_left.emplace_back(coordinate_type(a, b));
              if (!is_v1_right)m_v1_right.emplace_back(coordinate_type(a, b));
              if (!is_v1_top) m_v1_t_top.emplace_back(coordinate_type(a, b));
              if (!is_v1_bottom)m_v1_t_bottom.emplace_back(coordinate_type(a, b));
            }
            else if (is_v2) {
              if (!is_v2_left) m_v2_left.emplace_back(coordinate_type(a, b));
              if (!is_v2_right)m_v2_right.emplace_back(coordinate_type(a, b));
              if (!is_v2_top) m_v2_t_top.emplace_back(coordinate_type(a, b));
              if (!is_v2_bottom)m_v2_t_bottom.emplace_back(coordinate_type(a, b));
            }
          }
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

      bool equal(const circular_window_edge_iterator& that) const
      {
        return m_coordinates == that.m_coordinates;
      }

      const Indicator& dereference() const
      {
        return get_indicator();
      }

    private:
      index_type square(coordinate_type& c)
      {
        return c.row * c.row + c.col * c.col;
      }
      
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

      input_data_type* m_Data;
      double m_radius;

      v_view m_v_view;
      v1_view m_v1_view;
      v2_view m_v2_view;
      
      h_view m_h_view;
      h1_view m_h1_view;
      h2_view m_h2_view;
      
      v_t_view m_v_t_view;
      v1_t_view m_v1_t_view;
      v2_t_view m_v2_t_view;
      
      h_t_view m_h_t_view;
      h1_t_view m_h1_t_view;
      h2_t_view m_h2_t_view;

      v_iterator_vector m_v_left;
      v1_iterator_vector m_v1_left;
      v2_iterator_vector m_v2_left;
      
      h_iterator_vector m_h_left;
      h1_iterator_vector m_h1_left;
      h2_iterator_vector m_h2_left;
      
      v_iterator_vector m_v_right;
      v1_iterator_vector m_v1_right;
      v2_iterator_vector m_v2_right;

      h_iterator_vector m_h_right;
      h1_iterator_vector m_h1_right;
      h2_iterator_vector m_h2_right;

      v_t_iterator_vector m_v_top;
      v1_t_iterator_vector m_v1_top;
      v2_t_iterator_vector m_v2_top;

      h_t_iterator_vector m_h_top;
      h1_t_iterator_vector m_h1_top;
      h2_t_iterator_vector m_h2_top;

      v_t_iterator_vector m_v_bottom;
      v1_t_iterator_vector m_v1_bottom;
      v2_t_iterator_vector m_v2_bottom;

      h_t_iterator_vector m_h_bottom;
      h1_t_iterator_vector m_h1_bottom;
      h2_t_iterator_vector m_h2_bottom;

      coordinate_type m_coordinates;
      construction_functor<Indicator> m_initializer;

      Indicator m_indicator;
      Indicator m_indicator_start_of_row;
    };
  } // namespace moving_window
}
#endif