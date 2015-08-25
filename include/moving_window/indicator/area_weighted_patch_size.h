
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Not for public distribution
//=======================================================================

#ifndef INDICATOR_AREA_WEIGHTED_PATCH_SIZE_H_AHZ
#define INDICATOR_AREA_WEIGHTED_PATCH_SIZE_H_AHZ

#include <moving_window/default_construction_functor.h>
#include <moving_window/traits.h>

#include <boost/optional.hpp>
#include <boost/property_map/property_map.hpp>

#include <type_traits>
#include <utility>

namespace moving_window {

  struct area_weighted_patch_size_tag
  {};

  template<typename PatchSizeMap> // property map that maps index to patch size
  struct area_weighted_patch_size
  {
    area_weighted_patch_size(PatchSizeMap psm)
      : m_sum(0), m_weight(0), m_patch_size_map(psm)
    {}

    typedef typename boost::property_traits<PatchSizeMap>::key_type sample_type;
    typedef area_weighted_patch_size_tag my_tag;
    typedef area_weighted_patch_size<PatchSizeMap> this_type;

    struct initializer
    {
      initializer(PatchSizeMap pm, int number_of_patches) : m_pm(pm)
      {}

      area_weighted_patch_size<PatchSizeMap> operator()()
      {
        return area_weighted_patch_size(m_pm);
      }
      PatchSizeMap m_pm;
    };

    void add_sample(const sample_type& i)
    {
      // the sample gives the index to the patch property map
      // read the size from the property map
      const double s = get(m_patch_size_map, i);
      m_weight++;
      m_sum += s;
    }

    template<typename Weight>
    void add_sample(const sample_type& i, const Weight& w)
    {
      // the sample gives the index to the patch property map
      // read the size from the property map
      const double s = get(m_patch_size_map, i);
      m_weight += w;
      m_sum += w * s;
    }

    void subtract_sample(const sample_type& i)
    {
      // the sample gives the index to the patch property map
      // read the size from the property map
      const double s = get(m_patch_size_map, i);
      m_weight--;
      m_sum -= s;
    }

    template<typename Weight>
    void subtract_sample(const sample_type& i, const Weight& w)
    {
      // the sample gives the index to the patch property map
      // read the size from the property map
      const double s = get(m_patch_size_map, i);
      m_weight -= w;
      m_sum -= w * s;
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
      return boost::make_optional(m_weight > 0, m_sum / m_weight);
    }

    PatchSizeMap m_patch_size_map;
    double m_sum;
    double m_weight;
  };

  template<>
  struct indicator_traits<area_weighted_patch_size_tag>
  {
    typedef std::true_type  needs_patch_size;
    typedef std::false_type needs_patch_perimeter;
    typedef std::false_type needs_patch_category;

    typedef patch_element_tag element_type_tag;

    template<typename PatchSizeMap>
    struct indicator
    {
      typedef area_weighted_patch_size<PatchSizeMap> indicator_type;
      typedef default_construction_functor<indicator_type> initializer;
      initializer make_initializer()
      {
        return initializer();
      }
    };
  };
}
#endif // INDICATOR_AREA_WEIGHTED_PATCH_SIZE_H_AHZ
