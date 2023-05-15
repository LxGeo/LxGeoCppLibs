#pragma once
#pragma warning( disable : 4251 )
#include <boost/filesystem.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include "export_shared.h"
#include <boost/polygon/polygon.hpp>
#include "defs_ogr.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		namespace bg = boost::geometry;
		namespace bgi = boost::geometry::index;
		typedef bg::model::point<double, 2, bg::cs::cartesian> Boost_Point_2;
		typedef bg::model::multi_point<Boost_Point_2> Boost_MultiPoint_2;
		typedef bg::model::box<Boost_Point_2> Boost_Box_2;
		typedef bg::model::segment<Boost_Point_2> Boost_Segment_2;
		typedef bg::model::linestring<Boost_Point_2> Boost_LineString_2;
		typedef bg::model::ring<Boost_Point_2> Boost_Ring_2;
		typedef bg::model::polygon<Boost_Point_2> Boost_Polygon_2;
		typedef std::pair<Boost_Box_2, size_t> Boost_Value_2;
		typedef std::pair<Boost_Point_2, size_t> Boost_Value_2_point;
		typedef bgi::rtree<Boost_Value_2, bgi::quadratic<16> > Boost_RTree_2;
		typedef bgi::rtree<Boost_Value_2, bgi::linear<16> > Boost_RTree_2_linear;
		typedef bgi::rtree<Boost_Value_2_point, bgi::linear<16> > Boost_RTree_2_linear_points;

		typedef bg::model::point<int, 2, bg::cs::cartesian> Boost_Discrete_Point_2;
		typedef bg::model::multi_point<Boost_Discrete_Point_2> Boost_Discrete_MultiPoint_2;
		typedef bg::model::box<Boost_Discrete_Point_2> Boost_Discrete_Box_2;
		typedef bg::model::segment<Boost_Discrete_Point_2> Boost_Discrete_Segment_2;
		typedef bg::model::linestring<Boost_Discrete_Point_2> Boost_Discrete_LineString_2;
		typedef bg::model::ring<Boost_Discrete_Point_2> Boost_Discrete_Ring_2;
		typedef bg::model::polygon<Boost_Discrete_Point_2> Boost_Discrete_Polygon_2;


		LX_GEO_FACTORY_SHARED_API double angle3p(const Boost_Point_2& p_m, const Boost_Point_2& p_0, const Boost_Point_2& p_1);

		LX_GEO_FACTORY_SHARED_API Boost_Box_2 box_buffer(Boost_Box_2& in_box, double buff);

		//LX_GEO_FACTORY_SHARED_API void boost_fill_ring(Boost_Ring_2& in_ring) {/*bg::append(in_ring, head); */ }

		template <typename ...Tail>
		void boost_fill_ring(Boost_Ring_2& in_ring, Boost_Point_2& head, Tail&&... tail) {
			//boost_fill_ring(in_ring, head);
			bg::append(in_ring, head);
			boost_fill_ring(in_ring, tail...);
		}

		template <typename ...Tail>
		void boost_fill_polygon(Boost_Polygon_2& in_poly, size_t ring_idx, Boost_Point_2& head, Tail&&... tail)
		{
			Boost_Ring_2* requiered_ring;
			if (ring_idx == 0)
				requiered_ring = &in_poly.outer();
			else {
				size_t inner_rings_size = in_poly.inners().size();
				if (inner_rings_size < ring_idx)
					in_poly.inners().resize(ring_idx);
				requiered_ring = &in_poly.inners()[ring_idx];
			}
			boost_fill_ring(*requiered_ring, head, tail...);
		}
	}
}