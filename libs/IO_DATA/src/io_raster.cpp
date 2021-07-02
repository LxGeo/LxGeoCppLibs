#include "io_raster.h"
#include "defs.h"


namespace LxGeo
{

	namespace IO_DATA
	{
		bool RasterIO::load_raster(std::string raster_path, GDALAccess read_mode = GA_ReadOnly, bool lazy_load = true) {
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
			if (band_count < 1)
			{
				BOOST_LOG_TRIVIAL(info) << fmt::format("Error band count is equal to zero for raster at {} !", raster_path);
				return false;
			}
			spatial_refrence = &OGRSpatialReference(*raster_dataset->GetSpatialRef());
			raster_data_type = raster_dataset->GetRasterBand(0)->GetRasterDataType();

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

		void write_raster(std::string raster_path, bool force_write) {

		}
	}
}