#pragma once
#include "defs.h"
#include "spatial_datasets/patchified_dataset.h"
#include "lightweight/raster_profile.h"
#include "lightweight/geoimage.h"
#include "spatial_coord_transformer.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		using namespace LxGeo::IO_DATA;

		class WPRasterDataset {

		public:

			WPRasterDataset() {};

			WPRasterDataset(std::string _raster_file_path, RProfile _raster_profile, WriteMode md=WriteMode::create):
				raster_file_path(_raster_file_path), raster_profile(_raster_profile){

				std::string error_message = "WPRasterDataset with mode 'create' for exisiting path !";
				bool c_error_bool = (md != WriteMode::create || !boost::filesystem::exists(raster_file_path));
				assert(c_error_bool && error_message.c_str());

				if (md == WriteMode::create | md == WriteMode::overwrite)
					raster_dataset = raster_profile.to_gdal_dataset(_raster_file_path);
				else
					raster_dataset = std::shared_ptr<GDALDataset>((GDALDataset*)GDALOpen(raster_file_path.c_str(), GA_Update), GDALClose);
			}

			template <typename cv_mat_type>
			void write_geoimage(const GeoImage<cv_mat_type>& gimg) {
				gimg.to_dataset(raster_dataset);
			}



		private:
			std::string raster_file_path;
			std::shared_ptr<GDALDataset> raster_dataset;
			RProfile raster_profile;
		};

	}
}