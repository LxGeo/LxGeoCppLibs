#include "io_raster.h"
#include "defs.h"
#include "GDAL_OPENCV_IO.h"


namespace LxGeo
{

	namespace IO_DATA
	{
		bool RasterIO::load_raster(std::string raster_path, GDALAccess read_mode, bool lazy_load) {
			raster_dataset = (GDALDataset*)GDALOpen(raster_path.c_str(), read_mode);
			if (raster_dataset == NULL)
			{
				BOOST_LOG_TRIVIAL(fatal) << fmt::format("Error loading raster at {} !", raster_path);
				return false;
			}
			raster_X_size = raster_dataset->GetRasterXSize();
			raster_Y_size = raster_dataset->GetRasterYSize();
			raster_size = raster_X_size * raster_Y_size;
			band_count = raster_dataset->GetRasterCount();
			spatial_refrence = raster_dataset->GetSpatialRef()->Clone();
			raster_dataset->GetGeoTransform(geotransform);
			raster_data_type = raster_dataset->GetRasterBand(1)->GetRasterDataType();

			if (!lazy_load) {
				try {
					raster_data = cv::imread(raster_path, cv::IMREAD_LOAD_GDAL);
				}
				catch (std::exception& e) {
					BOOST_LOG_TRIVIAL(debug) << e.what();
					BOOST_LOG_TRIVIAL(fatal) << "Fatal error while load raster data using IMREAD_LOAD_GDAL!";
					return false;
				}
			}

			return true;
		}

		void RasterIO::write_raster(std::string raster_path, bool force_write) {
			if (raster_dataset) {
				BOOST_LOG_TRIVIAL(fatal) << "Cannot write on an already existing dataset!";
				return;
			}

			raster_dataset = create_copy_dataset(raster_path);
			kgdal2cv->ImgWriteByGDAL(raster_dataset, raster_data,0,0);

		}

		GDALDataset* RasterIO::create_copy_dataset(std::string raster_path){
			GDALDriver* tiff_driver = GetGDALDriverManager()->GetDriverByName("GTiff");
			GDALDataType out_data_type = kgdal2cv->opencv2gdal(raster_data.type());
			GDALDataset* new_dataset = tiff_driver->Create(raster_path.c_str(), raster_X_size, raster_Y_size, band_count, out_data_type, NULL);
			new_dataset->SetSpatialRef(spatial_refrence);
			new_dataset->SetGeoTransform(geotransform);
			return new_dataset;
		}
	}
}