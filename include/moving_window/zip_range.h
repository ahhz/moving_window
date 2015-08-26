//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// Implements a zip range, it is not a fully functional generic zip range, 
// but supports the forward-only iteration necessary in the project

#ifndef ZIP_RANGE_H_AHZ
#define ZIP_RANGE_H_AHZ

#include <functional>
#include <tuple>
#include <utility>
//#include <vector>

namespace moving_window {

  template<typename T> struct unwrap { typedef T type; };
  template<typename T> struct unwrap<std::reference_wrapper<T>> { typedef T& type; };
  template<typename T> struct unwrap_unref { typedef T type; }; 
  template<typename T> struct unwrap_unref<std::reference_wrapper<T> > { typedef T type; };

  template<int ...> struct seq { };

  template<int N, int ...S> struct gens : gens<N - 1, N - 1, S...> { };
  template<int ...S> struct gens<0, S...> { typedef seq<S...> type; };

  struct empty {};

  template<typename...Args> void ignore(Args...)
  {}

  template < typename...Iterators >
  class zip_iterator
  {
  public:
    zip_iterator(Iterators&&...its) : iterators(std::forward<Iterators>(its)...)
    { }

    typedef typename gens<sizeof...(Iterators)>::type tuple_indices;

  private:
    // Workaround for the Visual Studio bug
    // http://stackoverflow.com/questions/23347287/parameter-pack-expansion-fails
    template<typename T>
    struct get_reference
    {
      typedef typename T::reference reference;
    };

  public:
    //typedef std::tuple<typename Iterators::reference...> references;
    typedef std::tuple<typename get_reference<Iterators>::reference...> references;

    template<typename T>
    empty inc(T& t)
    {
      ++t;
      return empty();
    }

    template<typename T>
    typename T::reference single_deref(T& t)
    {
      return *t;
    }

    template<int ...S>
    references multi_deref(seq<S...>)
    {
      return references(single_deref(std::get<S>(iterators))...);
    }

    references operator*()
    {
      return multi_deref(tuple_indices());
    }

    template<int ...S>
    void multi_inc(seq<S...>)
    {
      ignore(inc(std::get<S>(iterators))...);
    }

    zip_iterator& operator++()
    {
      multi_inc(tuple_indices());
      return *this;
    }

    bool operator==(const zip_iterator& that) const
    {
      return std::get<0>(iterators) == std::get<0>(that.iterators);
    }

    bool operator!=(const zip_iterator& that) const
    {
      return std::get<0>(iterators) != std::get<0>(that.iterators);
    }

    std::tuple<Iterators...> iterators;

  };

  template<typename...Iterators>
  zip_iterator<Iterators...> make_zip_iterator(Iterators ...its)
  {
    return zip_iterator<Iterators...>(its...);
  }

  template<typename...Ranges>
  struct zip_range
  {

    typedef std::tuple<typename unwrap<Ranges>::type...> unwrapped_ranges;
    typedef zip_iterator<typename unwrap_unref<Ranges>::type::iterator...> iterator;
    typedef typename gens<sizeof...(Ranges)>::type tuple_indices;

    template<typename... InRanges>
    zip_range(InRanges&&... rgs) : ranges(std::forward<InRanges>(rgs)...)
    {

    }
    template<typename T>
    typename T::iterator single_begin(T& t)
    {
      return t.begin();
    }

    template<int ...S>
    iterator multi_begin(seq<S...>)
    {
      return iterator(single_begin(std::get<S>(ranges))...);
    }

    iterator begin()
    {
      return multi_begin(tuple_indices());
    }

    template<typename T>
    typename T::iterator single_end(T& t)
    {
      return t.end();
    }

    template<int ...S>
    iterator multi_end(seq<S...>)
    {
      return iterator(single_end(std::get<S>(ranges))...);
    }

    iterator end()
    {
      return multi_end(tuple_indices());
    }

    unwrapped_ranges ranges;
  };

  template<typename...Ranges>
  zip_range<typename std::remove_reference<Ranges>::type...> make_zip_range(Ranges&&... rgs)
  {
    return zip_range<typename std::remove_reference<Ranges>::type...>(std::forward<Ranges>(rgs)...);
  }


  //Iterate over a range of ranges
  // TODO: this is not being used at teh moment, but should come in handy for 
  // distance weighted moving windows.
  /* 
  template<typename RangeRange>
  struct range_range_iterator
  {

    using range = typename RangeRange::value_type;
    using value_type = typename range::value_type;
    using range_iterator = typename range::iterator;
    using range_reference = typename range_iterator::reference;
    using iterators = std::vector<range_iterator>;
    struct reference_proxy
    {
      void operator=(const value_type& v) const
      {
        **m_iter = v;
      }

      void operator=(const reference_proxy& that) const
      {
        **m_iter = static_cast<value_type>(**that.m_iter);
      }


      // conversion to make the iterator readable
      operator value_type() const
      {
        return **m_iter;;
      }

      reference_proxy(const range_iterator* iter) :m_iter(iter)
      {}

      const range_iterator* m_iter;
    };

    using reference = std::vector<reference_proxy>;


    void find_begin(RangeRange& ranges)
    {
      m_iterators.clear();
      for (auto& range : ranges)
      {
        m_iterators.push_back(range.begin());
      }
    }

    void find_end(RangeRange& ranges)
    {
      m_iterators.clear();
      for (auto& range : ranges)
      {
        m_iterators.push_back(range.end());
      }
    }


    range_reference get(int i)
    {
      return *(m_iterators[i]);

    }
    reference operator*()
    {
      reference result;
      for (int i = 0; i < m_iterators.size(); ++i)
      {
        result.push_back(reference_proxy(&m_iterators[i]));
      }
      return result;
    }

    range_range_iterator& operator++()
    {
      for (int i = 0; i < m_iterators.size(); ++i)
      {
        ++(m_iterators[i]);
      }
      return *this;
    }

    bool operator==(const range_range_iterator& that)
    {
      return m_iterators.size() == 0 || m_iterators[0] == that.m_iterators[0];
    }

    bool operator!=(const range_range_iterator& that)
    {
      return m_iterators.size() != 0 && m_iterators[0] != that.m_iterators[0];
    }

    iterators m_iterators;

  };

  template<typename RangeRange>
  struct range_zip_range
  {
    range_zip_range(RangeRange& range_range) : m_range_range(range_range)
    {}

    using iterator = range_range_iterator<RangeRange>;

    iterator begin()
    {
      iterator i;
      i.find_begin(m_range_range);
      return i;
    }

    iterator end()
    {
      iterator i;
      i.find_end(m_range_range);
      return i;
    }

    RangeRange& m_range_range;

  };

  template<typename RangeRange>
  range_zip_range<RangeRange> make_range_zip_range(RangeRange& rr)
  {
    return range_zip_range<RangeRange>(rr);
  }

  */

} //namespace moving_window 

#endif