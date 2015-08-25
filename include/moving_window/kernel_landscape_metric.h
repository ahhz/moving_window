#pragma once

namespace kernel_type_tag
{
	struct square{};
	struct circle{};
}
namespace element_type_tag
{
	struct edge{};
	struct patch{};
	struct pixel{};
}



template<typename Kernel, typename Accumulator, typename Raster, typename WeightRaster>
struct kernel_landscape_metric
{
	struct unsupported{};

	typedef Accumulator::element_type element_type;
	typedef typename boost::is_same<element_type, element_type_tag::edge>::type is_edge;
	typedef typename boost::is_same<element_type, element_type_tag::patch>::type is_patch;
	typedef typename boost::is_same<element_type, element_type_tag::pixel>::type is_pixel;
	
	typedef typename boost::is_same<Kernel, kernel_type_tag::square>::type is_square;
	typedef typename boost::is_same<Kernel, kernel_type_tag::circle>::type is_circle;
	
	typedef boost::if_<is_edge
		, boost::if_<is_square
			, square_window_edge_iterator<Accumulator, Raster, WeightRaster>
			, unsupported >::type
		,  boost::if_<is_square
			, square_window_iterator<Accumulator, Raster, WeightRaster>
			, non_square_window_iterator<Accumulator, Raster, WeightRaster>>::type
			>::type window_iterator;

	 

}