#pragma once

#include "defs.h"
#include "defs_cgal.h"
#include "defs_common.h"
#include "export_shared.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{
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

		LX_GEO_FACTORY_SHARED_API bool pts_collinear_2(Inexact_Point_2 p1, Inexact_Point_2 p2, Inexact_Point_2 p3);

		LX_GEO_FACTORY_SHARED_API double max_distance_between_lines(const Inexact_Point_2& p1,
			const Inexact_Point_2& p2,
			const Inexact_Point_2& p3,
			const Inexact_Point_2& p4);

		LX_GEO_FACTORY_SHARED_API OGRGeometry* BuildMultiLine(OGRGeometry* geometry);

	}
}