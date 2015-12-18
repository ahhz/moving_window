//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================       
//
// The square_indow_iterator provides a generalized interface for patch
// or pixel-based indicators
//
// TODO: Generalize this to a RectangleWindowIterator?

#ifndef BLINK_MOVING_WINDOW_SQUARE_WINDOW_ITERATOR_H_AHZ
#define BLINK_MOVING_WINDOW_SQUARE_WINDOW_ITERATOR_H_AHZ

#include <blink/moving_window/default_construction_functor.h>

#include <blink/iterator/zip_range.h>
#include <blink/iterator/zip_along_range.h>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/none_t.hpp>
#include <boost/optional.hpp>

#include <vector>

namespace blink {
  namespace moving_window {
   
    template<typename Indicator, typename IndicatorInputRaster>
    class square_window_iterator
      : public boost::iterator_facade
          < square_window_iterator<Indicator, IndicatorInputRaster>
          , const Indicator, boost::forward_traversal_tag >
    {
      using input_iterator = typename IndicatorInputRaster::iterator;
      using input_value = typename input_iterator::value_type;
      using default_initializer = default_construction_functor<Indicator>;
      using vector_iterator = typename std::vector<Indicator>::const_iterator;
      using is_weighted = typename IndicatorInputRaster::is_weighted;
  
    public:
      using coordinate_type = typename IndicatorInputRaster::coordinate_type;
      using index_type = typename coordinate_type::index_type;

      template<typename Window, typename Initializer = default_initializer>
      square_window_iterator(Window& window, IndicatorInputRaster* data, 
        Initializer initializer = Initializer() ) : m_data(data), 
        m_radius(window.get_radius()), m_initializer(initializer)
        , m_indicator(initializer())
      {}

      // disallow copying: this violates Iterator concept though :( 
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

      // disallow copyassigning
      square_window_iterator& operator=(const square_window_iterator&) = delete;

      void find_end()
      {
        m_coordinates = coordinate_type(m_data->size1(), 0);
      }

      void find_begin()
      {
        m_coordinates = coordinate_type(0, 0);

        m_buffer.assign(m_data->size2(), m_initializer());
        m_subtract_row_iterator = boost::none;
        m_add_row_iterator = m_data->begin();
        
        for (index_type row = 0; row <= m_radius; ++row) {
          // the case where radius >= size1() is covered by add_row_to_buffer()
          add_row_to_buffer();
        }

        prepare_first_pixel_of_row();
      }

    private:
      friend class boost::iterator_core_access;

      void increment()
      {
        if (++m_coordinates.col != m_data->size2()) {
          return moved_right();
        }

        m_coordinates.col = 0; // end of row, start new row

        if (++m_coordinates.row != m_data->size1()) {
          return moved_down();
        }
        //end of raster
      }

      bool equal(square_window_iterator const& other) const
      {
        return m_coordinates == other.m_coordinates;
      }

      const Indicator& dereference() const
      {
        return get_indicator();
      }

    private:
      const Indicator& get_indicator() const
      {
        return m_indicator;
      }
        
      const coordinate_type& get_coordinates() const
      {
        return m_coordinates;
      }
      void moved_right()
      {
        add_col_to_indicator();
        subtract_col_from_indicator();
      }

      void moved_down()
      {
        add_row_to_buffer();
        subtract_row_from_buffer();
        prepare_first_pixel_of_row();
      }
  
      void add_col_to_indicator()
      {
        if (m_add_col_iterator) {
          m_indicator.add_subtotal(**m_add_col_iterator);
          if (++(*m_add_col_iterator) == m_buffer.end()) {
            m_add_col_iterator = boost::none;
          }
        }
      }
      
      void subtract_col_from_indicator()
      {
        if (m_subtract_col_iterator.is_initialized() 
          || initial_subtract_col_iterator()) {

          m_indicator.subtract_subtotal(**m_subtract_col_iterator);
          ++(*m_subtract_col_iterator);
        }
      }
   
      void subtract_row_from_buffer()
      {
        if (m_subtract_row_iterator.is_initialized() 
          || initial_subtract_row_iterator()) {
          auto zip = iterator::make_zip_range(
            std::ref(m_buffer),
            iterator::make_zip_along_range(*m_subtract_row_iterator));

          for (auto&& z : zip) {
            // indirect because of proxy
            input_value v = std::get<1>(z);
            subtract_sample_from_indicator(is_weighted{}, std::get<0>(z), v);
          }
        }
      }

      void add_row_to_buffer()
      {
        if (m_add_row_iterator) {
          auto zip = iterator::make_zip_range(std::ref(m_buffer),
            iterator::make_zip_along_range(*m_add_row_iterator));
         
          for (auto&& z : zip) {
            // indirect because of proxy
            input_value v = std::get<1>(z);
            add_sample_to_indicator(is_weighted{}, std::get<0>(z), v);
          }
          
          if (*m_add_row_iterator == m_data->end())  {
            m_add_row_iterator = boost::none;
          }
        }
      }

      void prepare_first_pixel_of_row()
      {
        m_indicator = m_initializer();
        m_subtract_col_iterator = boost::none;
        m_add_col_iterator = m_buffer.begin();
        
        for (int col = 0; col <= m_radius; ++col) {
          // the case where m_radius >= size2() is covered 
          add_col_to_indicator();
        }
      }

      bool initial_subtract_row_iterator()
      {
        if (m_coordinates.row == m_radius + 1) {
          m_subtract_row_iterator = m_data->begin();
          return true;
        }
        return false;
      }

      bool initial_subtract_col_iterator()
      {
        if (m_coordinates.col == m_radius + 1) {
          m_subtract_col_iterator = m_buffer.begin();
          return true;
        }
        return false;
      }

      coordinate_type m_coordinates;
      IndicatorInputRaster* m_data;
      Indicator m_indicator;
      construction_functor<Indicator> m_initializer;
      index_type m_radius;

      boost::optional<vector_iterator> m_subtract_col_iterator;
      boost::optional<vector_iterator> m_add_col_iterator;
      boost::optional<input_iterator> m_subtract_row_iterator;
      boost::optional<input_iterator> m_add_row_iterator;

      // One indicator for each column, may be problematic for large indicators
      std::vector<Indicator> m_buffer;
    };
  }  
}

#endif
