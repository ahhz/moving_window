//
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Not for public distribution
//=======================================================================
//
// This file contains the functions for delineating patches in the dataset. 
// For very large datasets this can cause trouble as the algorihm creates a 
// std::deque of pixels to process that can turn very large.

#ifndef PATCH_DETECT_H_AHZ
#define PATCH_DETECT_H_AHZ

//#include <stack>
#include <boost/property_map/property_map.hpp>
#include <deque>
#include <type_traits>  //is_same 

namespace moving_window {

  template<typename Raster, typename Value>
  void fill_raster_uniform(Raster& r, const Value& v)
  {
    auto i = r.begin();
    auto i_end = r.end();
    for (; i != i_end; ++i)
    {
      (*i) = v;
    }
  }


  struct QueenTag{};
  struct RookTag{};

  template<typename InRaster, typename OutRaster, typename CatPropertyMap, typename AreaPropertyMap, typename PerimeterPropertyMap, typename DirectionTag>
  typename boost::property_traits<CatPropertyMap>::key_type patch_detect(const InRaster& in, OutRaster& out, CatPropertyMap cat, AreaPropertyMap area, PerimeterPropertyMap perim, DirectionTag&)
  {
    typedef OutRaster::coordinate_type coordinate;
    typedef OutRaster::value_type out_value_type;
    typedef boost::property_traits<CatPropertyMap>::key_type index_type;

    const int unpatched = -1;

    fill_raster_uniform(out, unpatched);

    const int rows = static_cast<int>(in.size1());
    const int cols = static_cast<int>(in.size2());

    index_type patch_index = 0;

    // Using deque (FIFO) instead of stack (LIFO) keeps the stack/deque smaller
    // and saves the day for the CORINE dataset
    // TODO: it remains a bottleneck and possibly replace for stack from STXXL
    std::deque<coordinate> pixel_stack;

    static const coordinate N(-1, 0);
    static const coordinate S(1, 0);
    static const coordinate E(0, 1);
    static const coordinate W(0, -1);
    static const coordinate NW(-1, -1);
    static const coordinate NE(-1, 1);
    static const coordinate SW(1, -1);
    static const coordinate SE(1, 1);

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        const coordinate coord(i, j);
        if (out.get(coord) == unpatched) {
          auto val = in.get(coord);
          out.put(coord, static_cast<out_value_type>(patch_index));

          pixel_stack.push_back(coord);
          int this_area = 1;
          int this_perim = 0;

          coordinate nb;

          auto rook_lambda = [&]() {
            if (in.get(nb) == val) {
              if (out.get(nb) == unpatched) {
                pixel_stack.push_back(nb);
                ++this_area;
                out.put(nb, static_cast<out_value_type>(patch_index));
              }
            }
            else {
              ++this_perim;
            }
          };

          auto queen_lambda = [&]()
          {
            if (in.get(nb) == val && out.get(nb) == unpatched) {
              pixel_stack.push_back(nb);
              ++this_area;
              out.put(nb, static_cast<out_value_type>(patch_index));
            }
          };

          while (!pixel_stack.empty())	{
            coordinate curr = pixel_stack.front(); pixel_stack.pop_front();

            nb = curr + N; if (nb.row > 0)      rook_lambda();
            nb = curr + S; if (nb.row < rows - 1) rook_lambda();
            nb = curr + W; if (nb.col > 0)      rook_lambda();
            nb = curr + E; if (nb.col < cols - 1) rook_lambda();

            if (std::is_same<DirectionTag, QueenTag>::value) { // compile time IF
              nb = curr + NW; if (nb.row > 0 && nb.col > 0)            queen_lambda();
              nb = curr + NE;	if (nb.row > 0 && nb.col < cols - 1)       queen_lambda();
              nb = curr + SW; if (nb.row < rows - 1 && nb.col > 0)       queen_lambda();
              nb = curr + SE; if (nb.row < rows - 1 && nb.col < cols - 1) 	queen_lambda();
            }
          } // while

          typedef typename boost::property_traits<CatPropertyMap>::value_type category_type;
          typedef typename boost::property_traits<AreaPropertyMap>::value_type area_type;
          typedef typename boost::property_traits<PerimeterPropertyMap>::value_type perimeter_type;

          put(cat, patch_index, static_cast<category_type>(val));
          put(area, patch_index, static_cast<area_type>(this_area));
          put(perim, patch_index, static_cast<perimeter_type>(this_perim));

          patch_index++;
        } // if
      } // j
    } //i
    return patch_index;
  } // function

  template<typename InRaster, typename OutRaster, typename CatPropertyMap
    , typename AreaPropertyMap, typename PerimeterPropertyMap>
    int patch_detect(const InRaster& in, OutRaster& out, CatPropertyMap cat
    , AreaPropertyMap area, PerimeterPropertyMap perim, bool queen)
  {
      if (queen)
        return static_cast<int>(patch_detect(in, out, cat, area, perim
        , QueenTag()));
      else
        return static_cast<int>(patch_detect(in, out, cat, area, perim
        , RookTag()));
  }

} // namespace moving_window 
#endif