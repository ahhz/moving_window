//
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// The moving_window_edge_iterator provides a generalized interface for both
// non-square moving window iterators of pixel (or patch) based indicators

#ifndef NON_SQUARE_WINDOW_ITERATOR_H_AHZ
#define NON_SQUARE_WINDOW_ITERATOR_H_AHZ

#include <moving_window/indicator_input_raster.h>
#include <moving_window/default_construction_functor.h>

#include <algorithm> // min max
#include <vector>
#include <utility> // pair

#include <boost/none_t.hpp>
#include <boost/optional.hpp>

namespace moving_window {

  template<typename Indicator, typename Raster, typename Window>
  class non_square_window_iterator; //forward declaration

  template<typename Indicator, typename Raster, typename Window>
  class non_square_window_iterator_facade
  {
  public:
    typedef non_square_window_iterator<Indicator, Raster, Window> base;
    friend class base;
    typedef typename boost::iterator_facade
      < base, const Indicator, boost::forward_traversal_tag > type;
  };

  template<typename Indicator, typename Raster, typename Window>
  class non_square_window_iterator
    : public non_square_window_iterator_facade< Indicator, Raster, Window>::type
  {
  public:
    typedef typename Raster::coordinate_type coordinate_type;
    typedef typename coordinate_type::index_type index_type;

  private:
    typedef Raster input_data_type;
    typedef raster_view<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::pixel,
      raster_iterator_tag::access::read_only,
      input_data_type> regular_view;

    typedef raster_view<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::pixel,
      raster_iterator_tag::access::read_only,
      input_data_type> trans_view;

    typedef typename regular_view::iterator input_iterator;
    typedef typename trans_view::iterator input_trans_iterator;


    typedef boost::optional<input_iterator> optional_input_iterator;
    typedef boost::optional<input_trans_iterator> optional_input_trans_iterator;

    // .first gives the offset relative to the current position of the iterator
    // .second gives the iterator at the offset position. Provided it is within 
    // the raster.
    typedef std::vector<std::pair<coordinate_type, optional_input_iterator> >
      iterator_vector;
    typedef std::vector<std::pair<coordinate_type, optional_input_trans_iterator>
    > trans_iterator_vector;

  public:
    template<typename Initializer = default_construction_functor<Indicator> >
    non_square_window_iterator(Window& window, Raster* data
      , Initializer initializer = Initializer()) : m_window(&window), m_data(data)
      , m_data_regular(data), m_data_trans(data), m_initializer(initializer)
      , m_indicator(initializer()), m_indicator_start_of_row(initializer())
      , m_coordinates(0, 0)
    {}

    void find_begin()
    {
      m_coordinates = coordinate_type(0, 0);
      Window& window = *m_window;
      for (index_type i = window.min_row_offset(); i <= window.max_row_offset() + 1;
        ++i)  {
        for (index_type j = window.min_col_offset(); j <= window.max_col_offset() + 1;
          ++j)  {
          if (window.has_offset(static_cast<int>(i), static_cast<int>(j))) {
            if (!window.has_offset(static_cast<int>(i - 1), static_cast<int>(j))) {
              m_top.push_back(std::make_pair(coordinate_type(i, j), boost::none));
            }

            if (!window.has_offset(static_cast<int>(i), static_cast<int>(j - 1))) {
              const index_type row = i;
              const index_type col = j - 1;

              if (col < 0) {
                m_left_behind.push_back(std::make_pair(coordinate_type(row, col),
                  boost::none));
              }
              else if (col == 0) {
                m_left_on.push_back(std::make_pair(coordinate_type(row, col),
                  boost::none));
              }
              else {
                m_left_ahead.push_back(std::make_pair(coordinate_type(row, col),
                  boost::none));
              }
            }

            if (!window.has_offset(static_cast<int>(i + 1), static_cast<int>(j))) {
              m_bottom.push_back(std::make_pair(coordinate_type(i, j),
                boost::none));
            }

            if (!window.has_offset(static_cast<int>(i), static_cast<int>(j + 1))) {
              const index_type row = i;
              const index_type col = j;
              if (col < 0) {
                m_right_behind.push_back(std::make_pair(coordinate_type(row, col),
                  boost::none));
              }
              else if (col == 0) {
                m_right_on.push_back(std::make_pair(coordinate_type(row, col),
                  boost::none));
              }
              else {
                m_right_ahead.push_back(std::make_pair(coordinate_type(row, col),
                  boost::none));
              }
            }
          }
        }
      }
      input_iterator iter = begin_input();
      index_type row = 0;
      for (index_type i = 0; i < std::min(size1(), window.max_row_offset() + 1); ++i,
        ++row)  {
        iter.find(coordinate_type(row, 0));
        for (index_type j = 0; j < std::min(size2(), window.max_col_offset() + 1);
          ++j, ++iter)  {
          if (window.has_offset(static_cast<int>(i), static_cast<int>(j))) {
            add_iterator_to_indicator(iter, m_indicator);
          }
        }
      }
      m_indicator_start_of_row = m_indicator;
    }

    void find_end()
    {
      m_coordinates = coordinate_type(size1(), 0);
      nullify_iterator_vector(m_right_on);
      nullify_iterator_vector(m_right_ahead);
      nullify_iterator_vector(m_right_behind);
      nullify_iterator_vector(m_left_on);
      nullify_iterator_vector(m_left_ahead);
      nullify_iterator_vector(m_left_behind);
      nullify_iterator_vector(m_top);
      nullify_iterator_vector(m_bottom);
      m_indicator_start_of_row = m_initializer();
      m_indicator = m_initializer();
    }

    const coordinate_type& get_coordinates() const
    {
      return m_coordinates;
    }

  private:
    friend class boost::iterator_core_access;

    void increment()
    {
      // todo: catch increment after end
      if (++m_coordinates.col == size2()) {
        m_coordinates.col = 0;
        if (++m_coordinates.row != size1()) {
          moved_down();
        }
      }
      else {
        moved_right();
      }
    }

    bool equal(const non_square_window_iterator& that) const
    {
      return m_coordinates == that.m_coordinates;
    }

    const Indicator& dereference() const
    {
      return get_indicator();
    }

  private:
    // The row index has incremented already, now catch up indicator;
    void moved_right()
    {

      const index_type endRow = size1() - m_coordinates.row;

      if (m_right_behind.size() > 0) {

        typename iterator_vector::iterator i, i_begin, i_end;
        boost::tie(i_begin, i_end) = iterator_vector_range(m_right_behind);
        i = i_begin;

        for (; i != i_end; ++i) {

          if (i->second) {
            ++(*(i->second));
            add_iterator_to_indicator(*(i->second), m_indicator);
          }
          else {
            const coordinate_type nb = m_coordinates + i->first;

            if (nb.col < size2()){
              i->second = begin_input();
              (i->second)->find(nb);
              add_iterator_to_indicator(*(i->second), m_indicator);
            }
          }
        }
      }

      if (m_right_on.size() > 0) {

        typename iterator_vector::iterator i, i_begin, i_end;
        boost::tie(i_begin, i_end) = iterator_vector_range(m_right_on);
        i = i_begin;

        for (; i != i_end; ++i)
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = begin_input();
            (i->second)->find(m_coordinates + i->first);
          }
          add_iterator_to_indicator(*(i->second), m_indicator);
        }
      }
      if (m_right_ahead.size() > 0) {

        typename iterator_vector::iterator i, i_begin, i_end;
        boost::tie(i_begin, i_end) = iterator_vector_range(m_right_ahead);
        i = i_begin;

        for (; i != i_end; ++i) {
          const coordinate_type nb = m_coordinates + i->first;
          if (nb.col < size2()) {
            if (i->second) {
              ++(*(i->second));
            }
            else {
              i->second = begin_input();
              (i->second)->find(nb);
            }
            add_iterator_to_indicator(*(i->second), m_indicator);
          }
        }
      }

      if (m_left_behind.size() > 0) {

        typename iterator_vector::iterator i, i_begin, i_end;
        boost::tie(i_begin, i_end) = iterator_vector_range(m_left_behind);
        i = i_begin;

        for (; i != i_end; ++i) {

          if (i->second) {
            ++(*(i->second));
            subtract_iterator_from_Indicator(*(i->second), m_indicator);
          }
          else  {
            const coordinate_type nb = m_coordinates + i->first;
            if (nb.col >= 0) {
              i->second = begin_input();
              (i->second)->find(nb);
              subtract_iterator_from_Indicator(*(i->second), m_indicator);
            }
          }
        }
      }

      if (m_left_on.size() > 0) {

        typename iterator_vector::iterator i, i_begin, i_end;
        boost::tie(i_begin, i_end) = iterator_vector_range(m_left_on);
        i = i_begin;

        for (; i != i_end; ++i) {

          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = begin_input();
            (i->second)->find(m_coordinates + i->first);
          }
          subtract_iterator_from_Indicator(*(i->second), m_indicator);
        }
      }

      if (m_left_ahead.size() > 0) {

        typename iterator_vector::iterator i, i_begin, i_end;
        boost::tie(i_begin, i_end) = iterator_vector_range(m_left_ahead);
        i = i_begin;

        for (; i != i_end; ++i) {

          const coordinate_type nb = m_coordinates + i->first;
          if (nb.col < size2()) {
            if (i->second) {
              ++(*(i->second));
            }
            else {
              i->second = begin_input();
              (i->second)->find(nb);
            }
            subtract_iterator_from_Indicator(*(i->second), m_indicator);
          }
        }
      }
    }

    // The col index has incremented already, now catch up indicator;
    void moved_down()
    {
      trans_iterator_vector::iterator i = m_bottom.begin();
      const trans_iterator_vector::iterator i_end = m_bottom.end();
      for (; i != i_end; ++i) {

        const coordinate_type nb = m_coordinates + i->first;
        if (is_in_range(nb.row, nb.col)) {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = begin_trans_input();
            (i->second)->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_indicator_start_of_row);
        }
      }

      trans_iterator_vector::iterator j = m_top.begin();
      const trans_iterator_vector::iterator j_end = m_top.end();
      for (; j != j_end; ++j)
      {
        const coordinate_type nb = m_coordinates + j->first +
          coordinate_type(-1, 0);
        if (is_in_range(nb.row, nb.col)) {
          if (j->second) {
            ++(*(j->second));
          }
          else {
            j->second = begin_trans_input();
            (j->second)->find(nb);
          }
          subtract_iterator_from_Indicator(*(j->second),
            m_indicator_start_of_row);
        }
      }
      m_indicator = m_indicator_start_of_row;

      // Invalidate all left to right iterators.
      nullify_iterator_vector(m_right_on);
      nullify_iterator_vector(m_right_ahead);
      nullify_iterator_vector(m_right_behind);
      nullify_iterator_vector(m_left_on);
      nullify_iterator_vector(m_left_ahead);
      nullify_iterator_vector(m_left_behind);
    }

    void nullify_iterator_vector(iterator_vector& v)
    {
      iterator_vector::iterator i = v.begin();
      const iterator_vector::iterator i_end = v.end();
      for (; i != i_end; ++i) {
        i->second = boost::none;
      }
    }

    void nullify_iterator_vector(trans_iterator_vector& v)
    {
      trans_iterator_vector::iterator i = v.begin();
      const trans_iterator_vector::iterator i_end = v.end();
      for (; i != i_end; ++i) {
        i->second = boost::none;
      }
    }

    std::pair<typename iterator_vector::iterator
      , typename iterator_vector::iterator>
      iterator_vector_range(iterator_vector& v) const
    {
        // Find the first iterator where offset + row >= 0
        typename iterator_vector::iterator begin = v.begin();
        //while(begin->first.row + m_coordinates.row < 0) {
        while (-begin->first.row > m_coordinates.row) {
          ++begin;
          if (begin == v.end()) return std::make_pair(v.end(), v.end());
        }

        typename iterator_vector::iterator end = v.begin() + (v.size() - 1);

        // Find the last iterator where offset + row < size1()
        index_type endRow = size1() - m_coordinates.row;
        //
        // while(end->first.row + m_coordinates.row >= size1() ) 
        while (end->first.row >= endRow) {
          if (end == v.begin()){
            return std::make_pair(v.end(), v.end());
          }
          --end;
        }
        ++end;
        return std::make_pair(begin, end);
    }

    typename iterator_vector::iterator
      iterator_vector_end(iterator_vector& v) const
    {
        iterator_vector::iterator i_end = v.begin() + v.size() - 1;

        index_type endRow = size1() - m_coordinates.row;

        while (i_end->first.row >= endRow) {
          if (i_end == v.begin()){
            return iterator_vector_begin(v);
          }
          --i_end;
        }
        return ++i_end;
    }


    index_type size1() const
    {
      return m_data->size1();
    }

    index_type size2() const
    {
      return m_data->size2();
    }

    const Indicator& get_indicator() const
    {
      return m_indicator;
    }

    bool is_in_range(const index_type& row, const index_type& col) const
    {
      return row >= 0 && row < size1() && col >= 0 && col < size2();
    }

    // Spliting m_Left into LeftAhead, m_left_on and m_left_behind, allowed for 
    // some efficiency gain. The same efficinecy gains could be made for 
    // m_top and m_bottom. But these are just for the first columns and not 
    // worth the bother now

    input_iterator begin_input()
    {
      return m_data_regular.begin();
    }

    input_trans_iterator begin_trans_input()
    {
      return m_data_trans.begin();
    }

    input_iterator end_input()
    {
      return m_data_regular.end();
    }

    input_trans_iterator end_trans_input()
    {
      return m_data_trans.end();
    }

    template<typename Iter, typename Accu>
    void add_iterator_to_indicator(Iter& iter, Accu& accu)
    {
      iter.add_to_indicator(accu);
    }

    template<typename Iter, typename Accu>
    void subtract_iterator_from_Indicator(Iter& iter, Accu& accu)
    {
      iter.subtract_from_to_indicator(accu);
    }

    Raster* m_data;
    Window* m_window;

    regular_view m_data_regular;
    trans_view m_data_trans;

    iterator_vector m_left_ahead, m_left_on, m_left_behind;
    iterator_vector m_right_ahead, m_right_on, m_right_behind;
    trans_iterator_vector m_top;
    trans_iterator_vector m_bottom;

    coordinate_type m_coordinates;
    construction_functor<Indicator> m_initializer;

    Indicator m_indicator;
    Indicator m_indicator_start_of_row;
  };

} // namespace moving_window 
#endif