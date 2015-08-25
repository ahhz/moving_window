//
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Not for public distribution
//=======================================================================
//
// The square_window_edge_iterator provides a generalized interface for 
// square moving window iterators of edge based indicators
// TODO: Turn this into a RectangleWindowIteratory
// TODO: Debug for case where radius >= rows or cols


#ifndef SQUARE_WINDOW_EDGE_ITERATOR_H_AHZ
#define SQUARE_WINDOW_EDGE_ITERATOR_H_AHZ

#include <moving_window/indicator_input_raster.h>
#include <moving_window/default_construction_functor.h>
#include <moving_window/edge_iterator.h>
#include <moving_window/raster_iterator.h>

#include <boost/none_t.hpp>
#include <boost/optional.hpp>

#include <algorithm> // min max
#include <utility> // pair

#include <vector>

namespace moving_window {

  template<typename Indicator, typename IndicatorInputRaster>
  class square_window_edge_iterator
    : public boost::iterator_facade
    < square_window_edge_iterator<Indicator, IndicatorInputRaster>
    , const Indicator
    , boost::forward_traversal_tag>
  {

  public:
    typedef square_window_edge_iterator<Indicator, IndicatorInputRaster> this_type;

    typedef typename IndicatorInputRaster::coordinate_type coordinate_type;
    typedef typename IndicatorInputRaster::index_type index_type;

    typedef typename std::vector<Indicator>::iterator buf_iter_type;
    typedef IndicatorInputRaster input_data_type;

    // shorthand 
    typedef raster_iterator_tag::orientation::row_major row_major;
    typedef raster_iterator_tag::element::v_edge v_edge;
    typedef raster_iterator_tag::element::v_edge_first_only v1_edge;
    typedef raster_iterator_tag::element::v_edge_second_only v2_edge;
    typedef raster_iterator_tag::element::h_edge h_edge;
    typedef raster_iterator_tag::element::h_edge_first_only h1_edge;
    typedef raster_iterator_tag::element::h_edge_second_only h2_edge;
    typedef raster_iterator_tag::access::read_only read_only;

    typedef raster_view<row_major, v_edge, read_only, input_data_type>  v_view;
    typedef raster_view<row_major, v1_edge, read_only, input_data_type> v1_view;
    typedef raster_view<row_major, v2_edge, read_only, input_data_type> v2_view;
    typedef raster_view<row_major, h_edge, read_only, input_data_type>  h_view;
    typedef raster_view<row_major, h1_edge, read_only, input_data_type> h1_view;
    typedef raster_view<row_major, h2_edge, read_only, input_data_type> h2_view;

    typedef typename  v_view::iterator  v_iterator;
    typedef typename v1_view::iterator v1_iterator;
    typedef typename v2_view::iterator v2_iterator;
    typedef typename  h_view::iterator  h_iterator;
    typedef typename h1_view::iterator h1_iterator;
    typedef typename h2_view::iterator h2_iterator;

    template<typename Squarewindow,
      typename Initializer = default_construction_functor<Indicator> >
      square_window_edge_iterator(Squarewindow window,
      IndicatorInputRaster* data
      , Initializer initializer = Initializer())
      : m_data(data), m_v_view(data), m_v1_view(data), m_v2_view(data)
      , m_h_view(data), m_h1_view(data), m_h2_view(data)
      , m_radius(window.get_radius()), m_initializer(initializer)
    {
        //find_begin();
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
      m_indicator = m_initializer();

      // Clear all 4 buffers
      m_h_buffer.clear();
      m_v_buffer.clear();
      m_v2_buffer.clear();
      m_v1_buffer.clear();

      // Set all 16 iterators to boost::none
      m_add_h_buffer_iterator = boost::none;
      m_add_v_buffer_iterator = boost::none;
      m_add_v2_buffer_iterator = boost::none;
      m_add_v1_buffer_iterator = boost::none;

      m_subtract_h_buffer_iterator = boost::none;
      m_subtract_v_buffer_iterator = boost::none;
      m_subtract_v2_buffer_iterator = boost::none;
      m_subtract_v1_buffer_iterator = boost::none;

      m_add_h_iterator = boost::none;
      m_add_v_iterator = boost::none;
      m_subtract_v_iterator = boost::none;
      m_subtract_h_iterator = boost::none;

      m_top_right_iterator = boost::none;
      m_bottom_right_iterator = boost::none;
      m_top_left_iterator = boost::none;
      m_bottom_left_iterator = boost::none;
    }

    void find_begin()
    {
      m_coordinates = coordinate_type(0, 0);
      m_indicator = m_initializer();

      // Initialize buffer of horizontal edges for each column
      m_h_buffer.resize(size2(), m_initializer());
      m_add_h_buffer_iterator = m_h_view.begin();
      (*m_add_h_buffer_iterator) += size2();// Jump to the first line
      // TODO: the case when m_radius > m_Rows;
      for (index_type row = 1; row <= m_radius; ++row) {
        auto h_buffer_iterator = m_h_buffer.begin();
        for (; h_buffer_iterator != m_h_buffer.end(); ++(*m_add_h_buffer_iterator), ++(h_buffer_iterator)) {
          m_add_h_buffer_iterator->add_to_indicator(*h_buffer_iterator);
        }
      }

      // Initialize 3 buffers of vertical edges for each column
      m_v_buffer.resize(size2() + 1, m_initializer());
      m_v2_buffer.resize(size2() + 1, m_initializer());
      m_v1_buffer.resize(size2() + 1, m_initializer());
      m_add_v_buffer_iterator = m_v_view.begin();
      m_add_v1_buffer_iterator = m_v1_view.begin();
      m_add_v2_buffer_iterator = m_v2_view.begin();

      // TODO: the case when m_radius > m_Rows;
      for (index_type row = 0; row <= m_radius; ++row) {

        auto v_buffer_iterator = m_v_buffer.begin();
        auto v2_buffer_iterator = m_v2_buffer.begin();
        auto v1_buffer_iterator = m_v1_buffer.begin();
        for (; v_buffer_iterator != m_v_buffer.end();
          ++(*m_add_v_buffer_iterator), ++(*m_add_v1_buffer_iterator), ++(*m_add_v2_buffer_iterator),
          ++v_buffer_iterator, ++v2_buffer_iterator, ++v1_buffer_iterator)
        {
          bool first_col = m_add_v_buffer_iterator->get_coordinates().col == 0;
          bool last_col = m_add_v_buffer_iterator->get_coordinates().col == size2(); // equal because there goes one more edge in a row than pixels

          // What if only one col, or zero?
          if (first_col) {
            m_add_v2_buffer_iterator->add_to_indicator(*v2_buffer_iterator);

          }
          else if (last_col) {
            m_add_v1_buffer_iterator->add_to_indicator(*v1_buffer_iterator);
          }
          else {
            m_add_v_buffer_iterator->add_to_indicator(*v_buffer_iterator);
            m_add_v1_buffer_iterator->add_to_indicator(*v1_buffer_iterator);
            m_add_v2_buffer_iterator->add_to_indicator(*v2_buffer_iterator);
          }
        }
      }

      m_subtract_h_buffer_iterator = boost::none;
      m_subtract_v_buffer_iterator = boost::none;
      m_subtract_v2_buffer_iterator = boost::none;
      m_subtract_v1_buffer_iterator = boost::none;

      m_add_h_iterator = m_h_buffer.begin();
      m_add_v_iterator = m_v_buffer.begin();

      // TODO: the case when m_radius > m_Cols;
      for (int col = 0; col <= m_radius; ++col, ++(*m_add_h_iterator), ++(*m_add_v_iterator)) {
        m_indicator.add_subtotal(**m_add_h_iterator);
        m_indicator.add_subtotal(**m_add_v_iterator);
      }
      m_subtract_h_iterator = boost::none;
      m_subtract_v_iterator = boost::none;

      // All the inner edges are included now. Remain the external edges.

      m_v2_iterator = m_v2_buffer.begin();
      m_v1_iterator = m_v1_buffer.begin() + (m_radius + 1);

      m_indicator.add_subtotal(**m_v1_iterator);
      m_indicator.add_subtotal(**m_v2_iterator);

      m_v2_iterator = boost::none;

      m_top_left_iterator = boost::none;
      m_bottom_left_iterator = boost::none;
      m_top_right_iterator = m_h2_view.begin();

      m_bottom_right_iterator = m_h1_view.begin() + (m_radius + 1)*size2();

      // TODO: the case when m_radius > m_Cols;
      for (int col = 0; col <= m_radius; ++col, ++(*m_bottom_right_iterator), ++(*m_top_right_iterator))
      {
        m_bottom_right_iterator->add_to_indicator(m_indicator);
        m_top_right_iterator->add_to_indicator(m_indicator);
      }
      //m_top_right_iterator = boost::none;
    }

  private:
    friend class boost::iterator_core_access;

    void  increment()
    {
      if (++m_coordinates.col != size2()) {
        moved_right();
      }
      else if (++m_coordinates.row != size1()) {
        m_coordinates.col = 0;
        moved_down();
      }
      else {
        find_end();

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
    template<typename Iter>
    void right_add(boost::optional<Iter>& iter, const Iter& end)
    {
      if (iter) {
        m_indicator.add_subtotal(**iter);
        if (++(*iter) == end) {
          iter = boost::none;
        }
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
        //	m_indicator.subtract_sample(*iter);
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

    void moved_right() // The row index has incremented already, now catch up indicator;
    {
      if (get_coordinates().col == size2() - 1)
      {
        bool breakhere = true;
      }
      // INNER
      // add horizontal edges at x + radius
      right_add(m_add_h_iterator, m_h_buffer.end());

      // add vertical edges at left of x + radius								
      right_add(m_add_v_iterator, m_v_buffer.begin() + (m_v_buffer.size() - 1));

      // subtract horizontal edges at x - radius - 1
      left_subtract(m_subtract_h_iterator, m_h_buffer.begin(), m_radius + 1);

      // subtract vertical edges at left of x - radius								
      left_subtract(m_subtract_v_iterator, m_v_buffer.begin(), m_radius);

      // OUTER
      //add vertical edges (left special) left of  x - radius
      move_left_border(m_v2_iterator, m_v2_buffer.begin(), m_radius);
      move_right_border(m_v1_iterator, m_v1_buffer.end());

      // Corners
      if (m_top_left_iterator) {
        m_top_left_iterator->subtract_from_to_indicator(m_indicator);
        ++(*m_top_left_iterator);
      }
      else if (get_coordinates().col == m_radius) {
        index_type toprow = std::max<index_type>(0, get_coordinates().row - m_radius);
        m_top_left_iterator = m_h2_view.begin() + size2() * toprow;
      }

      if (m_bottom_left_iterator) {
        m_bottom_left_iterator->subtract_from_to_indicator(m_indicator);
        ++(*m_bottom_left_iterator);
      }
      else if (get_coordinates().col == m_radius) {
        index_type bottomrow = std::min<index_type>(size1(), get_coordinates().row + m_radius + 1);
        m_bottom_left_iterator = m_h1_view.begin() + size2()*bottomrow;
      }

      if (m_top_right_iterator) {
        m_top_right_iterator->add_to_indicator(m_indicator);
        if (m_top_right_iterator->get_coordinates().col + 1 == size2()){
          m_top_right_iterator = boost::none;
        }
        else {
          ++(*m_top_right_iterator);
        }
      }

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

    void moved_down() // The col index has incremented already, now catch up indicator;
    {
      if (get_coordinates().row == size1() - 1) {
        bool breakhere = true;
      }
      // update 4 buffers (TODO: this contains a lot of code duplication)
      if (m_add_h_buffer_iterator) {
        auto h_buffer_iterator = m_h_buffer.begin();
        for (; h_buffer_iterator != m_h_buffer.end(); ++(*m_add_h_buffer_iterator), ++h_buffer_iterator) {
          m_add_h_buffer_iterator->add_to_indicator(*h_buffer_iterator);
        }
        if (get_coordinates().row + m_radius + 1 == size1()) {
          m_add_h_buffer_iterator = boost::none;
        }
      }
      if (m_subtract_h_buffer_iterator) {
        auto h_buffer_iterator = m_h_buffer.begin();
        for (; h_buffer_iterator != m_h_buffer.end(); ++(*m_subtract_h_buffer_iterator), ++h_buffer_iterator) {
          m_subtract_h_buffer_iterator->subtract_from_to_indicator(*h_buffer_iterator);
        }
      }
      else if (m_coordinates.row == m_radius) {
        m_subtract_h_buffer_iterator = m_h_view.begin() + size2(); // next time round, start at second line.
      }
      if (m_add_v_buffer_iterator) {
        auto v_buffer_iterator = m_v_buffer.begin();
        auto v2_buffer_iterator = m_v2_buffer.begin();
        auto v1_buffer_iterator = m_v1_buffer.begin();
        for (; v_buffer_iterator != m_v_buffer.end();
          ++(*m_add_v_buffer_iterator), ++(*m_add_v1_buffer_iterator), ++(*m_add_v2_buffer_iterator),
          ++v_buffer_iterator, ++v2_buffer_iterator, ++v1_buffer_iterator)
        {
          bool first_col = m_add_v_buffer_iterator->get_coordinates().col == 0;
          bool last_col = m_add_v_buffer_iterator->get_coordinates().col == size2(); // equal because there goes one more edge in a row than pixels
          // What if only one col, or zero?
          if (first_col) {
            m_add_v2_buffer_iterator->add_to_indicator(*v2_buffer_iterator);

          }
          else if (last_col) {
            m_add_v1_buffer_iterator->add_to_indicator(*v1_buffer_iterator);
          }
          else {
            m_add_v_buffer_iterator->add_to_indicator(*v_buffer_iterator);
            m_add_v1_buffer_iterator->add_to_indicator(*v1_buffer_iterator);
            m_add_v2_buffer_iterator->add_to_indicator(*v2_buffer_iterator);
          }
        }
        if (*m_add_v_buffer_iterator == m_v_view.end()){
          m_add_v_buffer_iterator = boost::none;
          m_add_v1_buffer_iterator = boost::none;
          m_add_v2_buffer_iterator = boost::none;
        }
      }

      if (m_subtract_v_buffer_iterator) {
        auto v_buffer_iterator = m_v_buffer.begin();
        auto v2_buffer_iterator = m_v2_buffer.begin();
        auto v1_buffer_iterator = m_v1_buffer.begin();
        for (; v_buffer_iterator != m_v_buffer.end();
          ++(*m_subtract_v_buffer_iterator), ++(*m_subtract_v1_buffer_iterator),
          ++(*m_subtract_v2_buffer_iterator),
          ++v_buffer_iterator, ++v2_buffer_iterator, ++v1_buffer_iterator)
        {
          bool first_col = m_subtract_v_buffer_iterator->get_coordinates().col == 0;
          bool last_col = m_subtract_v_buffer_iterator->get_coordinates().col == size2(); // equal because there goes one more edge in a row than pixels
          // What if only one col, or zero?
          if (first_col) {
            m_subtract_v2_buffer_iterator->subtract_from_to_indicator(*v2_buffer_iterator);

          }
          else if (last_col) {
            m_subtract_v1_buffer_iterator->subtract_from_to_indicator(*v1_buffer_iterator);
          }
          else {
            m_subtract_v_buffer_iterator->subtract_from_to_indicator(*v_buffer_iterator);
            m_subtract_v1_buffer_iterator->subtract_from_to_indicator(*v1_buffer_iterator);
            m_subtract_v2_buffer_iterator->subtract_from_to_indicator(*v2_buffer_iterator);
          }
        }
      }
      else if (get_coordinates().row == m_radius){
        m_subtract_v_buffer_iterator = m_v_view.begin();
        m_subtract_v1_buffer_iterator = m_v1_view.begin();
        m_subtract_v2_buffer_iterator = m_v2_view.begin();
      }

      m_add_h_iterator = m_h_buffer.begin();
      m_add_v_iterator = m_v_buffer.begin();

      m_indicator = m_initializer();

      int last = std::min(static_cast<int>(m_radius), static_cast<int>(size2()) - 1);
      for (int col = 0; col <= last; ++col, ++(*m_add_h_iterator), ++(*m_add_v_iterator)) {
        m_indicator.add_subtotal(**m_add_h_iterator);
        m_indicator.add_subtotal(**m_add_v_iterator);
      }
      m_subtract_h_iterator = boost::none;
      m_subtract_v_iterator = boost::none;

      // All the inner edges are included now. Remain the external edges.

      m_v2_iterator = m_v2_buffer.begin();
      m_v1_iterator = m_v1_buffer.begin() + (m_radius + 1);

      m_indicator.add_subtotal(**m_v2_iterator);
      m_indicator.add_subtotal(**m_v1_iterator);

      m_v2_iterator = boost::none;

      m_top_left_iterator = boost::none;
      m_bottom_left_iterator = boost::none;

      index_type bottomrow = std::min<index_type>(m_coordinates.row + m_radius + 1, size1());
      index_type toprow = std::max<index_type>(0, m_coordinates.row - m_radius);

      m_bottom_right_iterator = m_h1_view.begin() + size2() * bottomrow;
      m_top_right_iterator = m_h2_view.begin() + size2() * toprow;

      // TODO: the case when m_radius > m_Cols;
      last = std::min(static_cast<int>(m_radius), static_cast<int>(size2()) - 1);
      if (m_bottom_right_iterator) {
        for (int col = 0; col <= last; ++col, ++(*m_bottom_right_iterator)) {
          m_bottom_right_iterator->add_to_indicator(m_indicator);
        }
      }
      if (m_top_right_iterator) {
        for (int col = 0; col <= last; ++col, ++(*m_top_right_iterator)) {
          m_top_right_iterator->add_to_indicator(m_indicator);
        }
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

} // namespace moving_window 
#endif
