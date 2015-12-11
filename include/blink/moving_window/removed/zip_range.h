//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// Implements a zip range, it is probably not fully correct in providing all
// required range and iterator traits, but since these are still in flux
// anyway I am not bothering about those.

#ifndef BLINK_ITERATOR_ZIP_RANGE_H_AHZ
#define BLINK_ITERATOR_ZIP_RANGE_H_AHZ

#include <blink\utility\index_sequence.h>
#include <blink\iterator\zip_iterator.h>
#include <blink\iterator\detail.h>

#include <functional>
#include <tuple>
#include <utility>

namespace blink {
  namespace iterator {
    template<class...Ranges>
    class zip_range
    {
    private:
      using blink::utility::index_sequence; 
      using blink::utility::make_index_sequence;

      using unwrapped_ranges = std::tuple < unwrap<Ranges>... >;
      using tuple_indices = make_index_sequence < sizeof...(Ranges) > ;

    public:

      // use get_iterator instead of ::iterator to overcome Visual Studio bug
      using iterator = zip_iterator < get_type<get_iterator<unwrap_unref<Ranges> > >... > ;

      template<class... InRanges>
      explicit zip_range(InRanges&&... rgs) : ranges(std::forward<InRanges>(rgs)...)
      { }

      iterator begin()
      {
        return begin(tuple_indices());
      }

      iterator end()
      {
        return end(tuple_indices());
      }

    private:
      template<std::size_t ...S> iterator begin(index_sequence<S...>)
      {
        return iterator((std::get<S>(ranges).begin())...);
      }

      template<std::size_t ...S> iterator end(index_sequence<S...>)
      {
        return iterator(((std::get<S>(ranges).end())...);
      }

      unwrapped_ranges ranges;
    };

    template<class...Ranges>
    zip_range<get_type<std::remove_reference<Ranges> >...> make_zip_range(Ranges&&... rgs)
    {
      return zip_range<get_type<std::remove_reference<Ranges> >...>(std::forward<Ranges>(rgs)...);
    }
  }
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

//} //namespace moving_window 


// All of this was not necessary, because was already assignable
/*
template<class ValueTuple, class ReferenceTuple>
class zip_reference_proxy
{
using value_type = ValueTuple;
using reference = ReferenceTuple;
using tuple_indices = blink::utility::make_index_sequence<std::tuple_size<value_type>::value>;

template<std::size_t I>
using selected_value = get_type<std::tuple_element<I, value_type> >;

template<std::size_t I>
using selected_reference = get_type<std::tuple_element<I, reference> >;

public:
zip_reference_proxy(const reference& ref) : m_reference(ref)
{}

zip_reference_proxy(reference&& ref) : m_reference(ref)
{}
operator value_type() const
{
return value_type(get_value<S>()...);
}

template<std::size_t I> selected_reference<I> get()
{
return std::get<I>(m_reference);
}

template<class Tuple>
void operator=(const Tuple& v) const
{
do_nothing(set_values(tuple_indices(), v)...);
}

void operator=(const zip_reference_proxy& that) const
{
do_nothing(set_values(tuple_indices(), that.m_reference)...);
}
private:
template<std::size_t I> selected_value<I> get_value()
{
selected_value<I> value = std::get<I>(m_reference);
}

template<std::size_t I, class Tuple> void set_value(const Tuple& vs)
{
std::get<I>(m_reference) = std::get<I>(vs);
}
template<class Tuple, std::size_t...S> void set_values(blink::utility::index_sequence<S...>, const Tuple& vs)
{
do_nothing(set_value<S, Tuple>(vs));
}
reference m_reference;
};
*/
#endif