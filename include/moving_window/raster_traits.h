//
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// raster_traits are used to inspect properties of rasters
//     - value_type
//     - coordinate_type
//     - index_type
//     - iterator
// raster_operations define free functions to be overloaded for each raster type. 
// that default to appropriate member functions. 

//! Traits to make it easier to conform existing raster/matrix classes to requirements, 
//! by specializing these structs.  

#ifndef RASTER_TRAITS_H_AHZ
#define RASTER_TRAITS_H_AHZ

namespace moving_window {

  namespace raster_traits
  {
    //! Value type is the type of elements in a raster 
    template<typename Raster>
    struct value_type
    {
      typedef typename Raster::value_type type;
    };

    //! Coordinate type is the type used to describe coordinates in a raster 
    template<typename Raster>
    struct coordinate_type
    {
      typedef typename Raster::coordinate_type type;
    };

    //! index_type is the integer type used to enumerate rows, columns or elements
    template<typename Raster>
    struct index_type
    {
      typedef typename Raster::index_type type;
    };

    //! iterator: rasters can be iterated over in various ways 
    //! row_major / col_major orientation, over pixels / h_edges or v_edges
    //! , readonly / update
    template <typename OrientationTag, typename ElementTag, typename AccessTag, typename RasterType>
    struct iterator
    {
      struct has_no_default_implementation{};
      typedef has_no_default_implementation type;
    };

  };

  //! Wraps raster member functions in free functions to make it easier to conform existing 
  //! raster/matrix classes to requirements, by overloading these functions.  


  //! Required for SpecializedIterator concept
  namespace raster_operations
  {
    template <typename OrientationTag, typename ElementTag, typename AccessTag, typename Raster>
    typename raster_traits::iterator< OrientationTag, ElementTag, AccessTag, Raster>::type
      begin(const Raster& r)
    {
        typedef typename raster_traits::iterator< OrientationTag, ElementTag, AccessTag, Raster>::type
          iterator;
        return r.begin<iterator>();
    }
    template <typename OrientationTag, typename ElementTag, typename AccessTag, typename Raster>
    typename raster_traits::iterator< OrientationTag, ElementTag, AccessTag, Raster>::type
      end(const Raster& r)
    {
        typedef typename raster_traits::iterator< OrientationTag, ElementTag, AccessTag, Raster>::type
          iterator;
        return r.end<iterator>();
    }
  }


  namespace raster_operations
  {
    //! Required for RasterConcept returns the number of rows
    template<typename Raster>
    typename raster_traits::index_type<Raster>::type size1(const Raster& r)
    {
      return r.size1();
    }

    //! Required for RasterConcept returns the number of columns
    template<typename Raster>
    typename raster_traits::index_type<Raster>::type  size2(const Raster& r)
    {
      return r.size2();
    }
  }

  namespace raster_operations
  {
    //! Required for the BlockedRasterConcept returns the number of major rows
    template<typename Raster>
    typename raster_traits::index_type<Raster>::type block_size1(const Raster& r)
    {
      return r.block_size1();
    }

    //! Required for the BlockedRasterConcept returns the number of major columns
    //!
    template<typename Raster>
    typename raster_traits::index_type<Raster>::type  block_size2(const Raster& r)
    {
      return r.block_size2();
    }

    //! Required for the BlockedRasterConcept returns the number value of one pixel inside a block
    //!
    template<typename Raster>
    typename raster_traits::value_type<Raster>::type get_pixel_in_block(
      const Raster& r,
      typename raster_traits::index_type<Raster>::type block,
      typename raster_traits::index_type<Raster>::type pixel_in_block)
    {
      return r.get_pixel_in_block(block, pixel_in_block);
    }

    //! Required for the BlockedRasterConcept sets the value of one pixel inside a block
    //!
    template<typename Raster>
    void put_pixel_in_block(
      Raster& r,
      const typename raster_traits::value_type<Raster>::type& value,
      typename raster_traits::index_type<Raster>::type block,
      typename raster_traits::index_type<Raster>::type pixel_in_block)
    {
      r.put_pixel_in_block(block, pixel_in_block, value);
    }
  };

  template <typename OrientationTag, typename ElementTag, typename AccessTag, typename RasterType>
  struct raster_view
  {
    typedef typename raster_traits::iterator<
      OrientationTag,
      ElementTag,
      AccessTag,
      RasterType>::type iterator;

    raster_view(RasterType* raster) : m_raster(raster)
    {

    }
    iterator begin()
    {
      return raster_operations::begin<OrientationTag, ElementTag, AccessTag, RasterType>(*m_raster);
    }

    iterator end()
    {
      return raster_operations::end<OrientationTag, ElementTag, AccessTag, RasterType>(*m_raster);
    }
    /* should depend on type of elem
    int size1() const
    {
    return raster_operations::size1(m_raster);
    }

    int size2() const
    {
    return raster_operations::size2(m_raster);
    }
    */

    RasterType* m_raster;
  };


} // namespace moving_window 

#endif