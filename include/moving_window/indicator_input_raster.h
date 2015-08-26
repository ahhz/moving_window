
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// Depending on the weighted/unweighted characteristic of the indicator
// , we need to iterate over one or two maps for moving window creation. 
// The indicator_input_raster class can either have a weight or not
// TODO: The name of this file is no longer appropriate
//

#ifndef INDICATOR_INPUT_RASTER_H_AHZ
#define INDICATOR_INPUT_RASTER_H_AHZ

#include <moving_window/raster_iterator.h>
#include <moving_window/raster_traits.h>

#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/tuple/tuple.hpp>

#include <type_traits> // conditional is_same

namespace moving_window {

  // forward declaration
  template<typename Raster,
    typename WeightRaster,
    typename Orientation,
    typename ElementType,
    typename AccessType>
  struct indicator_iterator;


  template<typename Raster, typename WeightRaster>
  struct indicator_input_raster
  {
    // value_type ??
    typedef typename raster_traits::value_type<Raster>::type value_type_1;
    typedef typename raster_traits::value_type<WeightRaster>::type value_type_2;
    typedef typename std::pair<value_type_1, value_type_2> value_type;

    typedef typename raster_traits::coordinate_type<Raster>::type coordinate_type;
    typedef typename raster_traits::index_type<Raster>::type index_type;
    typedef Raster raster_type;
    typedef WeightRaster weight_raster_type;


    int size1()const
    {
      return m_Raster->size1();
    }
    int size2()const
    {
      return m_Raster->size2();
    }

    typedef indicator_input_raster<Raster, WeightRaster> this_type;
    indicator_input_raster(Raster* r, WeightRaster* w = NULL)
      : m_Raster(r), m_WeightRaster(w)
    {
    }

    typedef typename indicator_iterator<
      Raster, WeightRaster
      , raster_iterator_tag::orientation::row_major
      , raster_iterator_tag::element::pixel
      , raster_iterator_tag::access::read_only> iterator;


    template<typename special_iterator = iterator>
    special_iterator begin()
    {
      special_iterator i(m_Raster, m_WeightRaster);
      i.find_begin();
      return i;
    }

    template<typename special_iterator = iterator>
    special_iterator end()
    {
      special_iterator i(m_Raster, m_WeightRaster);
      i.find_end();
      return i;
    }
    Raster* get_raster()
    {
      return m_Raster;
    }

    WeightRaster* get_weight()
    {
      return m_WeightRaster;
    }
    Raster* m_Raster;
    WeightRaster* m_WeightRaster;
  };

  template<typename Raster>
  struct indicator_input_raster<Raster, int>
  {
    // value_type ??
    typedef typename raster_traits::value_type<Raster>::type value_type;
    typedef typename raster_traits::coordinate_type<Raster>::type coordinate_type;
    typedef typename raster_traits::index_type<Raster>::type index_type;
    typedef Raster raster_type;
    typedef int weight_raster_type;

    typedef typename indicator_iterator<
      Raster, int
      , raster_iterator_tag::orientation::row_major
      , raster_iterator_tag::element::pixel
      , raster_iterator_tag::access::read_only> iterator;

    index_type size1()const
    {
      return m_Raster->size1();
    }
    index_type size2()const
    {
      return m_Raster->size2();
    }


    typedef indicator_input_raster<Raster, int> this_type;
    indicator_input_raster(Raster* r, int* ignore = NULL) :m_Raster(r)
    {
    }

    template<typename special_iterator = iterator>
    special_iterator begin() const
    {
      special_iterator i(m_Raster);
      i.find_begin();
      return i;
    }

    template<typename special_iterator = iterator>
    special_iterator end() const
    {
      special_iterator i(m_Raster);
      i.find_end();
      return i;
    }
    Raster* get_raster()
    {
      return m_Raster;
    }

    int* get_weight()
    {
      return nullptr;
    }

    Raster* m_Raster;
  };

  template<typename Raster, typename WeightRaster, typename Orientation,
    typename ElementType, typename AccessType>
  struct indicator_iterator_helper
  {
    typedef typename raster_traits::iterator<Orientation, ElementType
      , AccessType, Raster>::type sample_iterator;

    typedef typename raster_traits::iterator<Orientation, ElementType
      , AccessType, WeightRaster>::type weight_iterator;

    typedef boost::zip_iterator<boost::tuple<sample_iterator, weight_iterator> >
      zip_iterator;

    typedef typename std::conditional<std::is_same<WeightRaster, int>::value,
      sample_iterator, zip_iterator>::type input_iterator;
  };




  // An iterator that goes over the input raster(s) and can be added / subtracted
  // to and from indicators. 
  template<
    typename Raster,
    typename WeightRaster,
    typename Orientation,
    typename ElementType,
    typename AccessType>
  struct indicator_iterator :
    public boost::iterator_adaptor< indicator_iterator<Raster, WeightRaster,
    Orientation, ElementType, AccessType>,
    typename indicator_iterator_helper<Raster, WeightRaster, Orientation,
    ElementType, AccessType>::zip_iterator,
    boost::use_default, boost::use_default>

    // Inheritance to give it all of the characteristics of an iterator.
  {
  public:
    typedef typename Raster::coordinate_type coordinate_type;
    typedef typename coordinate_type::index_type index_type;

    typedef indicator_iterator_helper<Raster, WeightRaster, Orientation,
      ElementType, AccessType> helper;

    typedef typename helper::sample_iterator sample_iterator;
    typedef typename helper::weight_iterator weight_iterator;
    typedef typename helper::input_iterator input_iterator;

    indicator_iterator(Raster* raster, WeightRaster* weightraster)
      : iterator_adaptor(input_iterator(boost::make_tuple(sample_iterator(raster),
      weight_iterator(weightraster))))
    {
    }

    indicator_iterator(indicator_input_raster<Raster, WeightRaster>* input)
      : iterator_adaptor(input_iterator(boost::make_tuple(
      sample_iterator(input->m_Raster),
      weight_iterator(input->m_WeightRaster))))
    {}

    void find_begin()
    {
      get_sample_iterator().find_begin();
      get_weight_iterator().find_begin();
    }

    void find_end()
    {
      get_sample_iterator().find_end();
      get_weight_iterator().find_end();
    }

    void find(const coordinate_type& coordinates)
    {
      get_sample_iterator().find(coordinates);
      get_weight_iterator().find(coordinates);
    }

    template<typename Indicator>
    void add_to_indicator(Indicator& acc)
    {
      acc.add_sample(*get_const_sample_iterator(), *get_const_weight_iterator());
    }

    template<typename Indicator>
    void subtract_from_to_indicator(Indicator& acc)
    {
      acc.subtract_sample(*get_const_sample_iterator(),
        *get_const_weight_iterator());
    }

    const coordinate_type& get_coordinates() const
    {
      return get_const_sample_iterator().get_coordinates();
    }

  private:
    const sample_iterator& get_const_sample_iterator() const
    {
      return base_reference().get_iterator_tuple().get<0>();
    }

    const weight_iterator& get_const_weight_iterator() const
    {
      return base_reference().get_iterator_tuple().get<1>();
    }

    // zip_iterator only gives const references to individual iterators.
    // but find / find_begin / find_end need non const iterators
    sample_iterator& get_sample_iterator()
    {
      return const_cast<sample_iterator&>(get_const_sample_iterator());
    }
    weight_iterator& get_weight_iterator()
    {
      return const_cast<weight_iterator&>(get_const_weight_iterator());
    }
  };


  template<typename Raster, typename Orientation, typename ElementType,
    typename AccessType>
  class indicator_iterator<Raster, int, Orientation, ElementType, AccessType>
    : public boost::iterator_adaptor< indicator_iterator<Raster, int,
    Orientation, ElementType, AccessType>, //this_type 
    typename indicator_iterator_helper<Raster, int, Orientation, ElementType,
    AccessType>::sample_iterator, boost::use_default, boost::use_default>
  {
  public:

    typedef typename Raster::coordinate_type coordinate_type;
    typedef typename coordinate_type::index_type index_type;

    typedef indicator_iterator_helper<Raster, int, Orientation, ElementType,
      AccessType> helper;
    typedef typename helper::sample_iterator sample_iterator;

    indicator_iterator(Raster* r, int* = NULL)
      : iterator_adaptor(sample_iterator(r))
    {}

    indicator_iterator(indicator_input_raster<Raster, int>* input)
      : iterator_adaptor(sample_iterator(input->m_Raster))
    {}

    template<typename Indicator>
    void add_to_indicator(Indicator& acc)
    {
      acc.add_sample(*get_const_sample_iterator());
    }

    template<typename Indicator>
    void subtract_from_to_indicator(Indicator& acc)
    {
      acc.subtract_sample(*get_const_sample_iterator());
    }

    void find_begin()
    {
      get_sample_iterator().find_begin();
    }

    void find_end()
    {
      get_sample_iterator().find_end();
    }

    void find(const coordinate_type& coordinates)
    {
      get_sample_iterator().find(coordinates);
    }
    coordinate_type get_coordinates() const
      // Making a copy to stop compiler warning about returning a local reference. 
      // Is this really necessary??
    {
      return get_const_sample_iterator().get_coordinates();
    }
  private:
    sample_iterator& get_sample_iterator()
    {
      return base_reference();
    }
    const sample_iterator& get_const_sample_iterator() const
    {
      return base_reference();
    }
  };

  // by setting these raster traits, the raster views become available!
  namespace raster_traits
  {
    template <typename OrientationTag, typename ElementTag, typename AccessTag,
      typename Raster, typename WeightRaster>
    struct iterator< OrientationTag, ElementTag, AccessTag,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster, OrientationTag,
        ElementTag, AccessTag> type;
    };


    // These are necessary to have prevalence over edge iterators for single raster
    // TODO solve better:
    template <typename Raster, typename WeightRaster>
    struct iterator<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::v_edge,
      raster_iterator_tag::access::read_only,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster,
        raster_iterator_tag::orientation::row_major,
        raster_iterator_tag::element::v_edge,
        raster_iterator_tag::access::read_only
      > type;
    };

    template <typename Raster, typename WeightRaster>
    struct iterator<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::v_edge_first_only,
      raster_iterator_tag::access::read_only,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster,
        raster_iterator_tag::orientation::row_major,
        raster_iterator_tag::element::v_edge_first_only,
        raster_iterator_tag::access::read_only
      > type;
    };

    template <typename Raster, typename WeightRaster>
    struct iterator<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::v_edge_second_only,
      raster_iterator_tag::access::read_only,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster,
        raster_iterator_tag::orientation::row_major,
        raster_iterator_tag::element::v_edge_second_only,
        raster_iterator_tag::access::read_only
      > type;
    };

    template <typename Raster, typename WeightRaster>
    struct iterator<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::h_edge,
      raster_iterator_tag::access::read_only,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster,
        raster_iterator_tag::orientation::row_major,
        raster_iterator_tag::element::h_edge,
        raster_iterator_tag::access::read_only
      > type;
    };

    template <typename Raster, typename WeightRaster>
    struct iterator<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::h_edge_first_only,
      raster_iterator_tag::access::read_only,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster,
        raster_iterator_tag::orientation::row_major,
        raster_iterator_tag::element::h_edge_first_only,
        raster_iterator_tag::access::read_only
      > type;
    };

    template <typename Raster, typename WeightRaster>
    struct iterator<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::h_edge_second_only,
      raster_iterator_tag::access::read_only,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster,
        raster_iterator_tag::orientation::row_major,
        raster_iterator_tag::element::h_edge_second_only,
        raster_iterator_tag::access::read_only
      > type;
    };


    template <typename Raster, typename WeightRaster>
    struct iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::v_edge,
      raster_iterator_tag::access::read_only,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster,
        raster_iterator_tag::orientation::col_major,
        raster_iterator_tag::element::v_edge,
        raster_iterator_tag::access::read_only
      > type;
    };

    template <typename Raster, typename WeightRaster>
    struct iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::v_edge_first_only,
      raster_iterator_tag::access::read_only,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster,
        raster_iterator_tag::orientation::col_major,
        raster_iterator_tag::element::v_edge_first_only,
        raster_iterator_tag::access::read_only
      > type;
    };

    template <typename Raster, typename WeightRaster>
    struct iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::v_edge_second_only,
      raster_iterator_tag::access::read_only,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster,
        raster_iterator_tag::orientation::col_major,
        raster_iterator_tag::element::v_edge_second_only,
        raster_iterator_tag::access::read_only
      > type;
    };

    template <typename Raster, typename WeightRaster>
    struct iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::h_edge,
      raster_iterator_tag::access::read_only,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster,
        raster_iterator_tag::orientation::col_major,
        raster_iterator_tag::element::h_edge,
        raster_iterator_tag::access::read_only
      > type;
    };

    template <typename Raster, typename WeightRaster>
    struct iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::h_edge_first_only,
      raster_iterator_tag::access::read_only,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster,
        raster_iterator_tag::orientation::col_major,
        raster_iterator_tag::element::h_edge_first_only,
        raster_iterator_tag::access::read_only
      > type;
    };

    template <typename Raster, typename WeightRaster>
    struct iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::h_edge_second_only,
      raster_iterator_tag::access::read_only,
      indicator_input_raster<Raster, WeightRaster> >
    {
      typedef indicator_iterator<Raster, WeightRaster,
        raster_iterator_tag::orientation::col_major,
        raster_iterator_tag::element::h_edge_second_only,
        raster_iterator_tag::access::read_only
      > type;
    };

  };

} // namespace moving_window 
#endif
