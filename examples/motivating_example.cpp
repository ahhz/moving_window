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

#include <blink/iterator/zip_range.h>

#include <blink/moving_window/indicator/count.h>
#include <blink/moving_window/indicator/edge_density.h>
#include <blink/moving_window/indicator/area_weighted_patch_size.h>
#include <blink/moving_window/moving_window_view.h>
#include <blink/moving_window/window.h>

#include <blink/raster/utility.h>

#include <functional>
#include <tuple>


int main()
{
  double radius = 3;

  // open a raster data set
  auto input = blink::raster::open_gdal_raster<int>("copy.tif", GA_ReadOnly);
  /*
  auto copy = blink::raster::create_gdal_raster_from_model<int>("copy.tif", input);

  auto copy_zip = blink::iterator::make_zip_range(
    std::ref(input), std::ref(copy));

  // extract the indicator value for each pixel and assign to output
  for (auto&& i : copy_zip) {
    std::get<1>(i) = std::get<0>(i);
  }
  */
  // create a raster data set, with same dimensions as input
  auto output = blink::raster::create_gdal_raster_from_model<double>("output.tif", input);

  // create the window to use
  auto window = blink::moving_window::make_square_window(radius);

  // Create a range over the windowed indicator for all pixels
  auto indicator_tag = blink::moving_window::area_weighted_patch_size_tag{};
  auto window_view = blink::moving_window::make_moving_window_view(indicator_tag, window, &input);

  // Create a range to simultaneously iterate over output and window_view
  auto zip = blink::iterator::make_zip_range(
    std::ref(output), std::ref(window_view));

  // extract the indicator value for each pixel and assign to output
  for (auto&& i : zip) {
    auto& output_i = std::get<0>(i);
    auto& window_i = std::get<1>(i);
    output_i = window_i.extract().get();
  }

  return 0;
}