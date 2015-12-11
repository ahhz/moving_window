//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// The moving_window_view<IndicatorTag, Window, Raster, WeightRaster> provides
// a range to iterate over the raster and at each instance return the indicator
// for the window centred on the cell.

#ifndef BLINK_MOVING_WINDOW_MOVING_WINDOW_VIEW_H_AHZ
#define BLINK_MOVING_WINDOW_MOVING_WINDOW_VIEW_H_AHZ

#include <blink/raster/gdal_raster.h>
#include <blink/raster/gdal_raster_view.h>
#include <blink/raster/edge_view.h>
#include <blink/raster/raster_traits.h>
#include <blink/raster/utility.h>

#include <blink/moving_window/traits.h>
#include <blink/moving_window/moving_window_patch_view_helper.h>
#include <blink/moving_window/window.h>
#include <blink/moving_window/non_square_window_edge_iterator.h>
#include <blink/moving_window/non_square_window_iterator.h>
#include <blink/moving_window/square_window_edge_iterator.h>
#include <blink/moving_window/square_window_iterator.h>

#include <type_traits>

namespace blink {
  namespace moving_window {

    template< typename IndicatorTag, typename Window, typename Raster>
    struct moving_window_pixel_view
    {
      using element_type = typename indicator_traits<IndicatorTag>::element_type_tag;
      using input_type = Raster;
      using raster_type = typename input_type::raster_type;
      using raster_value_type = blink::raster::raster_traits::value_type < raster_type > ;

      using resolver = typename indicator_traits<IndicatorTag>::indicator < raster_value_type > ;
      using indicator = typename resolver::indicator_type;
      using initializer = typename resolver::initializer;

      using square_window_type = typename square_window_iterator < indicator, input_type > ;
      using circle_window_type = typename non_square_window_iterator < indicator, input_type
        , Window > ;

      using window_family_type = typename window_family<Window>::type;

      using iterator = typename std::conditional
        < std::is_same<window_family_type, circle_tag>::value
        , circle_window_type, square_window_type > ::type;

      moving_window_pixel_view(const Window& window, const Raster& raster)
        : m_window(window), m_raster(raster)
      {
      }

      iterator begin()
      {
        iterator i(m_window, &m_raster, initializer());
        i.find_begin();
        return i;
      }

      iterator end()
      {
        iterator i(m_window, &m_raster, initializer());
        i.find_end();
        return i;
      }

      Window m_window; // copy by value
      input_type m_raster; // copy by value (raster is two pointers)
    };

    template< typename IndicatorTag, typename Window, typename Raster>
    struct moving_window_edge_view
    {
      using element_type = typename indicator_traits<IndicatorTag>::element_type_tag;
      using input_type = Raster;
      using raster_type = typename input_type::raster_type;
      using raster_value_type = blink::raster::raster_traits::value_type < raster_type > ;

      using resolver = typename indicator_traits<IndicatorTag>::indicator < raster_value_type > ;
      using indicator = typename resolver::indicator_type;
      using initializer = typename resolver::initializer;

      using square_window_type = square_window_edge_iterator < indicator, input_type > ;
      using circle_window_type = non_square_window_edge_iterator < indicator, input_type
        , Window > ;

      using window_family_type = typename  window_family<Window>::type;

      using iterator = typename std::conditional
        < std::is_same<window_family_type, circle_tag>::value
        , circle_window_type, square_window_type > ::type;

      moving_window_edge_view(const Window& window, const Raster& raster)
        : m_window(window), m_raster(raster)
      {
      }

      iterator begin()
      {
        iterator i(m_window, &m_raster, initializer());
        i.find_begin();
        return i;
      }

      iterator end()
      {
        iterator i(m_window, &m_raster, initializer());
        i.find_end();
        return i;

      }

      Window m_window; // copy by value
      input_type m_raster; // copy by value (raster is two pointers)
    };

    template< typename IndicatorTag, typename Window, typename Raster>
    struct moving_window_patch_view
    {
      using index_map_type = blink::raster::gdal_raster < int > ;
      using element_type = typename indicator_traits<IndicatorTag>::element_type_tag;
      using input_type = typename indicator_input_raster < index_map_type
        , typename Raster::weight_raster_type > ;
      using raster_type = typename input_type::raster_type;
      using raster_value_type = blink::raster::raster_traits::value_type < raster_type > ;

      using helper = typename moving_window_patch_view_helper < IndicatorTag > ;

      using indicator = typename helper::indicator_type;
      using initializer = typename helper::initializer;

      using square_window_type = typename square_window_iterator < indicator, input_type > ;
      using circle_window_type = typename non_square_window_iterator < indicator, input_type
        , Window > ;

      using window_family_type = typename window_family<Window>::type;

      using iterator = typename std::conditional
        < std::is_same<window_family_type, circle_tag>::value
        , circle_window_type, square_window_type > ::type;

      using patch_category_map_type = typename helper::patch_category_map_type;
      using patch_size_map_type = typename helper::patch_size_map_type;
      using patch_perimeter_map_type = typename helper::patch_perimeter_map_type;

      moving_window_patch_view(const Window& window, Raster& raster
        , bool use_queen_contiguity = true)
        : m_window(window)
        , m_raster(&m_index_raster, raster.get_weight())
      {
        m_index_raster = blink::raster::create_temp_gdal_raster<int>(
          static_cast<int>(raster.size1()),
          static_cast<int>(raster.size2()));

        int number_of_patches = patch_detect(
          *(raster.get_raster()), m_index_raster,
          m_category_map, m_size_map, m_perimeter_map,
          use_queen_contiguity);

        // TODO: initializer does not have a default constructor
        m_initializer = new initializer(helper::make_initializer(m_category_map, m_size_map
          , m_perimeter_map, number_of_patches));
      }

      moving_window_patch_view(const moving_window_patch_view&) = delete;
      ~moving_window_patch_view()
      {
        delete m_initializer;
      }
      iterator begin()
      {
        iterator i(m_window, &m_raster, *m_initializer);
        i.find_begin();
        return i;
      }

      iterator end()
      {
        iterator i(m_window, &m_raster, *m_initializer);
        i.find_end();
        return i;
      }

      Window m_window; // copy by value
      index_map_type m_index_raster;
      initializer* m_initializer;
      patch_category_map_type m_category_map;
      patch_size_map_type m_size_map;
      patch_perimeter_map_type m_perimeter_map;
      input_type m_raster; // copy by value (raster is two pointers)

    };

    template<typename IndicatorTag, typename Window, typename Raster
      , typename WeightRaster>
    struct moving_window_view_selector
    {
      using element_type = typename indicator_traits<IndicatorTag>::element_type_tag;
      using input_type = indicator_input_raster < Raster, WeightRaster > ;

      using patch_based = moving_window_patch_view < IndicatorTag, Window
        , input_type > ;

      using pixel_based = moving_window_pixel_view < IndicatorTag, Window
        , input_type > ;

      using edge_based = moving_window_edge_view < IndicatorTag
        , Window, input_type > ;

      struct unknown_element_tag{};

      template<typename ElementType>
      using elem_is = std::is_same < ElementType, element_type > ;

      using type =
        typename std::conditional < elem_is<patch_element_tag>::value, patch_based,
        typename std::conditional < elem_is<edge_element_tag >::value, edge_based,
        typename std::conditional<elem_is<pixel_element_tag>::value, pixel_based
        , unknown_element_tag>::type > ::type > ::type;

    };

    template<typename IndicatorTag, typename Window, typename Raster
      , typename WeightRaster>
      using moving_window_view = typename moving_window_view_selector
      < IndicatorTag, Window, Raster, WeightRaster>::type;

    template<typename IndicatorTag, typename Window, typename Raster
      , typename WeightRaster = int>
      moving_window_view<IndicatorTag, Window, Raster, WeightRaster>
      make_moving_window_view(const IndicatorTag&, Window& window
      , Raster* raster, WeightRaster* weightraster = nullptr)
    {

      indicator_input_raster<Raster, WeightRaster> input(raster, weightraster);
      return moving_window_view<IndicatorTag, Window, Raster, WeightRaster>(window
        , input);
    }

  } // namespace moving_window
}
#endif