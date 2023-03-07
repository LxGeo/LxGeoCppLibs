#pragma once
#include "defs.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		LX_GEO_FACTORY_SHARED_API void rasterize_shapefile(const std::string& out_raster_path, const std::string& input_shapefile_path,
			OGREnvelope* raster_extents, double raster_px_size, double raster_py_size, const std::list<std::string>& extra_options,
			double* nodata_value = nullptr, const std::string& output_type = "Byte");



	}
}