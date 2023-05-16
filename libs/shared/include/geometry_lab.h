#pragma once

#include "defs.h"
#include "defs_cgal.h"
#include "defs_common.h"
#include "export_shared.h"

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
		LX_GEO_FACTORY_SHARED_API std::vector<Inexact_Point_2> simplify_aberrant_polygon(const std::vector<Inexact_Point_2>& polygon_ring);

		/**
			*  Function to turn single part self-intersecting polygon into a list of non sel-intersecting polygons.
			*  @param polygon_ring: a vector representing closed polygon ring points.
			*  @return a list of exploded polygons parts as vector of points.
			*/
		LX_GEO_FACTORY_SHARED_API std::list<std::vector<Inexact_Point_2>> explose_self_intersecting_polygon(const std::vector<Inexact_Point_2>& polygon_ring);

		LX_GEO_FACTORY_SHARED_API std::list<Inexact_Polygon_2> explose_self_intersecting_polygon(const Inexact_Polygon_2& polygon_ring);

		

		LX_GEO_FACTORY_SHARED_API double max_distance_between_lines(const Inexact_Point_2& p1,
			const Inexact_Point_2& p2,
			const Inexact_Point_2& p3,
			const Inexact_Point_2& p4);

		LX_GEO_FACTORY_SHARED_API OGRGeometry* BuildMultiLine(OGRGeometry* geometry);

	}
}