//
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Not for public distribution
//=======================================================================
//
// raster iterator tags are used to inspect the properties of a raster iterator
// These are:
//     row_major / col_major
//     pixel, h_edge, v_edge, h_edge_first_only, h_edge_second_only
//     read_only / read_write
//     
#ifndef RASTER_ITERATOR_H_AHZ
#define RASTER_ITERATOR_H_AHZ

namespace moving_window {

  // Namespaces instead of struct to allow new tags to be defined elsewere.
  namespace raster_iterator_tag
  {
    namespace orientation
    {
      struct row_major{};
      struct col_major{};
    };

    namespace element
    {
      struct pixel{};
      struct h_edge{};
      struct h_edge_first_only{};// first is top
      struct h_edge_second_only{};
      struct v_edge{};
      struct v_edge_first_only{}; //first is left
      struct v_edge_second_only{};
    };

    namespace access
    {
      struct read_only{};
      struct read_write{};
    };
  };

  // This needs to be specialized
  /*template <typename OrientationTag, typename ElementTag, typename AccessTag, typename RasterType>
  struct raster_iterator_selector
  {
  struct needs_specialization{};
  typedef needs_specialization type;
  };
  */

} // namespace moving_window 
#endif