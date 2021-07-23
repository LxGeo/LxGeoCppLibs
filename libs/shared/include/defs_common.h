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
	}
}