#pragma once
#include "defs.h"
#include "lightweight/geoimage.h"
#include "geometries_with_attributes/geometries_with_attributes.h"
#include "export_shared.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		enum class RasterPixelsStitcherStartegy {
			vertex_only = 1 << 0,
			vertex_and_mid_point = 1 << 1,
			constant_walker = 1 << 2,
			contours = 1 << 3,
			filled_polygon = 1 << 4
		};

		using namespace LxGeo::IO_DATA;

		class RasterPixelsStitcher {

		public:
			RasterPixelsStitcher() {};
			RasterPixelsStitcher(GeoImage<cv::Mat>& _ref_gimg) { ref_gimg = _ref_gimg; };

			LX_GEO_FACTORY_SHARED_API double readPolygonPixels(Boost_Polygon_2& resp_polygon, RasterPixelsStitcherStartegy strategy);

		public:
			GeoImage<cv::Mat> ref_gimg;

		};
	}
}