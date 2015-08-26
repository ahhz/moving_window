//
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// The moving_window_patch_view_helper provides the typedefs particular for 
// the moving_window_view for patches. Moreover it provide a general
// initialization function for patch based indicators. 
//
// TODO: This class is overlycomplicated and needs a good rethink and cleanup.

#ifndef MOVING_WINDOW_PATCH_VIEW_HELPER_H_AHZ
#define MOVING_WINDOW_PATCH_VIEW_HELPER_H_AHZ

#include <moving_window/gdal_raster.h>
//#include <moving_window/window.h>
#include <moving_window/non_square_window_iterator.h>
#include <moving_window/square_window_iterator.h>
#include <moving_window/traits.h>
#include <moving_window/patch_detect.h>

#include <boost/graph/property_maps/null_property_map.hpp>
#include <boost/property_map/property_map.hpp>

#include <type_traits>

namespace moving_window {

  template<typename Tag>
  struct moving_window_patch_view_helper
  {
    template <typename Condition, typename T>
    struct optional_vector_property_map
    {
      typedef typename std::conditional<
        Condition::value, typename boost::vector_property_map<T>
        , typename boost::null_property_map<std::size_t, T> >::type type;
    };
    typedef typename indicator_traits<Tag> traits;

    typedef typename traits::needs_patch_category needs_category;
    typedef typename traits::needs_patch_size needs_size;
    typedef typename traits::needs_patch_perimeter needs_perimeter;

    typedef typename optional_vector_property_map<needs_category, int>::type patch_category_map_type;
    typedef typename optional_vector_property_map<needs_size, double>::type patch_size_map_type;
    typedef typename optional_vector_property_map<needs_perimeter, double>::type patch_perimeter_map_type;


    template<typename...OtherArgs>
    struct indicator_type_helper
    {
    private:
      template < typename...Args> struct finish
      {
        typedef typename traits::indicator<Args...>::indicator_type type;
      };

      template<typename...Args> struct prepend_category
      {
        typedef typename std::conditional
          < needs_category::value
          , typename finish<patch_category_map_type, Args...>
          , typename finish<Args...>
          >::type::type type;
      };

      template<typename...Args> struct prepend_size
      {
        typedef typename std::conditional
          < needs_size::value
          , typename prepend_category<patch_size_map_type, Args...>
          , typename prepend_category<Args...>
          >::type::type type;
      };

      template<typename...Args> struct prepend_perimeter
      {
        typedef typename std::conditional
          < needs_perimeter::value
          , typename prepend_size<patch_perimeter_map_type, Args...>
          , typename prepend_size<Args...>
          >::type::type type;
      };
    public:
      typedef typename prepend_perimeter<OtherArgs...>::type type;
    };

    typedef typename indicator_type_helper<>::type indicator_type;
    typedef typename indicator_type::initializer initializer;

    template<typename ...Other>
    static initializer initializer_add_cat(const std::true_type&,
      patch_category_map_type cm, Other... other)
    {
      return initializer(cm, other...);
    }

    template<typename ...Other>
    static initializer initializer_add_cat(const std::false_type&,
      patch_category_map_type cm, Other... other)
    {
      return initializer(other...);
    }

    template<typename ...Other>
    static initializer initializer_add_size(const std::true_type&,
      patch_category_map_type cm, patch_size_map_type sm, Other... other)
    {
      return initializer_add_cat(needs_category(), cm, sm, other...);
    }

    template<typename ...Other>
    static initializer initializer_add_size(const std::false_type&,
      patch_category_map_type cm, patch_size_map_type sm, Other... other)
    {
      return initializer_add_cat(needs_category(), cm, other...);
    }

    template<typename ...Other>
    static initializer initializer_add_perimeter(const std::true_type&,
      patch_category_map_type cm, patch_size_map_type sm, patch_perimeter_map_type pm,
      Other... other)
    {
      return initializer_add_size(needs_size(), cm, sm, pm, other...);
    }

    template<typename ...Other>
    static initializer initializer_add_perimeter(const std::false_type&,
      patch_category_map_type cm, patch_size_map_type sm, patch_perimeter_map_type pm,
      Other... other)
    {
      return initializer_add_size(needs_size(), cm, sm, other...);
    }

    template<typename ...Other>
    static initializer make_initializer(
      patch_category_map_type cm, patch_size_map_type sm, patch_perimeter_map_type pm,
      Other...other)
    {
      return initializer_add_perimeter(needs_perimeter(), cm, sm, pm, other...);
    }

  };

} // namespace moving_window
#endif