
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef BLINK_MOVING_WINDOW_INDICATOR_PATCH_WEIGHTED_SHAPE_H_AHZ
#define BLINK_MOVING_WINDOW_INDICATOR_PATCH_WEIGHTED_SHAPE_H_AHZ

#include <blink/moving_window/default_construction_functor.h>
#include <blink/moving_window/traits.h>

#include <boost/optional.hpp>
#include <boost/property_map/property_map.hpp>

#include <type_traits>
#include <utility>

namespace blink {
  namespace moving_window {

    struct patch_weighted_shape_index_tag
    {};

    template<typename PatchSizeMap, typename PatchPerimeterMap> // property map that maps index to patch size
    struct patch_weighted_shape_index
    {
      typedef boost::vector_property_map<double> patch_shape_map_type;

      patch_weighted_shape_index(
        PatchSizeMap sizemap,
        patch_shape_map_type shapemap)
        : m_sum(0), m_weight(0), m_patch_size_map(sizemap), m_shape_map(shapemap)
      {}

      typedef typename boost::property_traits<PatchSizeMap>::key_type sample_type;
      typedef patch_weighted_shape_index_tag my_tag;
      typedef patch_weighted_shape_index<PatchSizeMap, PatchPerimeterMap> this_type;

      struct initializer
      {
        initializer(PatchSizeMap psm, PatchPerimeterMap ppm, int number_of_patches)
          : m_psm(psm), m_ppm(ppm)
        {
          for (int i = 0; i < number_of_patches; ++i)
          {
            const int s = static_cast<int>(get(m_psm, i) + 0.5); // check rounding
            const int p = static_cast<int>(get(m_ppm, i) + 0.5);
            int n = static_cast<int>(sqrt(s));
            int pmin;
            if (s == n * n){
              pmin = 4 * n;
            }
            else if (s < n * (n + 1)) {
              pmin = 4 * n + 2;
            }
            else {
              pmin = 4 * n + 4;
            }
            put(m_shape_map, i, p / static_cast<double>(pmin));
          }
        }

        patch_weighted_shape_index<PatchSizeMap, PatchPerimeterMap> operator()()
        {
          return patch_weighted_shape_index(m_psm, m_shape_map);
        }

        PatchSizeMap m_psm;
        PatchPerimeterMap m_ppm;
        patch_shape_map_type m_shape_map;
        int m_category;
      };

      void add_sample(const sample_type& i)
      {
        const double area = get(m_patch_size_map, i);
        const double shape = get(m_shape_map, i);
        double w = 1.0 / area;
        m_weight += w;
        m_sum += w * shape;
      }

      template<typename Weight>
      void add_sample(const sample_type& i, const Weight& w)
      {
        const double area = get(m_patch_size_map, i);
        const double shape = get(m_shape_map, i);
        double w2 = w  * 1.0 / area;
        m_weight += w2;
        m_sum += w2 * shape;
      }

      void subtract_sample(const sample_type& i)
      {
        const double area = get(m_patch_size_map, i);
        const double shape = get(m_shape_map, i);
        double w = 1.0 / area;
        m_weight -= w;
        m_sum -= w * shape;
      }

      template<typename Weight>
      void subtract_sample(const sample_type& i, const Weight& w)
      {
        const double area = get(m_patch_size_map, i);
        const double shape = get(m_shape_map, i);
        double w2 = w  * 1.0 / area;
        m_weight -= w2;
        m_sum -= w2 * shape;
      }

      void add_subtotal(const this_type& subtotal)
      {
        m_sum += subtotal.m_sum;
        m_weight += subtotal.m_weight;
      }

      template<typename Weight>
      void add_subtotal(const this_type& subtotal, const Weight& w)
      {
        m_sum += w * subtotal.m_sum;
        m_weight += w * subtotal.m_weight;
      }

      void subtract_subtotal(const this_type& subtotal)
      {
        m_sum -= subtotal.m_sum;
        m_weight -= subtotal.m_weight;
      }

      template<typename Weight>
      void subtract_subtotal(const this_type& subtotal, const Weight& w)
      {
        m_sum -= w * subtotal.m_sum;
        m_weight -= w * subtotal.m_weight;
      }

      boost::optional<double> extract() const
      {
        return boost::make_optional(m_weight > 1e-8, m_sum / m_weight);
      }

      PatchSizeMap m_patch_size_map;
      patch_shape_map_type m_shape_map;
      double m_sum;
      double m_weight;
    };

    template<>
    struct indicator_traits < patch_weighted_shape_index_tag >
    {
      typedef std::true_type needs_patch_size;
      typedef std::true_type needs_patch_perimeter;
      typedef std::false_type needs_patch_category;

      typedef patch_element_tag element_type_tag;

      template<typename PatchSizeMap, typename PatchPerimeterMap>
      struct indicator
      {
        typedef patch_weighted_shape_index<PatchSizeMap, PatchPerimeterMap> indicator_type;

        //typedef default_construction_functor<indicator_type> initializer;
        //initializer make_initializer()
        //{
        //	return initializer();
        //}
      };
    };
  } //namespace moving_window 
}
#endif //INDICATOR_PATCH_WEIGHTED_SHAPE_H_AHZ