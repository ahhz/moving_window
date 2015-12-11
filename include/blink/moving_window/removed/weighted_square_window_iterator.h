//
//=======================================================================
// Copyright 2015
// Author: Alex Hagen-Zanker
// University of Surrey
//
// Distributed under the MIT Licence (http://opensource.org/licenses/MIT)
//=======================================================================
//
// This file provides the weighted square window. It is not tested and  probably 
// there is little point maintaining this until the rest of the library is more 
// stable in its interface.

#ifndef WEIGTHED_SQUARE_WINDOW_ITERATOR_H_AHZ
#define WEIGHTED_SQUARE_WINDOW_ITERATOR_H_AHZ

#include <moving_window/square_window_iterator.h>

#include <boost/accumulators/accumulators.hpp> //boost::accumulators::subtotal

#include <utility> // pair
#include <vector>


template<typename VisitorIterator, typename AccumulatorSet, typename Weight = double, typename Radius = int>
class weighted_square_window_iterator
{
public:
	typedef typename VisitorIterator::coordinate_type coordinate_type;
	typedef typename VisitorIterator::index_type index_type;
	
	typedef square_window_iterator<VisitorIterator, AccumulatorSet> square_window_iterator;
	typedef weighted_square_window_iterator<VisitorIterator, AccumulatorSet, Weight, Radius> this_type;
	
	weighted_square_window_iterator()
	{}

	weighted_square_window_iterator(const std::vector<std::pair<Weight, Radius> >& weights, VisitorIterator visitor_begin, VisitorIterator visitor_end)
	{
		m_Iterators.clear();

		m_Weights = weights;
		auto j = m_Weights.begin();
		const auto j_end = m_Weights.end();
		for(; j != j_end; ++j) {
			square_window_iterator sw(j->second, visitor_begin, visitor_end);
			m_Iterators.push_back(sw); 
			m_Iterators.back().find_begin(); // not sure if this is necessary
		}
	}
	
	weighted_square_window_iterator(const this_type& that)
	{
		*this = that;
	}
	/*
	void operator=(const this_type& that)
	{
		m_Weights = that.m_Weights;
		m_Iterators = that.m_Iterators;
	}
	*/
	void find_end()
	{
		auto i = m_Iterators.begin();
		const auto end = m_Iterators.end();
		for(; i != end; ++i) {
			i->find_end();
		}
	}

	void find_begin()
	{
		auto i = m_Iterators.begin();
		const auto end = m_Iterators.end();
		for(; i != end; ++i) {
			i->find_begin();
		}
	}

	bool operator==(const this_type& that) const
	{
		return get_coordinates() == that.get_coordinates()
			&& m_Weights == that.m_Weights;
	}

	bool operator!=(const this_type& that) const
	{
		return !(*this == that);
	}

	this_type& operator++() // prefix, don't supply postfix
	{
		// Increment each iterator
		auto i = m_Iterators.begin();
		const auto end = m_Iterators.end();
		for(; i != end; ++i) {
			++(*i);
		}
		return *this;
	}

	const coordinate_type& get_coordinates() const
	{
		return m_Iterators.front().get_coordinates();
	}

	AccumulatorSet get_accumulator() const 
	{
		AccumulatorSet acc = AccumulatorSet();
		auto i = m_Iterators.begin();
		auto j = m_Weights.begin();
		const auto end = m_Iterators.end();
		for(; i != end; ++i, ++j) {
			acc.add_subtotal(
				boost::accumulators::subtotal = i->get_accumulator() ,
				boost::accumulators::weight = j->first );
		}

		return acc;
	}
	
	std::vector<std::pair<Weight,Radius> > m_Weights;
	std::vector<square_window_iterator> m_Iterators;
};
#endif