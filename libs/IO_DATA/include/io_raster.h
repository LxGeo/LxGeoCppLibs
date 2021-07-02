#pragma once
#include "defs.h"

namespace LxGeo
{

	namespace IO_DATA
	{
		class RasterIO
		{
		public:

			RasterIO() {
				origin_point = Boost_Point_2(0, 0);
				bounding_box = Boost_Box_2();
				m_pixelSize = 0;
				raster_size = 0;
				band_count = 0;
				spatial_refrence = NULL;
				raster_data_type = GDT_Unknown;
				raster_data = matrix();
			};

			~RasterIO() {
				if (raster_dataset)
					GDALClose((GDALDatasetH)raster_dataset);
			};

			bool load_raster(std::string raster_path, GDALAccess read_mode= GA_ReadOnly, bool lazy_load = true);

			void write_raster(std::string raster_path, bool force_write);


		public:
			Boost_Point_2 origin_point;
			Boost_Box_2 bounding_box;
			double m_pixelSize;
			size_t raster_X_size;
			size_t raster_Y_size;
			size_t raster_size;
			size_t band_count;
			GDALDataset* raster_dataset;
			OGRSpatialReference* spatial_refrence;
			GDALDataType raster_data_type;
			matrix raster_data;
		};
		
	}
}