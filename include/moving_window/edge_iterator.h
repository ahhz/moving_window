//
//=======================================================================
// Copyright 2013-2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// This file presents iterators over the edges between cells in a raster
//
// The iterators (try to) use the concepts as proposed in the context of the BOOST library
//
// http://www.boost.org/doc/libs/1_52_0/libs/iterator/doc/new-iter-concepts.html
//
// TODO: the edge iterators are now const iterators only
//
// There are four edge-iterators
//
// h_edge_iterator 
//   - iterates over all horizontal edges in a raster
//   - line-by-line left-to-right.
//
// h_edge_trans_iterator 
//   - iterates over all horizontal edges in a raster
//   - column-by-column top-to-bottom.
//
//  v_edge_iterator 
//   - iterates over all vertical edges in a raster
//   - line-by-line left-to-right.
//
// v_edge_trans_iterator 
//   - iterates over all vertical edges in a raster
//   - column-by-column top-to-bottom.
//
// Dereferencing an edge_iterator gives two boost::optional-wrapped cell values
// h_edge iterators consist of one above the edge (first) and one below the edge (second).
// v_edge_iterators consist of one left of the edge (first) and one right of the edge (second).
//
// When there is no cell to the left/right/top/bottom of an edge, then the associated value 
// of the optional cell iterator is boost::none.
//
// The coordinate of a h_edge is that of the cell below.
// The coordinate of a v_edge is that of the cell to the right.
//
// This is chosen to the effect that coordinates are always positive: 0 <= i <= size1 and 0 <= j <= size2
//
// The following member functions are provided:
//   constructor(raster)
//   std::pair<Value, Value> get() gets the values next to the edge
//   operator++() increment iterator
//   find_begin(), find_end()
//   find(coordinates)
//   coordinates get_coordinates()
//
// there are three different get-strategies
// both      : return both values along the edge, if present.
// left_only : replaces right for boost::none
// right_only: replaces left for boost::none
// TODO: There is some code duplication here.
//

#ifndef EDGE_ITERATOR_H_AHZ
#define EDGE_ITERATOR_H_AHZ

#include <moving_window/raster_traits.h>

#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/optional/optional.hpp>

#include <type_traits>
#include <utility>

namespace moving_window {

  namespace get_strategy
  {
    struct both{};
    struct first_only{};
    struct second_only{};
  };

  template<typename Raster>
  struct return_type_helper
  {
    typedef boost::optional<typename Raster::value_type> optional_pixel_value;
    typedef std::pair<optional_pixel_value, optional_pixel_value> type;
  };



  template<typename Raster, typename GetStrategy = get_strategy::both>
  class h_edge_iterator :
    public boost::iterator_facade<
    h_edge_iterator<Raster, GetStrategy>,
    typename return_type_helper<Raster>::type,
    boost::random_access_traversal_tag,
    typename return_type_helper<Raster>::type, // return the value not a reference
    std::ptrdiff_t>
  {
    typedef h_edge_iterator<Raster, GetStrategy> this_type;
    typedef h_edge_iterator<const Raster, GetStrategy> this_type_const;
    typedef h_edge_iterator<typename std::remove_const<Raster>::type, GetStrategy> this_type_non_const;
  public:
    typedef typename Raster::coordinate_type coordinate_type;
    typedef typename coordinate_type::index_type index_type;

    typedef boost::optional<typename Raster::value_type> optional_pixel_value;
    typedef std::pair<optional_pixel_value, optional_pixel_value> value_type;

    typedef raster_view<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::pixel,
      raster_iterator_tag::access::read_only,
      Raster> pixel_view;

    typedef typename pixel_view::iterator pixel_iterator;

  public:
    h_edge_iterator(Raster* r = NULL) : m_pixel_view(r)
    {
      m_iterator_first = boost::none;
      m_iterator_second = boost::make_optional(r != NULL, m_pixel_view.begin());
    }

    void find_begin()
    {
      m_iterator_first = boost::none;
      m_iterator_second = m_pixel_view.begin();
    }

    void find_end()
    {
      m_iterator_first = pixel_view::end();// TODO : or boost::none???
      m_iterator_second = boost::none;
    }

    // finds edge above given coordinates
    void find(const coordinate_type& coordinates)
    {
      if (coordinates.row > 0)	{
        if (!m_iterator_first) {
          m_iterator_first = m_pixel_view.begin();
        }
        m_iterator_first->find(coordinates + coordinate_type(-1, 0));
      }
      else {
        m_iterator_first = boost::none;
      }

      if (coordinates.row < size1())	{
        if (!m_iterator_second) {
          m_iterator_second = m_pixel_view.begin();
        }
        m_iterator_second->find(coordinates);
      }
      else {
        m_iterator_second = boost::none;
      }
    }

    // returns coordinates below current edge
    coordinate_type get_coordinates() const
    {
      if (m_iterator_second) return m_iterator_second->get_coordinates();
      return m_iterator_first->get_coordinates() + coordinate_type(1, 0);
    }

  private:
    index_type size1() const
    {
      return m_pixel_view.m_raster->size1();
    }

    index_type size2() const
    {
      return m_pixel_view.m_raster->size2();
    }

    // necessary for iterator_facade
    friend class boost::iterator_core_access;

    // necessary for iterator_facade
    void increment()
    {
      // Once iter1 is valid. it remains valid
      if (m_iterator_first) {
        ++(*m_iterator_first);
      }
      else if (m_iterator_second->get_coordinates().col == size2() - 1) { // will progress from row 0 to row 1
        m_iterator_first = m_pixel_view.begin();
      }

      // Once iter2 is invalid. it remains invalid
      if (m_iterator_second) {
        ++(*m_iterator_second);
        if (*m_iterator_second == m_pixel_view.end()) {
          m_iterator_second = boost::none;
        }
      }
    }

    // necessary for iterator_facade
    void decrement()
    {
      if (m_iterator_second) {
        --m_iterator_second;
      }
      else if (m_iterator_first.get_coordinates().col == 0) {
        // will degress from last row to penultimate row.
        m_iterator_second = pixel_view::begin() + size1() * size2() - 1;
        // last cell in last row
      }

      if (m_iterator_first) {
        if (m_iterator_first != pixel_view::begin()) {
          --m_iterator_first;
        }
        else {
          m_iterator_first == boost::none;
        }
      }
    }

    // necessary for iterator_facade
    void advance(std::ptrdiff_t n)
    {
      index_type i = index() + n;
      coordinate_type c = index_to_coordinate(i);
      find(c);
    }


    // necessary for iterator_facade
    bool equal(const this_type_non_const& other) const
    {
      return get_coordinates() == other.get_coordinates();
    }

    bool equal(const this_type_const& other) const
    {
      return get_coordinates() == other.get_coordinates();
    }

    value_type dereference() const
    {
      return get_specialized(GetStrategy());
    }

    std::ptrdiff_t distance_to(const this_type_non_const& other) const
    {
      return other.index() - index();
    }

    std::ptrdiff_t distance_to(const this_type_const& other) const
    {
      return other.index() - this->index();
    }

    std::ptrdiff_t index() const
    {
      return get_coordinates().row * size2() + get_coordinates().col;
    }

    coordinate_type index_to_coordinate(index_type i)
    {
      return coordinate_type(i / size2(), i % size2());
    }

    value_type get_specialized(get_strategy::both) const
    {
      optional_pixel_value v1;
      optional_pixel_value v2;
      if (m_iterator_first) {
        v1 = **m_iterator_first;
      }
      if (m_iterator_second) {
        v2 = **m_iterator_second;
      }
      return std::make_pair(v1, v2);
    }

    value_type get_specialized(get_strategy::first_only) const
    {
      optional_pixel_value v1;
      optional_pixel_value v2;

      if (m_iterator_first) {
        v1 = **m_iterator_first;
      }

      return std::make_pair(v1, v2);
    }

    value_type get_specialized(get_strategy::second_only) const
    {
      optional_pixel_value v1;
      optional_pixel_value v2;

      if (m_iterator_second) {
        v2 = **m_iterator_second;
      }

      return std::make_pair(v1, v2);
    }

    boost::optional<pixel_iterator> m_iterator_first, m_iterator_second;
    pixel_view m_pixel_view;
  };

  template<typename Raster, typename GetStrategy = get_strategy::both>
  class h_edge_trans_iterator :
    public boost::iterator_facade<
    h_edge_trans_iterator<Raster, GetStrategy>,
    typename return_type_helper<Raster>::type,
    boost::random_access_traversal_tag,
    typename return_type_helper<Raster>::type,
    std::ptrdiff_t>
  {
    typedef h_edge_trans_iterator<Raster, GetStrategy> this_type;
    typedef h_edge_trans_iterator<const Raster, GetStrategy> this_type_const;
    typedef h_edge_trans_iterator<typename boost::remove_const<Raster>::type,
      GetStrategy> this_type_non_const;
  public:
    typedef typename Raster::coordinate_type coordinate_type;
    typedef typename coordinate_type::index_type index_type;

    typedef boost::optional<typename Raster::value_type> optional_pixel_value;
    typedef std::pair<optional_pixel_value, optional_pixel_value> value_type;

    typedef typename raster_view<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::pixel,
      raster_iterator_tag::access::read_only,
      Raster> pixel_view;

    typedef typename pixel_view::iterator pixel_iterator;

    h_edge_trans_iterator(Raster* r = NULL) : m_pixel_view(r)
    {
      m_iterator_first = boost::none;
      m_iterator_second = boost::make_optional(r != NULL, m_pixel_view.begin());
    }

    void find_begin()
    {
      m_iterator_first = boost::none;
      m_iterator_second = m_pixel_view.begin();
    }

    void find_end()
    {
      m_iterator_first = m_pixel_view.end();
      m_iterator_second = boost::none;
    }

    // finds edge above given coordinate
    void find(const coordinate_type& coordinates)
    {
      if (coordinates.row > 0)	{
        if (!m_iterator_first) {
          m_iterator_first = m_pixel_view.begin();
        }
        m_iterator_first->find(coordinates + coordinate_type(-1, 0));
      }
      else {
        m_iterator_first = boost::none;
      }

      if (coordinates.row < size1())	{
        if (!m_iterator_second) {
          m_iterator_second = m_pixel_view.begin();
        }
        m_iterator_second->find(coordinates);
      }
      else {
        m_iterator_second = boost::none;
      }
    }

    coordinate_type get_coordinates() const
    {
      if (m_iterator_second) return m_iterator_second->get_coordinates();
      return m_iterator_first->get_coordinates() + coordinate_type(1, 0);
    }


    index_type size1() const
    {
      return m_pixel_view.m_raster->size1();
      // +1??? TODO: decide what is size in this context
    }

    index_type size2() const
    {
      return m_pixel_view.m_raster->size2();
    }

  private:
    friend class boost::iterator_core_access;

    void increment()
    {
      if (!m_iterator_second) {
        // at any time, either iter1 or iter2 (or both) is initialized
        if (++(*m_iterator_first) == m_pixel_view.end())	{
          m_iterator_second = boost::none;
          m_iterator_first = boost::none;
        }
        else {
          m_iterator_second = ++(*m_iterator_first);
          m_iterator_first = boost::none;
        }
      }
      else if (m_iterator_second->get_coordinates().row == size1() - 1){
        m_iterator_first = m_iterator_second;
        m_iterator_second = boost::none;
      }
      else {
        m_iterator_first = m_iterator_second;
        ++(*m_iterator_second);
      }
    }

    void decrement()
    {
      if (!m_iterator_first) {
        if (*m_iterator_second == m_pixel_view::begin()){
          m_iterator_first = boost::none;
          m_iterator_second = boost::none;
        }
        else {
          m_iterator_first = --(*m_iterator_second);
          m_iterator_second = boost::none;
        }
      }
      else if (m_iterator_first->get_coordinates().row == 0){
        m_iterator_second = m_iterator_first;
        m_iterator_first = boost::none;
      }
      else {
        m_iterator_second = m_iterator_first;
        --(*m_iterator_first);
      }
    }

    void advance(std::ptrdiff_t n)
    {
      std::ptrdiff_t i = index() + n;
      coordinate_type c = index_to_coordinate(i);
      find(c);
    }

    value_type dereference() const
    {
      return get_specialized(GetStrategy());
    }

    std::ptrdiff_t distance_to(const this_type_non_const& other) const
    {
      return other.index() - index();
    }

    std::ptrdiff_t distance_to(const this_type_const& other) const
    {
      return other.index() - this->index();
    }

    bool equal(const this_type_non_const& that) const
    {
      return get_coordinates() != that.get_coordinates();
    }

    bool equal(const this_type_const& that) const
    {
      return get_coordinates() != that.get_coordinates();
    }

    std::ptrdiff_t index() const
    {
      return get_coordinates().col * (size1() + 1) + get_coordinates().row;
    }

    coordinate_type index_to_coordinate(index_type i) const
    {
      return coordinate_type(i % (size1() + 1), i / (size1() + 1));
    }

    value_type get_specialized(get_strategy::both) const
    {
      optional_pixel_value a = m_iterator_first
        ? optional_pixel_value(**m_iterator_first)
        : optional_pixel_value(boost::none);

      optional_pixel_value b = m_iterator_second
        ? optional_pixel_value(**m_iterator_second)
        : optional_pixel_value(boost::none);
      return std::make_pair(a, b);
    }
    value_type get_specialized(get_strategy::first_only) const
    {
      optional_pixel_value a = m_iterator_first
        ? optional_pixel_value(**m_iterator_first)
        : optional_pixel_value(boost::none);
      optional_pixel_value b = boost::none;
      return std::make_pair(a, b);
    }

    value_type get_specialized(get_strategy::second_only) const
    {
      optional_pixel_value a = boost::none;
      optional_pixel_value b = m_iterator_second
        ? optional_pixel_value(**m_iterator_second)
        : optional_pixel_value(boost::none);
      return std::make_pair(a, b);
    }

    boost::optional<pixel_iterator> m_iterator_first, m_iterator_second;
    pixel_view m_pixel_view;
  };

  template<typename Raster, typename GetStrategy = get_strategy::both>
  class v_edge_iterator :
    public boost::iterator_facade<
    v_edge_iterator<Raster, GetStrategy>,
    typename Raster::value_type,
    boost::random_access_traversal_tag,
    typename return_type_helper<Raster>::type,
    std::ptrdiff_t>
  {
    typedef v_edge_iterator<Raster, GetStrategy> this_type;
    typedef v_edge_iterator<const Raster, GetStrategy> this_type_const;
    typedef v_edge_iterator<typename boost::remove_const<Raster>::type,
      GetStrategy> this_type_non_const;
  public:
    typedef typename Raster::coordinate_type coordinate_type;
    typedef typename coordinate_type::index_type index_type;

    typedef boost::optional<typename Raster::value_type> optional_pixel_value;
    typedef std::pair<optional_pixel_value, optional_pixel_value> value_type;

    typedef typename raster_view<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::pixel,
      raster_iterator_tag::access::read_only,
      Raster> pixel_view;

    typedef typename pixel_view::iterator pixel_iterator;

  public:
    v_edge_iterator(Raster* r = NULL) : m_pixel_view(r)
    {
      m_iterator_first = boost::none;
      m_iterator_second = boost::make_optional(r != NULL, m_pixel_view.begin());
    }

    void find_begin()
    {
      m_iterator_first = boost::none;
      m_iterator_second = m_pixel_view.begin();
    }

    void find_end()
    {
      m_iterator_first = boost::none;
      m_iterator_second = m_pixel_view.end();
    }

    // finds edge left of given coordinate
    void find(const coordinate_type& coordinates)
    {
      if (coordinates.col > 0)	{
        if (!m_iterator_first)  {
          m_iterator_first = m_pixel_view.begin();
        }
        m_iterator_first->find(coordinates + coordinate_type(0, -1));
      }
      else {
        m_iterator_first = boost::none;
      }

      if (coordinates.col < size2())	{
        if (!m_iterator_second)  {
          m_iterator_second = m_pixel_view.begin();
        }
        m_iterator_second->find(coordinates);
      }
      else {
        m_iterator_second = boost::none;
      }
    }

    // Finds coordinates to right of edge
    coordinate_type get_coordinates() const
    {
      if (m_iterator_second) return m_iterator_second->get_coordinates();
      return m_iterator_first->get_coordinates() + coordinate_type(0, 1);
    }

    index_type size1() const
    {
      return m_pixel_view.m_raster->size1();
    }

    index_type size2() const
    {
      return m_pixel_view.m_raster->size2();
    }

  private:
    friend class boost::iterator_core_access;

    void increment()
    {
      if (m_iterator_first) { // at any time, either iter1 or iter2 is initialized
        ++(*m_iterator_first);
        if (m_iterator_second) {
          ++(*m_iterator_second);
          if (m_iterator_second->get_coordinates().col == 0) {
            m_iterator_second = boost::none;
          }
        }
        else { //!m_iterator_second
          m_iterator_second = m_iterator_first;
          m_iterator_first = boost::none;
        }
      }
      else { //!m_iterator_first
        m_iterator_first = m_iterator_second;
        ++(*m_iterator_second);
        if (m_iterator_second->get_coordinates().col == 0) {
          m_iterator_second = boost::none;
        }
      }
    }

    void decrement()
    {
      if (m_iterator_second) { // at any time, either iter1 or iter2 is initialized
        --(*m_iterator_second);
        if (m_iterator_first) {
          --(*m_iterator_first);
          if (m_iterator_second->get_coordinates().col == 0) {
            m_iterator_first = boost::none;
          }
        }
        else { //!m_iterator_second
          m_iterator_first = m_iterator_second;
          m_iterator_second = boost::none;
        }
      }
      else { //!m_iterator_second
        m_iterator_second = m_iterator_first;
        --(*m_iterator_first);
        if (m_iterator_second->get_coordinates().col == 0) {
          m_iterator_first = boost::none;
        }
      }
    }

    void advance(std::ptrdiff_t n)
    {
      find(index_to_coordinate(index() + n));
    }

    friend class boost::iterator_core_access;

    value_type dereference() const
    {
      return get_specialized(GetStrategy());
    }

    std::ptrdiff_t distance_to(const this_type_non_const& other) const
    {
      return other.index() - index();
    }

    std::ptrdiff_t distance_to(const this_type_const& other) const
    {
      return other.index() - index();
    }

    value_type get_specialized(get_strategy::both) const
    {
      optional_pixel_value a = m_iterator_first
        ? optional_pixel_value(**m_iterator_first)
        : optional_pixel_value(boost::none);
      optional_pixel_value b = m_iterator_second
        ? optional_pixel_value(**m_iterator_second)
        : optional_pixel_value(boost::none);
      return std::make_pair(a, b);
    }
    value_type get_specialized(get_strategy::first_only) const
    {
      optional_pixel_value a = m_iterator_first
        ? optional_pixel_value(**m_iterator_first)
        : optional_pixel_value(boost::none);
      return std::make_pair(a, boost::none);
    }

    value_type get_specialized(get_strategy::second_only) const
    {
      optional_pixel_value b = m_iterator_second
        ? optional_pixel_value(**m_iterator_second)
        : optional_pixel_value(boost::none);
      return std::make_pair(boost::none, b);
    }

    bool equal(const this_type_non_const& that) const
    {
      return get_coordinates() == that.get_coordinates();
    }

    bool equal(const this_type_const& that) const
    {
      return get_coordinates() == that.get_coordinates();
    }

    std::ptrdiff_t index() const
    {
      return get_coordinates().row * (size2() + 1) + get_coordinates().col;
    }

    coordinate_type index_to_coordinate(index_type i) const
    {
      return coordinate_type(i / (size2() + 1), i % (size2() + 1));
    }
    boost::optional<pixel_iterator> m_iterator_first, m_iterator_second;
    pixel_view m_pixel_view;
  };

  template<typename Raster, typename GetStrategy = get_strategy::both>
  class v_edge_trans_iterator :
    public boost::iterator_facade<
    v_edge_trans_iterator<Raster, GetStrategy>,
    typename Raster::value_type,
    boost::random_access_traversal_tag,
    typename return_type_helper<Raster>::type,
    std::ptrdiff_t>
  {
    typedef v_edge_trans_iterator<Raster, GetStrategy> this_type;
    typedef v_edge_trans_iterator<const Raster, GetStrategy> this_type_const;
    typedef v_edge_trans_iterator<typename boost::remove_const<Raster>::type,
      GetStrategy> this_type_non_const;
  public:
    typedef typename Raster::coordinate_type coordinate_type;
    typedef typename coordinate_type::index_type index_type;
    typedef boost::optional<typename Raster::value_type> optional_pixel_value;
    typedef std::pair<optional_pixel_value, optional_pixel_value> value_type;

    typedef typename raster_traits::iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::pixel,
      raster_iterator_tag::access::read_only,
      Raster>::type pixel_iterator;

  public:
    v_edge_trans_iterator(Raster* r = NULL) : m_Raster(r)
    {
      m_iterator_first = boost::none;
      m_iterator_second = boost::make_optional(r != NULL, pixel_begin());
    }

    value_type dereference() const
    {
      return get_specialized(GetStrategy());
    }

    std::ptrdiff_t distance_to(const this_type_non_const& other) const
    {
      return other.index() - index();
    }

    std::ptrdiff_t distance_to(const this_type_const& other) const
    {
      return other.index() - index();
    }

    value_type get_specialized(get_strategy::both) const
    {
      optional_pixel_value a = m_iterator_first
        ? optional_pixel_value(**m_iterator_first)
        : optional_pixel_value(boost::none);
      optional_pixel_value b = m_iterator_second
        ? optional_pixel_value(**m_iterator_second)
        : optional_pixel_value(boost::none);
      return std::make_pair(a, b);
    }
    value_type get_specialized(get_strategy::first_only) const
    {
      optional_pixel_value a = m_iterator_first
        ? optional_pixel_value(**m_iterator_first)
        : optional_pixel_value(boost::none);
      return std::make_pair(a, boost::none);
    }

    value_type get_specialized(get_strategy::second_only) const
    {
      optional_pixel_value b = m_iterator_second
        ? optional_pixel_value(**m_iterator_second)
        : optional_pixel_value(boost::none);
      return std::make_pair(boost::none, b);
    }


    void increment()
    {
      // Once iter2 is invalid. it remains invalid
      if (m_iterator_second) {
        ++(*m_iterator_second);
        if (m_iterator_second->get_coordinates().col == size2()) {
          m_iterator_second = boost::none;
        }
      }

      // Once iter1 is valid. it remains valid
      if (m_iterator_first) {
        ++(*m_iterator_first);
      }
      else if (m_iterator_second && m_iterator_second->get_coordinates().col > 0)
      {
        m_iterator_first = (*m_iterator_second) - size1();
      }
    }

    void decrement()
    {
      // Once iter1 is invalid. it remains invalid
      if (m_iterator_first) {
        --(*m_iterator_first);
        if (m_iterator_second->get_coordinates().col == 0) {
          m_iterator_first = boost::none;
        }
      }

      // Once iter2 is valid. it remains valid
      if (m_iterator_second) {
        --(*m_iterator_second)
      }
      else if (m_iterator_first && m_iterator_first.get_coordinates().col > 0) {
        m_iterator_second = m_iterator_first + size1();
      }
    }

    void advance(std::ptrdiff_t n)
    {
      find(index_to_coordinate(index() + n));
    }

    void find_begin()
    {
      m_iterator_first = boost::none;
      m_iterator_second = pixel_begin();
    }

    void find_end()
    {
      m_iterator_first = pixel_end();
      m_iterator_second = boost::none;
    }

    // always finds left of given coordinate
    void find(const coordinate_type& coordinates)
    {
      if (coordinates.col > 0)	{
        if (!m_iterator_first) {
          m_iterator_first = pixel_begin();
        }
        m_iterator_first->find(coordinates + coordinate_type(0, -1));
      }
      else {
        m_iterator_first = boost::none;
      }

      if (coordinates.col < size2())	{
        if (!m_iterator_second) {
          m_iterator_second = pixel_begin();
        }
        m_iterator_second->find(coordinates);
      }
      else {
        m_iterator_second = boost::none;
      }
    }

    //give coordinates on the right of current edge
    coordinate_type get_coordinates() const
    {
      if (m_iterator_second) return m_iterator_second->get_coordinates();
      return m_iterator_first->get_coordinates() + coordinate_type(0, 1);
    }

    bool equal(const this_type_const& that) const
    {
      return m_iterator_first == that.m_iterator_first
        && m_iterator_second == that.m_iterator_second;
    }

    bool equal(const this_type_non_const& that) const
    {
      return m_iterator_first == that.m_iterator_first
        && m_iterator_second == that.m_iterator_second;
    }
  private:
    index_type size1() const
    {
      return m_Raster->size1();
    }

    index_type size2() const
    {
      return m_Raster->size2();
    }
  private:
    std::ptrdiff_t index() const
    {
      return get_coordinates().col * size1() + get_coordinates().row;
    }

    coordinate_type index_to_coordinate(index_type i) const
    {
      return coordinate_type(i % size1(), i / size1());
    }

    pixel_iterator pixel_begin()
    {
      return m_Raster->begin<pixel_iterator>();
    };

    pixel_iterator pixel_end()
    {
      return m_Raster->end<pixel_iterator>();
    };

    boost::optional<pixel_iterator> m_iterator_first, m_iterator_second;
    Raster* m_Raster;
  };
  namespace raster_traits
  {
    template <typename Raster>
    struct iterator<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::h_edge,
      raster_iterator_tag::access::read_only
      , Raster>
    {
      typedef h_edge_iterator<Raster, get_strategy::both> type;
    };

    template <typename Raster>
    struct iterator<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::v_edge,
      raster_iterator_tag::access::read_only
      , Raster>
    {
      typedef v_edge_iterator<Raster, get_strategy::both> type;
    };

    template <typename Raster>
    struct iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::h_edge,
      raster_iterator_tag::access::read_only
      , Raster>
    {
      typedef h_edge_trans_iterator<Raster, get_strategy::both> type;
    };

    template <typename Raster>
    struct iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::v_edge,
      raster_iterator_tag::access::read_only
      , Raster>
    {
      typedef v_edge_trans_iterator<Raster, get_strategy::both> type;
    };

    template <typename Raster>
    struct iterator<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::h_edge_first_only,
      raster_iterator_tag::access::read_only
      , Raster>
    {
      typedef h_edge_iterator<Raster, get_strategy::first_only> type;
    };

    template <typename Raster>
    struct iterator<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::v_edge_first_only,
      raster_iterator_tag::access::read_only
      , Raster>
    {
      typedef v_edge_iterator<Raster, get_strategy::first_only> type;
    };

    template <typename Raster>
    struct iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::h_edge_first_only,
      raster_iterator_tag::access::read_only
      , Raster>
    {
      typedef h_edge_trans_iterator<Raster, get_strategy::first_only> type;
    };

    template <typename Raster>
    struct iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::v_edge_first_only,
      raster_iterator_tag::access::read_only
      , Raster>
    {
      typedef v_edge_trans_iterator<Raster, get_strategy::first_only> type;
    };

    template <typename Raster>
    struct iterator<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::h_edge_second_only,
      raster_iterator_tag::access::read_only
      , Raster>
    {
      typedef h_edge_iterator<Raster, get_strategy::second_only> type;
    };

    template <typename Raster>
    struct iterator<
      raster_iterator_tag::orientation::row_major,
      raster_iterator_tag::element::v_edge_second_only,
      raster_iterator_tag::access::read_only
      , Raster>
    {
      typedef v_edge_iterator<Raster, get_strategy::second_only> type;
    };

    template <typename Raster>
    struct iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::h_edge_second_only,
      raster_iterator_tag::access::read_only
      , Raster>
    {
      typedef h_edge_trans_iterator<Raster, get_strategy::second_only> type;
    };

    template <typename Raster>
    struct iterator<
      raster_iterator_tag::orientation::col_major,
      raster_iterator_tag::element::v_edge_second_only,
      raster_iterator_tag::access::read_only
      , Raster>
    {
      typedef v_edge_trans_iterator<Raster, get_strategy::second_only> type;
    };

  } // namespace raster_traits

} // namespace moving_window
#endif