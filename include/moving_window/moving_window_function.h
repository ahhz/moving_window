//
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
// this file is intended to support the python wrapper, but not finished at all...

#ifndef MOVING_WINDOW_FUNCTION_H_AHZ
#define MOVING_WINDOW_FUNCTION_H_AHZ

#include <boost/python.hpp>
#include <boost/python/enum.hpp>

//#include "accumulate_edge_density.h"
//#include "accumulate_interspersion.h"
//#include "square_window_edge_iterator.h"
//#include "non_square_window_edge_iterator.h" // NOT SUPPORTED

//#include "gdal_raster.h"
//#include <string>

namespace moving_window {

  enum window_type_enum
  {
    square,
    circle
  };

  enum metric_enum
  {
    edge_density,
    interspersion
  };
  /*
  boost::python::enum_<window_type_enum>("window_choice")
  .value("circle", circle)
  .value("square", square);

  boost::python::enum_<metric_enum>("metric_choice")
  .value("edge_density",edge_density)
  .value("interspersion",interspersion);
  */

} // namespace moving_window 

#endif