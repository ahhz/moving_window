//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// The moving_window_edge_iterator provides a generalized interface for both
// non-square moving window iterators of edge based indicators
// TODO: should we disallow copying?

#ifndef NON_SQUARE_WINDOW_EDGE_ITERATOR_H_AHZ
#define NON_SQUARE_WINDOW_EDGE_ITERATOR_H_AHZ

#include <moving_window/indicator_input_raster.h>
#include <moving_window/default_construction_functor.h>

#include <boost/none_t.hpp>
#include <boost/optional.hpp>

#include <algorithm> // min max
#include <utility> // pair
#include <vector>

namespace moving_window {

  template<typename Indicator, typename Raster, typename Window>
  class non_square_window_edge_iterator; //forward declaration

  template<typename Indicator, typename Raster, typename Window>
  class non_square_window_edge_iterator_facade
  {
  public:
    typedef non_square_window_edge_iterator<Indicator, Raster, Window> base;
    friend class base;
    typedef typename boost::iterator_facade
      < base, const Indicator, boost::forward_traversal_tag > type;
  };

  template<typename Indicator, typename Raster, typename Window>
  class non_square_window_edge_iterator :
    public non_square_window_edge_iterator_facade<Indicator, Raster, Window>::type
  {
    typedef non_square_window_edge_iterator this_type;
    typedef typename Raster::coordinate_type coordinate_type;
    typedef typename Raster::index_type index_type;
    typedef Raster input_data_type;

    typedef raster_iterator_tag::orientation::row_major row_major;
    typedef raster_iterator_tag::orientation::col_major col_major;
    typedef raster_iterator_tag::element::v_edge v_edge;
    typedef raster_iterator_tag::element::v_edge_first_only v1_edge;
    typedef raster_iterator_tag::element::v_edge_second_only v2_edge;
    typedef raster_iterator_tag::element::h_edge h_edge;
    typedef raster_iterator_tag::element::h_edge_first_only h1_edge;
    typedef raster_iterator_tag::element::h_edge_second_only h2_edge;
    typedef raster_iterator_tag::access::read_only read_only;

    typedef raster_view<row_major, v_edge, read_only, input_data_type>  v_view;
    typedef raster_view<row_major, v1_edge, read_only, input_data_type> v1_view;
    typedef raster_view<row_major, v2_edge, read_only, input_data_type> v2_view;
    typedef raster_view<row_major, h_edge, read_only, input_data_type>  h_view;
    typedef raster_view<row_major, h1_edge, read_only, input_data_type> h1_view;
    typedef raster_view<row_major, h2_edge, read_only, input_data_type> h2_view;
    typedef raster_view<col_major, v_edge, read_only, input_data_type>  v_t_view;
    typedef raster_view<col_major, v1_edge, read_only, input_data_type> v1_t_view;
    typedef raster_view<col_major, v2_edge, read_only, input_data_type> v2_t_view;
    typedef raster_view<col_major, h_edge, read_only, input_data_type>  h_t_view;
    typedef raster_view<col_major, h1_edge, read_only, input_data_type> h1_t_view;
    typedef raster_view<col_major, h2_edge, read_only, input_data_type> h2_t_view;

    typedef typename  v_view::iterator  v_iterator;
    typedef typename v1_view::iterator v1_iterator;
    typedef typename v2_view::iterator v2_iterator;
    typedef typename  h_view::iterator  h_iterator;
    typedef typename h1_view::iterator h1_iterator;
    typedef typename h2_view::iterator h2_iterator;
    typedef typename  v_t_view::iterator  v_t_iterator;
    typedef typename v1_t_view::iterator v1_t_iterator;
    typedef typename v2_t_view::iterator v2_t_iterator;
    typedef typename  h_t_view::iterator  h_t_iterator;
    typedef typename h1_t_view::iterator h1_t_iterator;
    typedef typename h2_t_view::iterator h2_t_iterator;

    typedef boost::optional<v_iterator> opt_v_iterator;
    typedef boost::optional<v1_iterator> opt_v1_iterator;
    typedef boost::optional<v2_iterator> opt_v2_iterator;
    typedef boost::optional<h_iterator> opt_h_iterator;
    typedef boost::optional<h1_iterator> opt_h1_iterator;
    typedef boost::optional<h2_iterator> opt_h2_iterator;
    typedef boost::optional<v_t_iterator> opt_v_t_iterator;
    typedef boost::optional<v1_t_iterator> opt_v1_t_iterator;
    typedef boost::optional<v2_t_iterator> opt_v2_t_iterator;
    typedef boost::optional<h_t_iterator> opt_h_t_iterator;
    typedef boost::optional<h1_t_iterator> opt_h1_t_iterator;
    typedef boost::optional<h2_t_iterator> opt_h2_t_iterator;

    // .first gives the offset relative to the current position of the iterator
    // .second gives the iterator at the offset position. Provided it is within the raster.
    typedef coordinate_type ct;
    typedef std::vector<std::pair<ct, opt_v_iterator> > v_iterator_vector;
    typedef std::vector<std::pair<ct, opt_v1_iterator> > v1_iterator_vector;
    typedef std::vector<std::pair<ct, opt_v2_iterator> > v2_iterator_vector;
    typedef std::vector<std::pair<ct, opt_h_iterator> > h_iterator_vector;
    typedef std::vector<std::pair<ct, opt_h1_iterator> > h1_iterator_vector;
    typedef std::vector<std::pair<ct, opt_h2_iterator> > h2_iterator_vector;
    typedef std::vector<std::pair<ct, opt_v_t_iterator> > v_trans_iterator_vector;
    typedef std::vector<std::pair<ct, opt_v1_t_iterator>> v1_trans_iterator_vector;
    typedef std::vector<std::pair<ct, opt_v2_t_iterator>> v2_trans_iterator_vector;
    typedef std::vector<std::pair<ct, opt_h_t_iterator> > h_trans_iterator_vector;
    typedef std::vector<std::pair<ct, opt_h1_t_iterator>> h1_trans_iterator_vector;
    typedef std::vector<std::pair<ct, opt_h2_t_iterator>> h2_trans_iterator_vector;

  public:
    template<typename Initializer = default_construction_functor<Indicator> >
    non_square_window_edge_iterator(Window& window, input_data_type* data,
      Initializer initializer = Initializer())
      : m_window(&window), m_Data(data), m_Initializer(initializer)
      , m_Indicator(initializer()), m_IndicatorStartOfRow(initializer())
    {
    }

    void find_begin()
    {
      Window& window = *m_window;
      h_iterator h_temp = m_Data->begin<h_iterator>();
      h1_iterator h1_temp = m_Data->begin<h1_iterator>();
      h2_iterator h2_temp = m_Data->begin<h2_iterator>();
      v_iterator v_temp = m_Data->begin<v_iterator>();
      v1_iterator v1_temp = m_Data->begin<v1_iterator>();
      v2_iterator v2_temp = m_Data->begin<v2_iterator>();

      auto test_in_window = [&](const coordinate_type& window_coord){
        return window.has_offset(static_cast<int>(window_coord.row), static_cast<int>(window_coord.col)); };

      auto test_in_map = [&](const coordinate_type& map_coord)->bool {
        return map_coord.row >= 0 && map_coord.row < size1()
          && map_coord.col >= 0 && map_coord.col < size2(); };

      auto test_in_corner_window = [&](const coordinate_type& window_coord)->bool {
        return test_in_window(window_coord) && test_in_map(window_coord); };

      for (index_type i = 0; i <= window.max_row_offset() + 1; ++i)  {
        for (index_type j = 0; j <= window.max_col_offset() + 1; ++j)  {

          coordinate_type elem(i, j);
          coordinate_type bottom1(i + 1, j);
          coordinate_type right1(i, j + 1);
          coordinate_type top1(i - 1, j);
          coordinate_type top2(i - 2, j);
          coordinate_type left1(i, j - 1);
          coordinate_type left2(i, j - 2);
          coordinate_type topleft(i - 1, j - 1);
          coordinate_type topright(i - 1, j + 1);
          coordinate_type bottomleft(i + 1, j - 1);
          coordinate_type bottomright(i + 1, j + 1);

          bool in_elem = test_in_corner_window(elem);
          bool in_bottom1 = test_in_corner_window(bottom1);
          bool in_right1 = test_in_corner_window(right1);
          bool in_top1 = test_in_corner_window(top1);
          bool in_top2 = test_in_corner_window(top2);
          bool in_left1 = test_in_corner_window(left1);
          bool in_left2 = test_in_corner_window(left2);
          bool in_topleft = test_in_corner_window(topleft);
          bool in_topright = test_in_corner_window(topright);
          bool in_bottomleft = test_in_corner_window(bottomleft);
          bool in_bottomright = test_in_corner_window(bottomright);

          // INITIALIZE INDICATOR
          if (in_elem && in_left1){
            v_temp.find(elem);
            add_iterator_to_indicator(v_temp, m_Indicator);
          }
          if (in_elem && !in_left1) {
            v2_temp.find(elem);
            add_iterator_to_indicator(v2_temp, m_Indicator);
          }
          if (!in_elem && in_left1) {
            v1_temp.find(elem);
            add_iterator_to_indicator(v1_temp, m_Indicator);
          }

          if (in_elem && in_top1) {
            h_temp.find(elem);
            add_iterator_to_indicator(h_temp, m_Indicator);
          }
          if (in_elem && !in_top1) {
            h2_temp.find(elem);
            add_iterator_to_indicator(h2_temp, m_Indicator);
          }
          if (!in_elem && in_top1){
            h1_temp.find(elem);
            add_iterator_to_indicator(h1_temp, m_Indicator);
          }
        }
      }

      for (index_type i = window.min_row_offset(); i <= window.max_row_offset() + 1; ++i)  {
        for (index_type j = window.min_col_offset(); j <= window.max_col_offset() + 1; ++j)  {
          coordinate_type elem(i, j);
          coordinate_type bottom1(i + 1, j);
          coordinate_type right1(i, j + 1);
          coordinate_type top1(i - 1, j);
          coordinate_type top2(i - 2, j);
          coordinate_type left1(i, j - 1);
          coordinate_type left2(i, j - 2);
          coordinate_type topleft(i - 1, j - 1);
          coordinate_type topright(i - 1, j + 1);
          coordinate_type bottomleft(i + 1, j - 1);
          coordinate_type bottomright(i + 1, j + 1);
          bool in_elem = test_in_window(elem);
          bool in_bottom1 = test_in_window(bottom1);
          bool in_right1 = test_in_window(right1);
          bool in_top1 = test_in_window(top1);
          bool in_top2 = test_in_window(top2);
          bool in_left1 = test_in_window(left1);
          bool in_left2 = test_in_window(left2);
          bool in_topleft = test_in_window(topleft);
          bool in_topright = test_in_window(topright);
          bool in_bottomleft = test_in_window(bottomleft);
          bool in_bottomright = test_in_window(bottomright);

          // INITIALIZE ALL ITERATORS
          if (in_elem && in_left1 && !(in_left2))
          {
            m_VLeft.push_back(std::make_pair(elem, boost::none));
          }

          if (in_elem && in_left1 && !(in_right1))
          {
            m_VRight.push_back(std::make_pair(elem, boost::none));
          }

          if (in_elem && !in_left1)
          {
            m_V2Left.push_back(std::make_pair(elem, boost::none));
            m_V2Right.push_back(std::make_pair(elem, boost::none));
          }

          if (!in_elem && in_left1)
          {
            m_V1Left.push_back(std::make_pair(elem, boost::none));
            m_V1Right.push_back(std::make_pair(elem, boost::none));
          }

          if (in_elem && in_left1 && !(in_top1 && in_topleft))
          {
            m_VTop.push_back(std::make_pair(elem, boost::none));
          }
          if (in_elem && !in_left1 && !(in_top1 && !in_topleft))
          {
            m_V2Top.push_back(std::make_pair(elem, boost::none));
          }

          if (!in_elem && in_left1 && !(!in_top1 && in_topleft))
          {
            m_V1Top.push_back(std::make_pair(elem, boost::none));
          }

          if (in_elem && in_left1 && !(in_bottom1 && in_bottomleft))
          {
            m_VBottom.push_back(std::make_pair(elem, boost::none));
          }

          if (in_elem && !in_left1 && !(in_bottom1 && !in_bottomleft))
          {
            m_V2Bottom.push_back(std::make_pair(elem, boost::none));
          }

          if (!in_elem && in_left1 && !(!in_bottom1 && in_bottomleft))
          {
            m_V1Bottom.push_back(std::make_pair(elem, boost::none));
          }

          if (in_elem && in_top1 && !(in_top2))
          {
            m_HTop.push_back(std::make_pair(elem, boost::none));
          }

          if (in_elem && in_top1 && !(in_bottom1))
          {
            m_HBottom.push_back(std::make_pair(elem, boost::none));
          }

          if (in_elem && !in_top1)
          {
            m_H2Bottom.push_back(std::make_pair(elem, boost::none));
            m_H2Top.push_back(std::make_pair(elem, boost::none));
          }

          if (!in_elem && in_top1)
          {
            m_H1Bottom.push_back(std::make_pair(elem, boost::none));
            m_H1Top.push_back(std::make_pair(elem, boost::none));
          }

          if (in_elem && in_top1 && !(in_left1 && in_topleft))
          {
            m_HLeft.push_back(std::make_pair(elem, boost::none));
          }

          if (in_elem && !in_top1 && !(in_left1 && !in_topleft))
          {
            m_H2Left.push_back(std::make_pair(elem, boost::none));
          }

          if (!in_elem && in_top1 && !(!in_left1 && in_topleft))
          {
            m_H1Left.push_back(std::make_pair(elem, boost::none));
          }

          if (in_elem && in_top1 && !(in_right1 && in_topright))
          {
            m_HRight.push_back(std::make_pair(elem, boost::none));
          }

          if (in_elem && !in_top1 && !(in_right1 && !in_topright))
          {
            m_H2Right.push_back(std::make_pair(elem, boost::none));
          }

          if (!in_elem && in_top1 && !(!in_right1 && in_topright))
          {
            m_H1Right.push_back(std::make_pair(elem, boost::none));
          }
        }
      }
      m_IndicatorStartOfRow = m_Indicator;
    }

    void find_end()
    {
      m_Coordinates = coordinate_type(size1(), 0);
      // TODO clear up mess
    }

  private:
    friend class boost::iterator_core_access;
    void increment()
    {
      // todo: catch increment after end
      if (++m_Coordinates.col == size2()) {
        m_Coordinates.col = 0;
        if (++m_Coordinates.row != size1()) {
          MovedDown();
        }
      }
      else {
        MovedRight();
      }
    }

    bool equal(const this_type& that) const
    {
      return m_Coordinates == that.m_Coordinates;
    }

    const Indicator& dereference() const
    {
      return get_indicator();
    }

  private:
    void MovedRight() // The row index has incremented already, now catch up indicator;
    {
      for (auto i = m_VRight.begin(); i != m_VRight.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first;
        if (v_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<v_iterator>();
            i->second->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_Indicator);
        }
        else {
          i->second = boost::none;
        }
      }
      for (auto i = m_V1Right.begin(); i != m_V1Right.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first;
        if (v_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<v1_iterator>();
            i->second->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_Indicator);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_V2Right.begin(); i != m_V2Right.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first;
        if (v_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<v2_iterator>();
            i->second->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_Indicator);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_VLeft.begin(); i != m_VLeft.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first - coordinate_type(0, 1);
        if (v_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<v_iterator>();
            i->second->find(nb);
          }
          subtract_iterator_from_Indicator(*(i->second), m_Indicator);
        }
        else {
          i->second = boost::none;
        }
      }
      for (auto i = m_V1Left.begin(); i != m_V1Left.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first - coordinate_type(0, 1);
        if (v_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<v1_iterator>();
            i->second->find(nb);
          }
          subtract_iterator_from_Indicator(*(i->second), m_Indicator);
        }
        else {
          i->second = boost::none;
        }
      }
      for (auto i = m_V2Left.begin(); i != m_V2Left.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first - coordinate_type(0, 1);
        if (v_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<v2_iterator>();
            i->second->find(nb);
          }
          subtract_iterator_from_Indicator(*(i->second), m_Indicator);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_HRight.begin(); i != m_HRight.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first;
        if (h_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<h_iterator>();
            i->second->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_Indicator);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_H1Right.begin(); i != m_H1Right.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first;
        if (h_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<h1_iterator>();
            i->second->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_Indicator);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_H2Right.begin(); i != m_H2Right.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first;
        if (h_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<h2_iterator>();
            i->second->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_Indicator);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_HLeft.begin(); i != m_HLeft.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first - coordinate_type(0, 1);
        if (h_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<h_iterator>();
            i->second->find(nb);
          }
          subtract_iterator_from_Indicator(*(i->second), m_Indicator);
        }
        else {
          i->second = boost::none;
        }
      }
      for (auto i = m_H1Left.begin(); i != m_H1Left.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first - coordinate_type(0, 1);
        if (h_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<h1_iterator>();
            i->second->find(nb);
          }
          subtract_iterator_from_Indicator(*(i->second), m_Indicator);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_H2Left.begin(); i != m_H2Left.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first - coordinate_type(0, 1);
        if (h_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<h2_iterator>();
            i->second->find(nb);
          }
          subtract_iterator_from_Indicator(*(i->second), m_Indicator);
        }
        else {
          i->second = boost::none;
        }
      }

    }

    void MovedDown() // The col index has incremented already, now catch up indicator;
    {
      for (auto i = m_VBottom.begin(); i != m_VBottom.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first;
        if (v_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<v_t_iterator>();
            i->second->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_IndicatorStartOfRow);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_V1Bottom.begin(); i != m_V1Bottom.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first;
        if (v_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<v1_t_iterator>();
            i->second->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_IndicatorStartOfRow);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_V2Bottom.begin(); i != m_V2Bottom.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first;
        if (v_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<v2_t_iterator>();
            i->second->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_IndicatorStartOfRow);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_HBottom.begin(); i != m_HBottom.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first;
        if (h_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<h_t_iterator>();
            i->second->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_IndicatorStartOfRow);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_H1Bottom.begin(); i != m_H1Bottom.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first;
        if (h_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<h1_t_iterator>();
            i->second->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_IndicatorStartOfRow);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_H2Bottom.begin(); i != m_H2Bottom.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first;
        if (h_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<h2_t_iterator>();
            i->second->find(nb);
          }
          add_iterator_to_indicator(*(i->second), m_IndicatorStartOfRow);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_VTop.begin(); i != m_VTop.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first - coordinate_type(1, 0);
        if (v_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<v_t_iterator>();
            i->second->find(nb);
          }
          subtract_iterator_from_Indicator(*(i->second), m_IndicatorStartOfRow);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_V1Top.begin(); i != m_V1Top.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first - coordinate_type(1, 0);
        if (v_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<v1_t_iterator>();
            i->second->find(nb);
          }
          subtract_iterator_from_Indicator(*(i->second), m_IndicatorStartOfRow);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_V2Top.begin(); i != m_V2Top.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first - coordinate_type(1, 0);
        if (v_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<v2_t_iterator>();
            i->second->find(nb);
          }
          subtract_iterator_from_Indicator(*(i->second), m_IndicatorStartOfRow);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_HTop.begin(); i != m_HTop.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first - coordinate_type(1, 0);
        if (h_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<h_t_iterator>();
            i->second->find(nb);
          }
          subtract_iterator_from_Indicator(*(i->second), m_IndicatorStartOfRow);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_H1Top.begin(); i != m_H1Top.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first - coordinate_type(1, 0);
        if (h_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<h1_t_iterator>();
            i->second->find(nb);
          }
          subtract_iterator_from_Indicator(*(i->second), m_IndicatorStartOfRow);
        }
        else {
          i->second = boost::none;
        }
      }

      for (auto i = m_H2Top.begin(); i != m_H2Top.end(); ++i)
      {
        coordinate_type nb = m_Coordinates + i->first - coordinate_type(1, 0);
        if (h_is_in_range(nb))
        {
          if (i->second) {
            ++(*(i->second));
          }
          else {
            i->second = m_Data->begin<h2_t_iterator>();
            i->second->find(nb);
          }
          subtract_iterator_from_Indicator(*(i->second), m_IndicatorStartOfRow);
        }
        else {
          i->second = boost::none;
        }
      }
      m_Indicator = m_IndicatorStartOfRow;

      // Invalidate all left to right iterators.
      nullify_iterator_vector(m_VLeft);
      nullify_iterator_vector(m_VRight);
      nullify_iterator_vector(m_HLeft);
      nullify_iterator_vector(m_HRight);
      nullify_iterator_vector(m_V1Left);
      nullify_iterator_vector(m_V1Right);
      nullify_iterator_vector(m_H1Left);
      nullify_iterator_vector(m_H1Right);
      nullify_iterator_vector(m_V2Left);
      nullify_iterator_vector(m_V2Right);
      nullify_iterator_vector(m_H2Left);
      nullify_iterator_vector(m_H2Right);
    }

    template<typename IteratorVector>
    void nullify_iterator_vector(IteratorVector& v)
    {
      typename IteratorVector::iterator i = v.begin();
      const typename IteratorVector::iterator i_end = v.end();
      for (; i != i_end; ++i) {
        i->second = boost::none;
      }
    }

    index_type size1() const
    {
      return m_Data->size1();
    }

    index_type size2() const
    {
      return m_Data->size2();
    }

    const coordinate_type& get_coordinates() const
    {
      return m_Coordinates;
    }

    const Indicator& get_indicator() const
    {
      return m_Indicator;
    }

    //bool is_in_range(const index_type& row, const index_type& col) const
    //{
    //	return row >= 0 && row < size1() && col >= 0 && col < size2();
    //}

    bool v_is_in_range(const coordinate_type& coord) const
    {
      return v_is_in_range(coord.row, coord.col);
    }
    bool h_is_in_range(const coordinate_type& coord) const
    {
      return h_is_in_range(coord.row, coord.col);
    }
    bool v_is_in_range(const index_type& row, const index_type& col) const
    {
      return row >= 0 && row < size1() && col >= 0 && col <= size2();
    }

    bool h_is_in_range(const index_type& row, const index_type& col) const
    {
      return row >= 0 && row <= size1() && col >= 0 && col < size2();
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

    input_data_type* m_Data;
    Window* m_window;

    v_iterator_vector m_VLeft, m_VRight;
    h_iterator_vector m_HLeft, m_HRight;
    v_trans_iterator_vector m_VTop, m_VBottom;
    h_trans_iterator_vector m_HTop, m_HBottom;

    v1_iterator_vector m_V1Left, m_V1Right;
    h1_iterator_vector m_H1Left, m_H1Right;
    v1_trans_iterator_vector m_V1Top, m_V1Bottom;
    h1_trans_iterator_vector m_H1Top, m_H1Bottom;

    v2_iterator_vector m_V2Left, m_V2Right;
    h2_iterator_vector m_H2Left, m_H2Right;
    v2_trans_iterator_vector m_V2Top, m_V2Bottom;
    h2_trans_iterator_vector m_H2Top, m_H2Bottom;

    coordinate_type m_Coordinates;
    construction_functor<Indicator> m_Initializer;

    Indicator m_Indicator;
    Indicator m_IndicatorStartOfRow;
  };

} // namespace moving_window 

#endif