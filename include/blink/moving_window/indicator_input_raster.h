
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// Depending on the weighted/unweighted characteristic of the indicator, we 
// need to iterate over one or two maps for moving window creation. The 
// indicator_input_raster class can either have a weight or not.
//

#ifndef BLINK_MOVING_WINDOW_INDICATOR_INPUT_RASTER_H_AHZ
#define BLINK_MOVING_WINDOW_INDICATOR_INPUT_RASTER_H_AHZ

#include <blink/raster/raster_iterator.h>
#include <blink/raster/raster_traits.h>
#include <blink/raster/raster_view.h>

#include <blink/iterator/zip_range.h>

#include <boost/iterator/iterator_adaptor.hpp>

#include <type_traits> // conditional is_same

namespace blink {
  namespace moving_window {

    template<class T, class Indicator>
    void add_sample_to_indicator(std::true_type, Indicator& indicator, T& value)
    {
      indicator.add_sample(std::get<0>(value), std::get<1>(value));
    }

    template<class T, class Indicator>
    void add_sample_to_indicator(std::false_type, Indicator& indicator, T& value)
    {
      indicator.add_sample(value);
    }

    template<class T, class Indicator>
    void subtract_sample_from_indicator(std::true_type, Indicator& indicator, T& value)
    {
      indicator.subtract_sample(std::get<0>(value), std::get<1>(value));
    }

    template<class T, class Indicator>
    void subtract_sample_from_indicator(std::false_type, Indicator& indicator, T& value)
    {
      indicator.subtract_sample(value);
    }

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
      using is_weighted = std::true_type;
      using value_type_1 = blink::raster::raster_traits::value_type < Raster > ;
      using value_type_2 = blink::raster::raster_traits::value_type < WeightRaster > ;

      using value_type = std::tuple < value_type_1, value_type_2 > ;
      using coordinate_type = blink::raster::raster_traits::coordinate_type < Raster > ;
      using index_type = blink::raster::raster_traits::index_type < Raster > ;
      using raster_type = Raster;
      using weight_raster_type = WeightRaster;

      index_type size1()const
      {
        return blink::raster::raster_operations::size1(*m_Raster);
      }
      index_type size2()const
      {
        return blink::raster::raster_operations::size2(*m_Raster);
      }
      using this_type = indicator_input_raster < Raster, WeightRaster > ;

      indicator_input_raster(Raster* r, WeightRaster* w = NULL)
        : m_Raster(r), m_WeightRaster(w)
      {
      }

      using iterator = indicator_iterator <
        Raster, WeightRaster
        , blink::raster::orientation::row_major
        , blink::raster::element::pixel
        , blink::raster::access::read_only > ;


      iterator begin()
      {
        iterator i(m_Raster, m_WeightRaster);
        i.find_begin();
        return i;
      }

      iterator end()
      {
        iterator i(m_Raster, m_WeightRaster);
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
    struct indicator_input_raster < Raster, int >
    {
      using is_weighted = std::false_type;

      using this_type = indicator_input_raster < Raster, int > ;
      using value_type = blink::raster::raster_traits::value_type < Raster > ;
      using coordinate_type = blink::raster::raster_traits::coordinate_type < Raster > ;
      using index_type = blink::raster::raster_traits::index_type < Raster > ;
      using raster_type = Raster;
      using weight_raster_type = int;

      using iterator = indicator_iterator <
        Raster, int
        , blink::raster::orientation::row_major
        , blink::raster::element::pixel
        , blink::raster::access::read_only > ;

      index_type size1()const
      {
        return blink::raster::raster_operations::size1(*m_Raster);
      }
      index_type size2()const
      {
        return blink::raster::raster_operations::size2(*m_Raster);
      }

      indicator_input_raster(Raster* r, int* ignore = NULL) :m_Raster(r)
      {
      }

      iterator begin() const
      {
        iterator i(m_Raster);
        i.find_begin();
        return i;
      }

      iterator end() const
      {
        iterator i(m_Raster);
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
      using sample_view = blink::raster::raster_view < Orientation, ElementType
        , AccessType, Raster > ;
      using weight_view = blink::raster::raster_view < Orientation, ElementType
        , AccessType, WeightRaster > ;

      using sample_iterator = typename sample_view::iterator;
      using weight_iterator = typename weight_view::iterator;

      using zip_view = blink::iterator::zip_range < sample_view, weight_view > ;
      using zip_iterator = typename zip_view::iterator;

      using input_view = typename std::conditional < std::is_same<WeightRaster, int>::value,
        sample_view, zip_view > ::type;

      using input_iterator = typename input_view::iterator;

    };

    template<typename Raster, typename Orientation,
      typename ElementType, typename AccessType>
    struct indicator_iterator_helper < Raster, int, Orientation, ElementType, AccessType >
    {
      using sample_view = blink::raster::raster_view < Orientation, ElementType
        , AccessType, Raster > ;
      using sample_iterator = typename sample_view::iterator;
      using input_view = sample_view;
      using input_iterator = typename input_view::iterator;
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
      public boost::iterator_adaptor < indicator_iterator<Raster, WeightRaster,
      Orientation, ElementType, AccessType>,
      typename indicator_iterator_helper<Raster, WeightRaster, Orientation,
      ElementType, AccessType>::zip_iterator,
      boost::use_default, boost::use_default >

      // Inheritance to give it all of the characteristics of an iterator.
    {
    public:
      typedef typename Raster::coordinate_type coordinate_type;
      typedef typename coordinate_type::index_type index_type;

      typedef indicator_iterator_helper < Raster, WeightRaster, Orientation,
        ElementType, AccessType > helper;

      typedef typename helper::sample_iterator sample_iterator;
      typedef typename helper::weight_iterator weight_iterator;
      typedef typename helper::input_iterator input_iterator;
      using sample_value_type = typename sample_iterator::value_type;
      using weight_value_type = typename weight_iterator::value_type;


      indicator_iterator(Raster* raster, WeightRaster* weightraster)
        : iterator_adaptor(input_iterator(sample_iterator(raster),
        weight_iterator(weightraster)))
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
        sample_value_type v = *get_const_sample_iterator();
        weight_value_type w = *get_const_weight_iterator();
        acc.add_sample(v, w);
      }

      template<typename Indicator>
      void subtract_from_to_indicator(Indicator& acc)
      {
        sample_value_type v = *get_const_sample_iterator();
        weight_value_type w = *get_const_weight_iterator();
        acc.subtract_sample(v, w);
      }

      const coordinate_type& get_coordinates() const
      {
        return get_const_sample_iterator().get_coordinates();
      }

    private:
      const sample_iterator& get_const_sample_iterator() const
      {
        return base_reference().get<0>();
      }

      const weight_iterator& get_const_weight_iterator() const
      {
        return base_reference().get<1>();
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
      : public boost::iterator_adaptor < indicator_iterator<Raster, int,
      Orientation, ElementType, AccessType>, //this_type 
      typename indicator_iterator_helper<Raster, int, Orientation, ElementType,
      AccessType>::sample_iterator, boost::use_default, boost::use_default >
    {
    public:

      typedef typename Raster::coordinate_type coordinate_type;
      typedef typename coordinate_type::index_type index_type;

      typedef indicator_iterator_helper<Raster, int, Orientation, ElementType,
        AccessType> helper;
      typedef typename helper::sample_iterator sample_iterator;
      using sample_value_type = typename sample_iterator::value_type;

      indicator_iterator(Raster* r, int* = NULL)
        : iterator_adaptor(sample_iterator(r))
      {}

      indicator_iterator(indicator_input_raster<Raster, int>* input)
        : iterator_adaptor(sample_iterator(input->m_Raster))
      {}

      template<typename Indicator>
      void add_to_indicator(Indicator& acc)
      {
        sample_value_type v = *get_const_sample_iterator();
        acc.add_sample(v);
      }

      template<typename Indicator>
      void subtract_from_to_indicator(Indicator& acc)
      {
        // indirect because iterator dereference may be a proxy
        sample_value_type v = *get_const_sample_iterator();
        acc.subtract_sample(v);
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

    template < class Orientation, class Element, class Access, class IndicatorInputRaster >
    class indicator_input_view
    {
    private:
      using raster_type = typename IndicatorInputRaster::raster_type;
      using weight_raster_type = typename IndicatorInputRaster::weight_raster_type;
    public:
      using iterator = indicator_iterator < raster_type, weight_raster_type, Orientation,
        Element, Access > ;

      indicator_input_view(IndicatorInputRaster* raster = nullptr) : m_raster(raster)
      {}

      int size1()const
      {
        return blink::raster::raster_operations::size1(*m_raster);
      }
      int size2()const
      {
        return blink::raster::raster_operations::size2(*m_raster);
      }

      iterator begin()
      {
        iterator i(m_raster->get_raster(), m_raster->get_weight());
        i.find_begin();
        return i;
      }

      iterator end()
      {
        iterator i(m_raster->get_raster(), m_raster->get_weight());
        i.find_end();
        return i;
      }
      IndicatorInputRaster* m_raster;
    };
  }
}

namespace blink
{
  namespace raster
  {
    template<class Orientation, class Element, class Access, class Raster, class Weight>
    struct raster_view_lookup < Orientation, Element, Access, moving_window::indicator_input_raster<Raster, Weight> >
    {
      using type = moving_window::indicator_input_view < Orientation, Element, Access,
        moving_window::indicator_input_raster<Raster, Weight> > ;
    };

    template<class Orientation, class Access, class Raster, class Weight>
    struct raster_view_lookup < Orientation, element::h_edge, Access, moving_window::indicator_input_raster<Raster, Weight> >
    {
      using type = moving_window::indicator_input_view < Orientation, element::h_edge, Access,
        moving_window::indicator_input_raster<Raster, Weight> >;
    };

    template<class Orientation, class Access, class Raster, class Weight>
    struct raster_view_lookup < Orientation, element::h_edge_first_only, Access, moving_window::indicator_input_raster<Raster, Weight> >
    {
      using type = moving_window::indicator_input_view < Orientation, element::h_edge_first_only, Access,
        moving_window::indicator_input_raster<Raster, Weight> >;
    };
    template<class Orientation, class Access, class Raster, class Weight>
    struct raster_view_lookup < Orientation, element::h_edge_second_only, Access, moving_window::indicator_input_raster<Raster, Weight> >
    {
      using type = moving_window::indicator_input_view < Orientation, element::h_edge_second_only, Access,
        moving_window::indicator_input_raster<Raster, Weight> >;
    };

    template<class Orientation, class Access, class Raster, class Weight>
    struct raster_view_lookup < Orientation, element::v_edge, Access, moving_window::indicator_input_raster<Raster, Weight> >
    {
      using type = moving_window::indicator_input_view < Orientation, element::v_edge, Access,
        moving_window::indicator_input_raster<Raster, Weight> >;
    };

    template<class Orientation, class Access, class Raster, class Weight>
    struct raster_view_lookup < Orientation, element::v_edge_first_only, Access, moving_window::indicator_input_raster<Raster, Weight> >
    {
      using type = moving_window::indicator_input_view < Orientation, element::v_edge_first_only, Access,
        moving_window::indicator_input_raster<Raster, Weight> >;
    };
    template<class Orientation, class Access, class Raster, class Weight>
    struct raster_view_lookup < Orientation, element::v_edge_second_only, Access, moving_window::indicator_input_raster<Raster, Weight> >
    {
      using type = moving_window::indicator_input_view < Orientation, element::v_edge_second_only, Access,
        moving_window::indicator_input_raster<Raster, Weight> >;
    };
  }
// by setting these raster traits, the raster views become available!
  /*  namespace raster_traits
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
  */
} // namespace moving_window 
#endif
