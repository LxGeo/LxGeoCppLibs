#pragma once
#include "defs.h"
#include "lightweight/geoimage.h"
#include "export_shared.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		using namespace IO_DATA;

		/*
		* Gdal proximity runs a gdal proximity on a geoImage and returns a geoimage
		*/
		LX_GEO_FACTORY_SHARED_API GeoImage<cv::Mat> proximity_raster(const GeoImage<cv::Mat>& input_image, size_t source_band_idx = 1, const std::list<double>& target_pixels = {},
			GDALDataType output_type = GDT_CFloat32, std::string distunits = "GEO", std::optional<double>maxdist = std::optional<double>(),
			std::optional<double> nodata = std::optional<double>(), std::optional<double> fixed_buf_val = std::optional<double>(), const std::list<std::string>& extra_options = {});



	}
}