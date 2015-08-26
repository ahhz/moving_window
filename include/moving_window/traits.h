// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// traits are used to inspect properties of moving window based indicators
// TODO rename this file to something more specific

#ifndef TRAITS_H_AHZ
#define TRAITS_H_AHZ

#include <boost/none_t.hpp>

namespace moving_window {

  struct patch_element_tag{};
  struct pixel_element_tag{};
  struct edge_element_tag{};

  struct circle_tag{};
  struct square_tag{};



  template<typename Tag>
  struct indicator_traits
  {
    typedef boost::none_t element_type_tag;
    template<typename...Args>
    struct indicator{
      typedef int type;
      typedef int initializer;
      initializer make();
    };
  };
} // 

#endif