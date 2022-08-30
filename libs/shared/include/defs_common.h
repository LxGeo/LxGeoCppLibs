#pragma once
#include <ogrsf_frmts.h>
#include "defs_boost.h"
#include "defs_cgal.h"
#include "export_shared.h"

namespace LxGeo
{

	namespace GeometryFactoryShared
	{

		LX_GEO_FACTORY_SHARED_API Inexact_Point_2 transform_B2C_Point(const Boost_Point_2& boost_point);

		LX_GEO_FACTORY_SHARED_API Boost_Point_2 transform_C2B_Point(Inexact_Point_2& cgal_point);
		
		LX_GEO_FACTORY_SHARED_API void container_transform_B2C_Points(std::vector<Boost_Point_2>& input_container, std::vector<Inexact_Point_2>& output_container);

		LX_GEO_FACTORY_SHARED_API void container_transform_OGRRING2vector_Points(OGRLinearRing* input_ring, std::vector<Inexact_Point_2>& output_vector);

		LX_GEO_FACTORY_SHARED_API std::vector<Inexact_Point_2> container_transform_OGRRING2vector_Points(OGRLinearRing* input_ring);

		LX_GEO_FACTORY_SHARED_API OGRLinearRing container_transform_vector_Points2OGRRING(const std::vector<Inexact_Point_2>& input_vector);

		LX_GEO_FACTORY_SHARED_API OGRPoint transform_B2OGR_Point(Boost_Point_2& in_point);

		LX_GEO_FACTORY_SHARED_API OGRLinearRing transform_B2OGR_Ring(Boost_Ring_2& in_ring);

		LX_GEO_FACTORY_SHARED_API Boost_Polygon_2 transform_OGR2B_Polygon(OGRPolygon* ogr_polygon);

		LX_GEO_FACTORY_SHARED_API OGRPolygon transform_B2OGR_Polygon(Boost_Polygon_2& in_polygon);

		LX_GEO_FACTORY_SHARED_API OGRLineString transform_B2OGR_LineString(Boost_LineString_2& in_linestring);
		
		LX_GEO_FACTORY_SHARED_API OGREnvelope transform_B2OGR_Envelope(Boost_Box_2& in_envelope);


	}
}