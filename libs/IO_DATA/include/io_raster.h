#pragma once
#include "defs.h"
#include "export_io_data.h"
#include "GDAL_OPENCV_IO.h"

namespace LxGeo
{

	namespace IO_DATA
	{
		class RasterIO
		{

		public:

			IO_DATA_API RasterIO() {
				bounding_box = Boost_Box_2();
				m_pixelSize = 0;
				raster_size = 0;
				band_count = 0;
				spatial_refrence = NULL;
				//geotransform = { 0,0,0,0,0,0 };
				raster_data_type = GDT_Unknown;
				raster_data = matrix();
				raster_dataset = NULL;
			};

			IO_DATA_API RasterIO(RasterIO& copy_raster) {
				bounding_box = Boost_Box_2(copy_raster.bounding_box);
				m_pixelSize = copy_raster.m_pixelSize;
				raster_X_size = copy_raster.raster_X_size;
				raster_Y_size = copy_raster.raster_Y_size;
				raster_size = copy_raster.raster_size;
				band_count = copy_raster.band_count;
				if (copy_raster.spatial_refrence)
					spatial_refrence = copy_raster.spatial_refrence->Clone();
				else
					spatial_refrence = nullptr;
				for (size_t cpy_idx = 0; cpy_idx < 6; ++cpy_idx) geotransform[cpy_idx] = copy_raster.geotransform[cpy_idx];
				raster_data_type = copy_raster.raster_data_type;
				raster_data = matrix(copy_raster.raster_data);
				raster_dataset = NULL;
			}

			IO_DATA_API RasterIO(RasterIO& copy_raster, matrix& copy_raster_data): RasterIO(copy_raster){
				if (copy_raster_data.cols == raster_X_size && copy_raster_data.rows == raster_Y_size)
					raster_data = copy_raster_data;
				else
					throw std::runtime_error("RasterIO creation failed! matrix and raster size are different!");
			}

			IO_DATA_API void close() {
				if (raster_dataset)
					GDALClose((GDALDatasetH)raster_dataset);
			}

			IO_DATA_API ~RasterIO() {
				close();
			};

			IO_DATA_API double get_pixel_width() {
				return geotransform[1];
			}

			IO_DATA_API double get_pixel_height() {
				return geotransform[5];
			}

			IO_DATA_API Boost_Point_2 get_origin_point() {
				return Boost_Point_2(geotransform[0], geotransform[3]);
			}

			IO_DATA_API bool load_raster(std::string raster_path, GDALAccess read_mode= GA_ReadOnly, bool lazy_load = true);

			IO_DATA_API void write_raster(std::string raster_path, bool force_write);

			GDALDataset* create_copy_dataset(std::string raster_path);


		public:
			Boost_Box_2 bounding_box;
			double m_pixelSize;
			size_t raster_X_size;
			size_t raster_Y_size;
			size_t raster_size;
			size_t band_count;
			GDALDataset* raster_dataset;
			OGRSpatialReference* spatial_refrence;
			double geotransform[6];
			GDALDataType raster_data_type;
			matrix raster_data;
		};
		
	}
}