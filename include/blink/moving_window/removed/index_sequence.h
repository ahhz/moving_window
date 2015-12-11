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

#ifndef INDEX_SEQUENCE_H_AHZ
#define INDEX_SEQUENCE_H_AHZ

#include <functional>
#include <tuple>
#include <utility>

namespace blink {
  namespace utility {

    template<std::size_t ...>
    struct index_sequence { };

    namespace detail {
      template<std::size_t N, std::size_t ...S>
      struct make_index_sequence_helper : make_index_sequence_helper < N - 1, N - 1, S... >
      { };

      template<std::size_t ...S>
      struct make_index_sequence_helper < 0, S... >
      {
        using type = index_sequence < S... > ;
      };
    }

    template<std::size_t N>
    using make_index_sequence = typename detail::make_index_sequence_helper<N>::type;
  }
}
#endif