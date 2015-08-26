//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// The class coordinate_2d is used to provide 2d coordinates as an index to 
// rasters. 
// It could possibly be taken from an external source, e.g. BOOST

#ifndef MOVING_WINDOW_EXCEPTIONS_H_AHZ
#define MOVING_WINDOW_EXCEPTIONS_H_AHZ

#include <boost/exception/all.hpp>

namespace moving_window {

  struct creating_a_raster_failed : public boost::exception, public std::exception
  {
    const char *what() const { return "creating a raster failed"; }
  };

  struct insufficient_memory_for_raster_block : public boost::exception, public std::exception
  {
    const char *what() const { return "insufficient memory for reading a raster block"; }
  };

  struct opening_raster_failed : public boost::exception, public std::exception
  {
    const char *what() const { return "opening raster failed"; }
  };
  struct reading_from_raster_failed : public boost::exception, public std::exception
  {
    const char *what() const { return "reading from raster failed"; }
  };

  struct writing_to_raster_failed : public boost::exception, public std::exception
  {
    const char *what() const { return "writing to raster failed"; }
  };

} //namespace moving_window 
#endif