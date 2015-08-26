//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// Contains the motivating example for the Moving Window library 

#include <moving_window/indicator/edge_density.h>
#include <moving_window/moving_window_view.h>
#include <moving_window/utility.h>
#include <moving_window/window.h>
#include <moving_window/zip_range.h>

#include <functional>
#include <tuple>

using namespace moving_window;

int main()
{
  double radius = 10;

  // open a raster data set
  auto input = open_gdal_raster<int>("input.tif", GA_ReadOnly);

  // create a raster data set, with same dimensions as input
  auto output = create_gdal_raster_from_model<double>("output.tif", input);

  // create the window to use
  auto window = make_square_window(radius);

  // Create a range over the windowed indicator for all pixels
  auto window_view = make_moving_window_view(edge_density_tag(), window, &input);

  // Create a range to simultaneously iterate over output and window_view
  auto zip = make_zip_range(std::ref(output), std::ref(window_view));

  // extract the indicator value for each pixel and assign to output
  for (auto& i : zip) {
    auto& output_i = std::get<0>(i);
    auto& window_i = std::get<1>(i);
    output_i = window_i.extract().get();
  }

  return 0;
}