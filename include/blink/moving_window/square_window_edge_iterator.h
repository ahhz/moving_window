//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// The square_window_edge_iterator provides a generalized interface for 
// square moving window iterators of edge based indicators
// TODO: Turn this into a RectangleWindowIteratory
// TODO: Debug for case where radius >= rows or cols


#ifndef BLINK_MOVING_WINDOW_SQUARE_WINDOW_EDGE_ITERATOR_H_AHZ
#define BLINK_MOVING_WINDOW_SQUARE_WINDOW_EDGE_ITERATOR_H_AHZ

#include <blink/moving_window/indicator_input_raster.h>
#include <blink/moving_window/default_construction_functor.h>

#include <blink/raster/raster_view.h>

#include <boost/none_t.hpp>
#include <boost/optional.hpp>

#include <algorithm> // min max
#include <vector>

namespace blink {
  namespace moving_window {

    template<class Indicator, class IndicatorInputRaster>
    class square_window_edge_iterator
      : public boost::iterator_facade
      < square_window_edge_iterator<Indicator, IndicatorInputRaster>
      , const Indicator
      , boost::forward_traversal_tag >
    {
      const static bool do_outfacing = true;
  
      using this_type = square_window_edge_iterator < Indicator, IndicatorInputRaster > ;
      using buf_iter_type = typename std::vector<Indicator>::iterator;
      using input_data_type = IndicatorInputRaster;
      using is_weighted = typename IndicatorInputRaster::is_weighted;

      // shorthand 
      using row_major = blink::raster::orientation::row_major;
      using v_edge = blink::raster::element::v_edge;
      using v1_edge = blink::raster::element::v_edge_first_only;
      using v2_edge = blink::raster::element::v_edge_second_only;
      using h_edge = blink::raster::element::h_edge;
      using h1_edge = blink::raster::element::h_edge_first_only;
      using h2_edge = blink::raster::element::h_edge_second_only;
      using read_only = blink::raster::access::read_only;

      using v_view = blink::raster::raster_view < row_major, v_edge, read_only, input_data_type > ;
      using v1_view = blink::raster::raster_view < row_major, v1_edge, read_only, input_data_type > ;
      using v2_view = blink::raster::raster_view < row_major, v2_edge, read_only, input_data_type > ;
      using h_view = blink::raster::raster_view < row_major, h_edge, read_only, input_data_type > ;
      using h1_view = blink::raster::raster_view < row_major, h1_edge, read_only, input_data_type > ;
      using h2_view = blink::raster::raster_view < row_major, h2_edge, read_only, input_data_type > ;

      using v_iterator = typename  v_view::iterator;
      using v1_iterator = typename v1_view::iterator;
      using v2_iterator = typename v2_view::iterator;
      using h_iterator = typename  h_view::iterator;
      using h1_iterator = typename h1_view::iterator;
      using h2_iterator = typename h2_view::iterator;

    public:
      using coordinate_type = typename IndicatorInputRaster::coordinate_type;
      using index_type = typename IndicatorInputRaster::index_type;

      template<typename Squarewindow,
        typename Initializer = default_construction_functor<Indicator> >
        square_window_edge_iterator(Squarewindow window,
        IndicatorInputRaster* data
        , Initializer initializer = Initializer())
        : m_data(data), m_v_view(data), m_v1_view(data), m_v2_view(data)
        , m_h_view(data), m_h1_view(data), m_h2_view(data)
        , m_radius(window.get_radius()), m_initializer(initializer)
      {
      }

      square_window_edge_iterator(const square_window_edge_iterator&) = delete;

      square_window_edge_iterator& operator=(const square_window_edge_iterator&)
        = delete;

      square_window_edge_iterator(square_window_edge_iterator&& that)
        : m_radius(that.m_radius)
        , m_v_view(that.m_v_view)
        , m_v1_view(that.m_v1_view)
        , m_v2_view(that.m_v2_view)
        , m_h_view(that.m_h_view)
        , m_h1_view(that.m_h1_view)
        , m_h2_view(that.m_h2_view)
        , m_data(that.m_data)
        , m_initializer(std::move(that.m_initializer))
        , m_coordinates(that.m_coordinates)
        , m_h_buffer(std::move(that.m_h_buffer))
        , m_v2_buffer(std::move(that.m_v2_buffer))
        , m_v1_buffer(std::move(that.m_v1_buffer))
        , m_v_buffer(std::move(that.m_v_buffer))
        , m_indicator(std::move(that.m_indicator))
        , m_add_h_buffer_iterator(that.m_add_h_buffer_iterator)
        , m_subtract_h_buffer_iterator(that.m_subtract_h_buffer_iterator)
        , m_add_v_buffer_iterator(that.m_add_v_buffer_iterator)
        , m_subtract_v_buffer_iterator(that.m_subtract_v_buffer_iterator)
        , m_add_v2_buffer_iterator(that.m_add_v2_buffer_iterator)
        , m_subtract_v2_buffer_iterator(that.m_subtract_v2_buffer_iterator)
        , m_add_v1_buffer_iterator(that.m_add_v1_buffer_iterator)
        , m_subtract_v1_buffer_iterator(that.m_subtract_v1_buffer_iterator)
        , m_add_h_iterator(that.m_add_h_iterator)
        , m_subtract_h_iterator(that.m_subtract_h_iterator)
        , m_add_v_iterator(that.m_add_v_iterator)
        , m_subtract_v_iterator(that.m_subtract_v_iterator)
        , m_v2_iterator(that.m_v2_iterator)
        , m_v1_iterator(that.m_v1_iterator)
        , m_top_right_iterator(that.m_top_right_iterator)
        , m_top_left_iterator(that.m_top_left_iterator)
        , m_bottom_left_iterator(that.m_bottom_left_iterator)
        , m_bottom_right_iterator(that.m_bottom_right_iterator)
      {}

      index_type size1() const
      {
        return m_data->size1();
      }
      index_type size2() const
      {
        return m_data->size2();
      }

      void find_end()
      {
        m_coordinates = coordinate_type(size1(), 0);
      }

      void find_begin()
      {
        m_coordinates = coordinate_type{ 0, 0 };

        // Initialize buffer of horizontal edges for each column
        m_h_buffer.resize(size2(), m_initializer());

        // Initialize 3 buffers of vertical edges for each column
        m_v_buffer.resize(size2() + 1, m_initializer());
        m_v1_buffer.resize(size2() + 1, m_initializer());
        m_v2_buffer.resize(size2() + 1, m_initializer());

        m_add_h_buffer_iterator = m_h_view.begin() + size2();// Skip first line of h2_edges

        for (index_type row = 1; row <= m_radius; ++row) {
          add_h_row_to_buffer();
        }

        m_add_v_buffer_iterator = m_v_view.begin();
        m_add_v1_buffer_iterator = m_v1_view.begin();
        m_add_v2_buffer_iterator = m_v2_view.begin();

        for (index_type row = 0; row <= m_radius; ++row) {
          add_v_row_to_buffer();
        }
        m_subtract_h_buffer_iterator = boost::none;
        m_subtract_v_buffer_iterator = boost::none;
        m_subtract_v2_buffer_iterator = boost::none;
        m_subtract_v1_buffer_iterator = boost::none;

        prepare_first_pixel_of_row();
      }

    private:
      friend class boost::iterator_core_access;

      void  increment()
      {
        if (++m_coordinates.col != size2()) {
          return moved_right();
        }
        m_coordinates.col = 0;
        if (++m_coordinates.row != size1()) {
          return moved_down();
        }
      }

      bool equal(const this_type& that) const
      {
        return m_coordinates == that.m_coordinates;
      }

      const Indicator& dereference() const
      {
        return get_indicator();
      }

    private:
      void add_h_row_to_buffer()
      {
        if (m_add_h_buffer_iterator) {
          auto zip = blink::iterator::make_zip_range(
            std::ref(m_h_buffer),
            blink::iterator::make_zip_along_range(*m_add_h_buffer_iterator));
          for (auto&& z : zip) {
            typename h_iterator::value_type v = std::get<1>(z);
            add_sample_to_indicator(is_weighted{}, std::get<0>(z), v);
          }
          if (get_coordinates().row + m_radius + 1 == size1()) {
            m_add_h_buffer_iterator = boost::none;
          }
        }
      }

      void subtract_h_row_from_buffer()
      {
        if (m_subtract_h_buffer_iterator) {

          auto zip = blink::iterator::make_zip_range(
            std::ref(m_h_buffer),
            blink::iterator::make_zip_along_range(*m_subtract_h_buffer_iterator));

          for (auto&& z : zip) {
            typename h_iterator::value_type v = std::get<1>(z);
            subtract_sample_from_indicator(is_weighted{}, std::get<0>(z), v);
          }
        }
        else if (m_coordinates.row == m_radius) {
          m_subtract_h_buffer_iterator = m_h_view.begin() + size2(); // next time round, start at second line.
        }
      }

      void prepare_first_pixel_of_row()
      {
        m_add_h_iterator = m_h_buffer.begin();
        m_add_v_iterator = m_v_buffer.begin();

        m_indicator = m_initializer();

        for (int col = 0; col <= m_radius; ++col) {
          add_h_to_indicator();
          add_v_to_indicator();
        }
        m_subtract_h_iterator = boost::none;
        m_subtract_v_iterator = boost::none;

         if (do_outfacing) {
          m_v2_iterator = m_v2_buffer.begin();
          m_v1_iterator = m_v1_buffer.begin() + (m_radius + 1);

          m_indicator.add_subtotal(**m_v2_iterator);
          m_indicator.add_subtotal(**m_v1_iterator);

          m_v2_iterator = boost::none;
       
          index_type bottom_row = std::min<index_type>(m_coordinates.row + m_radius + 1, size1());
          index_type top_row = std::max<index_type>(0, m_coordinates.row - m_radius);

          m_bottom_right_iterator = m_h1_view.begin() + size2() * bottom_row;
          m_top_right_iterator = m_h2_view.begin() + size2() * top_row;

          for (int col = 0; col <= m_radius; ++col) {
            add_bottom_right();
            add_top_right();
          }
          m_top_left_iterator = boost::none;
          m_bottom_left_iterator = boost::none;
        }
      }

      void add_bottom_right()
      {
        if (m_bottom_right_iterator) {
          m_bottom_right_iterator->add_to_indicator(m_indicator);
          if (m_bottom_right_iterator->get_coordinates().col + 1 == size2()){
            m_bottom_right_iterator = boost::none;
          }
          else {
            ++(*m_bottom_right_iterator);
          }
        }
      }

      void add_top_right()
      {
        if (m_top_right_iterator) {
          m_top_right_iterator->add_to_indicator(m_indicator);
          if (m_top_right_iterator->get_coordinates().col + 1 == size2()){
            m_top_right_iterator = boost::none;
          }
          else {
            ++(*m_top_right_iterator);
          }
        }
      }
      void subtract_top_left()
      {
        if (m_top_left_iterator) {
          m_top_left_iterator->subtract_from_to_indicator(m_indicator);
          ++(*m_top_left_iterator);
        }
        else if (get_coordinates().col == m_radius) {
          index_type top_row = std::max<index_type>(0, m_coordinates.row - m_radius);
          m_top_left_iterator = m_h2_view.begin() + size2() * top_row;
        }
      }

      void subtract_bottom_left()
      {
        if (m_bottom_left_iterator) {
          m_bottom_left_iterator->subtract_from_to_indicator(m_indicator);
          ++(*m_bottom_left_iterator);
        }
        else if (get_coordinates().col == m_radius) {
          index_type bottom_row = std::min<index_type>(m_coordinates.row + m_radius + 1, size1());
          m_bottom_left_iterator = m_h1_view.begin() + size2() * bottom_row;
        }
      }

      void add_v_row_to_buffer()
      {
        if (m_add_v_buffer_iterator) {

          auto zip = blink::iterator::make_zip_range(
            std::ref(m_v_buffer),  //0
            std::ref(m_v1_buffer),  //1
            std::ref(m_v2_buffer),  //2
            blink::iterator::make_zip_along_range(*m_add_v_buffer_iterator), //3
            blink::iterator::make_zip_along_range(*m_add_v1_buffer_iterator),//4
            blink::iterator::make_zip_along_range(*m_add_v2_buffer_iterator));//5

          int count = 0;
          for (auto&& z : zip)
          {
            if (count == 0) // first col
            {
              typename v2_iterator::value_type v2 = std::get<5>(z);
              add_sample_to_indicator(is_weighted{}, std::get<2>(z), v2);
            }
            else if (count == size2()) // last col
            {
              typename v1_iterator::value_type v1 = std::get<4>(z);
              add_sample_to_indicator(is_weighted{}, std::get<1>(z), v1);
             }
            else
            {
              typename v_iterator::value_type v = std::get<3>(z);
              typename v1_iterator::value_type v1 = std::get<4>(z);
              typename v2_iterator::value_type v2 = std::get<5>(z);
              add_sample_to_indicator(is_weighted{}, std::get<0>(z), v);
              add_sample_to_indicator(is_weighted{}, std::get<1>(z), v1);
              add_sample_to_indicator(is_weighted{}, std::get<2>(z), v2);
            }
            ++count;
          }
          if (*m_add_v_buffer_iterator == m_v_view.end()){
            m_add_v_buffer_iterator = boost::none;
            m_add_v1_buffer_iterator = boost::none;
            m_add_v2_buffer_iterator = boost::none;
          }
        }
      }

      void subtract_v_row_from_buffer()
      {
        if (m_subtract_v_buffer_iterator) {
          auto zip = blink::iterator::make_zip_range(
            std::ref(m_v_buffer),  //0
            std::ref(m_v1_buffer),  //1
            std::ref(m_v2_buffer),  //2
            blink::iterator::make_zip_along_range(*m_subtract_v_buffer_iterator), //3
            blink::iterator::make_zip_along_range(*m_subtract_v1_buffer_iterator),//4
            blink::iterator::make_zip_along_range(*m_subtract_v2_buffer_iterator));//5

          int count = 0;
          for (auto&& z : zip)
          {
            if (count == 0) // first col
            {
              typename v2_iterator::value_type v2 = std::get<5>(z);
              subtract_sample_from_indicator(is_weighted{}, std::get<2>(z), v2);
            }
            else if (count == size2()) // last col
            {
              typename v1_iterator::value_type v1 = std::get<4>(z);
              subtract_sample_from_indicator(is_weighted{}, std::get<1>(z), v1);
            }
            else
            {
              typename v_iterator::value_type v = std::get<3>(z);
              typename v1_iterator::value_type v1 = std::get<4>(z);
              typename v2_iterator::value_type v2 = std::get<5>(z);
              subtract_sample_from_indicator(is_weighted{}, std::get<0>(z), v);
              subtract_sample_from_indicator(is_weighted{}, std::get<1>(z), v1);
              subtract_sample_from_indicator(is_weighted{}, std::get<2>(z), v2);
          }
            ++count;
          }
        }
        else if (get_coordinates().row == m_radius){
          m_subtract_v_buffer_iterator = m_v_view.begin();
          m_subtract_v1_buffer_iterator = m_v1_view.begin();
          m_subtract_v2_buffer_iterator = m_v2_view.begin();
        }
      }

      template<typename Iter>
      void left_subtract(boost::optional<Iter>& iter, const Iter& begin, const index_type& offset)
      {
        if (iter) {
          ++(*iter);
          m_indicator.subtract_subtotal(**iter);
        }
        else if (m_coordinates.col == offset)	{
          iter = begin;// + m_coordinates.col - m_radius - 1;
          m_indicator.subtract_subtotal(**iter);
        }
      }

      void subtract_h_from_indicator()
      {
        left_subtract(m_subtract_h_iterator, m_h_buffer.begin(), m_radius + 1);
      }
          
      void subtract_v_from_indicator()
      {
        left_subtract(m_subtract_v_iterator, m_v_buffer.begin(), m_radius);
      }

      template<typename Iter>
      void move_left_border(boost::optional<Iter>& iter, Iter& begin, const index_type& offset)
      {
        if (iter) {
          m_indicator.subtract_subtotal(**iter);
          ++(*iter);
          m_indicator.add_subtotal(**iter);
        }
        else if (m_coordinates.col == offset) {
          iter = begin;
        }
      }

      template<typename Iter>
      void move_right_border(boost::optional<Iter>& iter, Iter& end)
      {
        if (iter) {
          if (std::distance(*iter, end) == 1) {
            iter = boost::none;
          }
          else {
            m_indicator.subtract_subtotal(**iter);
            ++(*iter);
            m_indicator.add_subtotal(**iter);
          }
        }
      }

      void add_h_to_indicator()
      {
        if (m_add_h_iterator) {
          m_indicator.add_subtotal(**m_add_h_iterator);
          if (++(*m_add_h_iterator) == m_h_buffer.end()) {
            m_add_h_iterator = boost::none;
          }
        }
      }

      void add_v_to_indicator()
      {
        if (m_add_v_iterator) {
          m_indicator.add_subtotal(**m_add_v_iterator);
          if (++(*m_add_v_iterator) == m_v_buffer.end() - 1) {
            m_add_v_iterator = boost::none;
          }
        }
      }

      void moved_right() // The row index has incremented already, now catch up indicator;
      {
          // INNER
        add_h_to_indicator();
        add_v_to_indicator();
        subtract_h_from_indicator();
        subtract_v_from_indicator();
              
        // OUTER
        if (do_outfacing) {
          move_left_border(m_v2_iterator, m_v2_buffer.begin(), m_radius);
          move_right_border(m_v1_iterator, m_v1_buffer.end());

          add_bottom_right();
          add_top_right();
          subtract_bottom_left();
          subtract_top_left();
        }
      }

      void moved_down() 
      {
        add_h_row_to_buffer();
        subtract_h_row_from_buffer();

        add_v_row_to_buffer();
        subtract_v_row_from_buffer();

        prepare_first_pixel_of_row();
      }

      const coordinate_type& get_coordinates() const
      {
        return m_coordinates;
      }

      const Indicator& get_indicator() const
      {
        return m_indicator;
      }

      // Input
      index_type m_radius;

      IndicatorInputRaster* m_data;

      v_view m_v_view;
      v1_view m_v1_view;
      v2_view m_v2_view;
      h_view m_h_view;
      h1_view m_h1_view;
      h2_view m_h2_view;

      construction_functor<Indicator> m_initializer;

      // Output
      coordinate_type m_coordinates;
      Indicator m_indicator;

      // 4 Buffers that accumulate cols
      std::vector<Indicator> m_h_buffer;	// horizontal inner 
      std::vector<Indicator> m_v2_buffer;	// vertical left special
      std::vector<Indicator> m_v1_buffer;	// vertical right special
      std::vector<Indicator> m_v_buffer;	// vertical inner

      // 8 Iterators to update 4 buffers
      // These are updated when moving down
      boost::optional<h_iterator> m_add_h_buffer_iterator;
      boost::optional<h_iterator> m_subtract_h_buffer_iterator;
      boost::optional<v_iterator> m_add_v_buffer_iterator;
      boost::optional<v_iterator> m_subtract_v_buffer_iterator;
      boost::optional<v2_iterator> m_add_v2_buffer_iterator;
      boost::optional<v2_iterator> m_subtract_v2_buffer_iterator;
      boost::optional<v1_iterator> m_add_v1_buffer_iterator;
      boost::optional<v1_iterator> m_subtract_v1_buffer_iterator;

      // 4 Iterators to update inner
      // These are updated when moving right, and reset when moving down
      // TODO: Need a better name
      boost::optional<buf_iter_type> m_add_h_iterator, m_subtract_h_iterator;
      boost::optional<buf_iter_type> m_add_v_iterator, m_subtract_v_iterator;

      // 6 Iterators to update borders
      // These are updated when moving right, and reset when moving down
      boost::optional<buf_iter_type> m_v2_iterator;
      boost::optional<buf_iter_type> m_v1_iterator;
      boost::optional<h2_iterator> m_top_right_iterator, m_top_left_iterator;
      boost::optional<h1_iterator> m_bottom_left_iterator, m_bottom_right_iterator;
    };
  }  
}
#endif
