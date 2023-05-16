#pragma once

#include "defs.h"
#include "defs_cgal.h"
#include "defs_common.h"
#include "export_shared.h"
#include "design_pattern/circular_range_wrapper.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		template <typename PointT>
		bool pts_collinear_2(const PointT& p0, const PointT& p1, const PointT& p2) {
			double x0 = PointTraits<PointT>::getX(p0);
			double y0 = PointTraits<PointT>::getY(p0);
			double x1 = PointTraits<PointT>::getX(p1);
			double y1 = PointTraits<PointT>::getY(p1);
			double x2 = PointTraits<PointT>::getX(p2);
			double y2 = PointTraits<PointT>::getY(p2);

			// Compute the cross product and check for collinearity
			double cross_product = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
			double magnitude_product = std::sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) *
				std::sqrt((x2 - x0) * (x2 - x0) + (y2 - y0) * (y2 - y0));
			const double epsilon = 1e-3;

			return std::abs(cross_product/ magnitude_product) < epsilon;
		}


		/**
			*  Function to simplify aberrant lines and repetitve points in polygon
			*  @param polygon_ring: a vector representing closed polygon ring points.
			*  @return a vector of simplified polygon 'closed'
			*/
		/*LX_GEO_FACTORY_SHARED_API std::vector<Inexact_Point_2> simplify_aberrant_polygon(const std::vector<Inexact_Point_2>& polygon_ring);*/

		/**
			*  Function to turn single part self-intersecting polygon into a list of non sel-intersecting polygons.
			*  @param polygon_ring: a vector representing closed polygon ring points.
			*  @return a list of exploded polygons parts as vector of points.
			*/
		LX_GEO_FACTORY_SHARED_API std::list<std::vector<Inexact_Point_2>> explose_self_intersecting_polygon(const std::vector<Inexact_Point_2>& polygon_ring);

		LX_GEO_FACTORY_SHARED_API std::list<Inexact_Polygon_2> explose_self_intersecting_polygon(const Inexact_Polygon_2& polygon_ring);

		
		template <typename Iterator, template <typename...> class Container = std::vector>
		Container<typename std::iterator_traits<Iterator>::value_type> simplify_aberrant_ring(Iterator begin, Iterator end) {

			using point_type = typename std::iterator_traits<Iterator>::value_type;
			Container<point_type> simplified_R; //simplified_R.reserve(in_ring.size());

			auto circ_iter = CopyCircularWrapper<std::list<point_type>>(begin, std::prev(end));
									
			auto c_view = circ_iter.getRangeIterators(circ_iter.begin(), 1, 1);

			auto turn_m2 = c_view[0];
			auto turn_m1 = c_view[1];
			auto turn_m0 = c_view[2];

			while (simplified_R.size() < circ_iter.size()) {
				if (!pts_collinear_2(*turn_m0, *turn_m1, *turn_m2)) {
					simplified_R.push_back(*turn_m1);
					turn_m2++;
					turn_m1++;
					turn_m0++;
				}
				else {
					turn_m1 = circ_iter.erase(turn_m1);
					turn_m1--;
					if (!simplified_R.empty()) simplified_R.pop_back();
					if (circ_iter.size() == 0)
						continue;
					turn_m0 = turn_m1; turn_m0++;
					turn_m2 = turn_m1; turn_m2--;
				}
			}
			if (simplified_R.size() > 0) simplified_R.push_back(simplified_R.front());
			return simplified_R;
			
			
		}

		template <typename polygonType>
		polygonType simplify_aberrant_polygon(const polygonType& polygon) {

			using PointType = typename PolygonTrait<polygonType>::PointType;

			std::vector<PointType> exterior_ring = PolygonTrait<polygonType>::getOuterRing(polygon);
			std::vector<PointType> simplified_exterior_ring = simplify_aberrant_ring(exterior_ring.begin(), exterior_ring.end());

			std::vector<std::vector<PointType>> simplified_interior_rings;
			for (size_t c_interior_idx = 0; c_interior_idx < PolygonTrait<polygonType>::interiorCount(polygon); c_interior_idx++) {
				std::vector<PointType> c_interior_ring = PolygonTrait<polygonType>::getInnerRing(polygon, c_interior_idx);
				simplified_interior_rings.push_back(simplify_aberrant_ring(c_interior_ring.begin(), c_interior_ring.end()));
			}

			polygonType simplified_polygon = PolygonTrait<polygonType>::create(simplified_exterior_ring, simplified_interior_rings);

			return simplified_polygon;
		}

		LX_GEO_FACTORY_SHARED_API double max_distance_between_lines(const Inexact_Point_2& p1,
			const Inexact_Point_2& p2,
			const Inexact_Point_2& p3,
			const Inexact_Point_2& p4);

		LX_GEO_FACTORY_SHARED_API OGRGeometry* BuildMultiLine(OGRGeometry* geometry);

	}
}