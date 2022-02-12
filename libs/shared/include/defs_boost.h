#pragma once
#pragma warning( disable : 4251 )
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include "export_shared.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		namespace bg = boost::geometry;
		namespace bgi = boost::geometry::index;
		typedef bg::model::point<double, 2, bg::cs::cartesian> Boost_Point_2;
		typedef bg::model::box<Boost_Point_2> Boost_Box_2;
		typedef std::pair<Boost_Box_2, size_t> Boost_Value_2;
		typedef std::pair<Boost_Point_2, size_t> Boost_Value_2_point;
		typedef bgi::rtree<Boost_Value_2, bgi::quadratic<16> > Boost_RTree_2;
		typedef bgi::rtree<Boost_Value_2, bgi::linear<16> > Boost_RTree_2_linear;
		typedef bg::model::segment<Boost_Point_2> Boost_Segment_2;
		typedef bg::model::linestring<Boost_Point_2> Boost_LineString_2;
		typedef bg::model::ring<Boost_Point_2> Boost_Ring_2;
		typedef bg::model::polygon<Boost_Point_2> Boost_Polygon_2;

		LX_GEO_FACTORY_SHARED_API double angle3p(const Boost_Point_2& p_m, const Boost_Point_2& p_0, const Boost_Point_2& p_1);
	}
}