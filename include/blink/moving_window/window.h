//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// Neighourhood, defines the shape of a kernel as a square vector-matrix of 
// bool. It is an ugly solution and not most efficient with memory and 
// computation, but OK for now

#ifndef BLINK_MOVING_WINDOW_WINDOW_H_AHZ
#define BLINK_MOVING_WINDOW_WINDOW_H_AHZ

#include <blink/raster/coordinate_2d.h>
#include <vector>
#include <math.h>

namespace blink {
  namespace moving_window {

    template<typename Coord = blink::raster::coordinate_2d >
    struct window
    {
      typedef typename Coord::index_type index_type;

      virtual bool has_offset(int rowoff, int coloff)  const = 0;

      virtual index_type max_row_offset() const = 0;
      virtual index_type min_row_offset() const = 0;
      virtual index_type max_col_offset() const = 0;
      virtual index_type min_col_offset() const = 0;
    };


    //template<typename Coord = coordinate_2d >
    struct circular_window : public window < blink::raster::coordinate_2d >
    {
      circular_window(double radius) : m_Radius(radius)
      {
        SetCircle(radius);
      }
      bool has_offset(int rowoff, int coloff)  const
      {
        return
          rowoff >= min_row_offset() && rowoff <= max_row_offset() &&
          coloff >= min_col_offset() && coloff <= max_col_offset() &&
          m_Include[rowoff + m_Center.row][coloff + m_Center.col];
      }

      index_type max_row_offset() const
      {
        return static_cast<index_type>(ceil(m_Radius));
      }

      index_type min_row_offset() const
      {
        return -static_cast<index_type>(ceil(m_Radius));
      }

      index_type max_col_offset() const
      {
        return static_cast<index_type>(ceil(m_Radius));
      }

      index_type min_col_offset() const
      {
        return -static_cast<index_type>(ceil(m_Radius));
      }

      double get_radius() const
      {
        return m_Radius;
      }

    private:
      double m_Radius;
      std::vector<std::vector<bool> > m_Include;
      blink::raster::coordinate_2d m_Center;

      void SetCircle(double radius)
      {
        int rSq = (int)(radius * radius);
        int r = (int)radius;
        m_Include.assign(2 * r + 1, std::vector<bool>(2 * r + 1, false));
        m_Center = blink::raster::coordinate_2d(r, r);

        for (int i = 0; i <= r; ++i)
        {
          for (int j = 0; j*j + i*i <= rSq; ++j) {
            m_Include[i + r][j + r] = true;
            m_Include[i + r][-j + r] = true;
            m_Include[-i + r][j + r] = true;
            m_Include[-i + r][-j + r] = true;
          }
        }
      }
    };



    struct square_window
    {
      square_window(int radius) : m_Radius(radius)
      {};
      int get_radius()
      {
        return m_Radius;
      }
    private:
      int m_Radius;
    };

    inline circular_window make_circular_window(double radius)
    {
      return circular_window(radius);
    }

    inline square_window make_square_window(double radius)
    {
      return square_window(static_cast<int>(radius));
    }



    template<typename T>
    struct window_family
    {
      typedef int type;
    };

    template<>
    struct window_family < square_window >
    {
      typedef square_tag type;
    };

    template<>
    struct window_family < typename circular_window >
    {
      typedef circle_tag type;
    };


    inline circular_window make_window(const circle_tag&, double radius)
    {
      return make_circular_window(radius);
    }

    inline square_window make_window(const square_tag&, int radius)
    {
      return make_square_window(radius);
    }
  } //namespace moving_window
}
#endif