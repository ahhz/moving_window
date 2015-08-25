//
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Not for public distribution
//=======================================================================       
//
// The square_indow_iterator provides a generalized interface for patch
// or pixel-based indicators
//
// TODO: Turn this into a RectangleWindowIterator?

#ifndef SQUARE_WINDOW_ITERATOR_H_AHZ
#define SQUARE_WINDOW_ITERATOR_H_AHZ

#include <moving_window/indicator_input_raster.h>
#include <moving_window/default_construction_functor.h>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/none_t.hpp>
#include <boost/optional.hpp>

#include <algorithm> // min max
#include <utility> // pair
#include <vector>

namespace moving_window {

  template<typename Indicator, typename IndicatorInputRaster>
  class square_window_iterator; //forward declaration

  template<typename Indicator, typename IndicatorInputRaster>
  class square_window_iterator_facade
  {
  public:
    typedef square_window_iterator<Indicator, IndicatorInputRaster> base;
    friend class base;
    typedef typename boost::iterator_facade
      < base, const Indicator, boost::forward_traversal_tag > type;
  };

  template<typename Indicator, typename IndicatorInputRaster>
  class square_window_iterator
    : public square_window_iterator_facade<Indicator, IndicatorInputRaster >::type
  {
  private:
    typedef typename IndicatorInputRaster::iterator input_iterator;
    typedef default_construction_functor<Indicator> default_initializer;

  public:
    typedef typename IndicatorInputRaster::coordinate_type coordinate_type;
    typedef typename coordinate_type::index_type index_type;

    template<typename window, typename Initializer = default_initializer>
    square_window_iterator(window& window, IndicatorInputRaster* data
      , Initializer initializer = Initializer()) : m_data(data)
      , m_radius(window.get_radius()), m_initializer(initializer)
      , m_indicator(initializer())
    {
      //find_begin();
    }

    // disallow copying
    square_window_iterator(const square_window_iterator&) = delete;

    // allow moving
    square_window_iterator(square_window_iterator&& that)
      : m_coordinates(that.m_coordinates)
      , m_data(that.m_data)
      , m_indicator(that.m_indicator)
      , m_initializer(that.m_initializer)
      , m_radius(that.m_radius)
      , m_subtract_col_iterator(that.m_subtract_col_iterator)
      , m_add_col_iterator(that.m_add_col_iterator)
      , m_subtract_row_iterator(that.m_subtract_row_iterator)
      , m_add_row_iterator(that.m_add_row_iterator)
      , m_buffer(std::move(that.m_buffer)) // this is the crux: do not copy buffer
    {}

    // disallow assigning
    square_window_iterator& operator=(const square_window_iterator&) = delete;

  private:
    friend class boost::iterator_core_access;

    void increment()
    {
      if (++m_coordinates.col == size2()) {
        m_coordinates.col = 0;
        if (++m_coordinates.row == size1()) {
          m_buffer.clear();
        }
        else {
          moved_down();
        }
      }
      else {
        moved_right();
      }
    }

    bool equal(square_window_iterator const& other) const
    {
      return m_coordinates == other.m_coordinates;
    }

    const Indicator& dereference() const
    {
      return get_indicator();
    }

  public:
    const coordinate_type& get_coordinates() const
    {
      return m_coordinates;
    }

    void find_end()
    {
      m_coordinates = coordinate_type(size1(), 0);
      m_buffer.clear();
      m_indicator = m_initializer();
      m_subtract_col_iterator = boost::none;
      m_subtract_row_iterator = boost::none;
      m_add_col_iterator = boost::none;
      m_add_row_iterator = boost::none;
    }

    void find_begin()
    {
      m_coordinates = coordinate_type(0, 0);

      m_buffer.assign(size2(), m_initializer());

      m_subtract_col_iterator = boost::none;
      m_subtract_row_iterator = boost::none;

      // Fill buffer
      const index_type endrow = std::min(size1(), m_radius + 1);

      m_add_row_iterator = begin_input();
      for (index_type row = 0; row < endrow; ++row) {
        auto i = m_buffer.begin();
        const auto end = m_buffer.end();
        for (; i != end; ++i, ++(*m_add_row_iterator)) {
          add_iterator_to_indicator(*m_add_row_iterator, *i);
        }
      }
      if (size1() <= m_radius + 1) {
        assert(m_add_row_iterator == end_input());
        m_add_row_iterator = boost::none;
      }

      // Calculate cell (0,0)
      m_add_col_iterator = m_buffer.begin();;
      const index_type endcol = std::min(size2(), m_radius + 1);
      const auto end = *m_add_col_iterator + endcol;

      m_indicator = m_initializer();
      for (; (*m_add_col_iterator) != end; ++(*m_add_col_iterator)) {
        m_indicator.add_subtotal(**m_add_col_iterator);
      }
      if (size2() <= m_radius + 1) {
        assert((*m_add_col_iterator) == m_buffer.end());
        m_add_col_iterator = boost::none;
      }
    }
  private:
    const Indicator& get_indicator() const
    {
      return m_indicator;
    }

    index_type size1() const
    {
      return m_data->size1();
    }

    index_type size2() const
    {
      return m_data->size2();
    }

    // The col index has incremented already, now catch up the indicator;
    void moved_right()
    {
      // Once we stopped adding, it won't start again
      if (m_add_col_iterator) {
        m_indicator.add_subtotal(**m_add_col_iterator);
        if (++(*m_add_col_iterator) == m_buffer.end()) {
          m_add_col_iterator = boost::none;
        }
      }
      // Once we are subtracting, it does not end
      if (m_subtract_col_iterator){
        m_indicator.subtract_subtotal(**m_subtract_col_iterator);
        ++(*m_subtract_col_iterator);
      }
      else if (m_coordinates.col > m_radius) { // == m_radius + 1, really
        m_subtract_col_iterator = m_buffer.begin();
        m_indicator.subtract_subtotal(**m_subtract_col_iterator);
        ++(*m_subtract_col_iterator);
      }
    }

    // The row index has incremented already, now catch up the indicator;
    void moved_down()
    {
      // Once we stopped adding, it won't start again
      if (m_add_row_iterator) {

        if (m_coordinates.row + m_radius == size1()) {
          m_add_row_iterator = boost::none;
        }
        else {
          auto i = m_buffer.begin();
          const auto end = m_buffer.end();

          // add each element of the new row to the buffer for its column
          for (; i != end; ++i, ++(*m_add_row_iterator)) {
            add_iterator_to_indicator(*m_add_row_iterator, *i);
          }
        }
      }
      // Once we are subtracting, it does not end
      if (m_subtract_row_iterator || initial_subtract_row_iterator()) {
        auto i = m_buffer.begin();
        const auto end = m_buffer.end();
        // subtract each element of the abandoned row from its col buffer
        for (; i != end; ++i, ++(*m_subtract_row_iterator)) {
          subtract_iterator_from_indicator(*m_subtract_row_iterator, *i);
        }
      }

      // Reset the indicator and col iterators
      m_indicator = m_initializer();
      m_subtract_col_iterator = boost::none;

      // Accumulate the first <radius> rows to get the value for the first cell
      const index_type endcol = std::min(size2(), m_radius + 1);
      m_add_col_iterator = m_buffer.begin();
      const auto end = (*m_add_col_iterator) + endcol;;

      for (; m_add_col_iterator != end; ++(*m_add_col_iterator)) {
        m_indicator.add_subtotal(**m_add_col_iterator);
      }
      if (size2() <= m_radius + 1) {
        assert((*m_add_col_iterator) == m_buffer.end());
        m_add_col_iterator = boost::none;
      }
    }

    bool initial_subtract_row_iterator()
    {
      if (m_coordinates.row > m_radius) {// really == m_radius + 1
        m_subtract_row_iterator = begin_input();
        return true;
      }
      else {
        return false;
      }
    }

    input_iterator begin_input()
    {
      return m_data->begin();
    }

    input_iterator end_input()
    {
      return m_data->end();
    }

    template<typename Iter, typename Accu>
    void add_iterator_to_indicator(Iter& iter, Accu& accu)
    {
      iter.add_to_indicator(accu);
    }

    template<typename Iter, typename Accu>
    void subtract_iterator_from_indicator(Iter& iter, Accu& accu)
    {
      iter.subtract_from_to_indicator(accu);
    }

    coordinate_type m_coordinates;
    IndicatorInputRaster* m_data;
    Indicator m_indicator;
    construction_functor<Indicator> m_initializer;
    index_type m_radius;

    typedef typename std::vector<Indicator>::const_iterator vector_iterator;
    boost::optional<vector_iterator> m_subtract_col_iterator;
    boost::optional<vector_iterator> m_add_col_iterator;
    boost::optional<input_iterator> m_subtract_row_iterator;
    boost::optional<input_iterator> m_add_row_iterator;

    // One indicator for each column, may be problematic for large indicators
    std::vector<Indicator> m_buffer;
  };

} // namespace moving_window 

#endif
