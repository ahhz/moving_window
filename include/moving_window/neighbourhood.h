//
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Not for public distribution
//=======================================================================
//
// Neighourhood, defines the shape of a kernel as a square vector-matrix of bools
// It is an ugly solution and no most efficient with memory and computation
// , but OK for now

#ifndef NEIGHBOURHOOD_H_AHZ
#define NEIGHBOURHOOD_H_AHZ

#include <moving_window/coordinate_2d.h>
#include <vector>
#include <math.h>

template<typename Coord = coordinate_2d >
struct neighbourhood
{
  typedef typename Coord::index_type index_type;

  virtual bool has_offset(int rowoff, int coloff)  const = 0;
  
  virtual index_type max_row_offset() const = 0;
  virtual index_type min_row_offset() const = 0;
  virtual index_type max_col_offset() const = 0;
  virtual index_type min_col_offset() const = 0;
};


template<typename Coord = coordinate_2d >
struct circular_neighbourhood : public neighbourhood<Coord> 
{
  circular_neighbourhood(double radius) : m_Radius(radius)
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

private:
  double m_Radius;
  std::vector<std::vector<bool> > m_Include;
  Coord m_Center;

  void SetCircle(double radius)
  {
    int rSq = (int)(radius * radius);
    int r = (int)radius;
    m_Include.assign(2 * r + 1, std::vector<bool>(2 * r + 1, false));
    m_Center = Coord(r, r);

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



struct square_neighbourhood
{
  square_neighbourhood(int radius) : m_Radius(radius)
  {};
  int get_radius()
  {
    return m_Radius;
  }
private:
  int m_Radius;
};

inline circular_neighbourhood<coordinate_2d> make_circular_neighbourhood(double radius)
{
  return circular_neighbourhood<coordinate_2d>(radius);
}

inline square_neighbourhood make_square_neighbourhood(double radius)
{
  return square_neighbourhood(static_cast<int>(radius));
}

inline circular_neighbourhood<coordinate_2d> make_neighbourhood(
  const circle_tag&, double radius)
{
  return make_circular_neighbourhood(radius);
}

inline square_neighbourhood make_neighbourhood(
  const square_tag&, int radius)
{
  return make_square_neighbourhood(radius);
}

template<typename T>
struct window_family
{
  typedef int type;
};

template<>
struct window_family<square_neighbourhood>
{
  typedef square_tag type;
};

template<typename Coord>
struct window_family<typename circular_neighbourhood<Coord> >
{
  typedef circle_tag type;
};

#endif