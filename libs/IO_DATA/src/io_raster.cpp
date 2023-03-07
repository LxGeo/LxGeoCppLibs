#include "io_raster.h"
#include "defs.h"
#include "GDAL_OPENCV_IO.h"


namespace LxGeo
{

	namespace IO_DATA
	{

		bool RasterIO::load_raster(std::string _raster_path, GDALAccess read_mode, bool lazy_load) {

			raster_path = _raster_path;
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
			if (raster_dataset->GetSpatialRef()) spatial_refrence = raster_dataset->GetSpatialRef()->Clone();
			raster_dataset->GetGeoTransform(geotransform);
			raster_data_type = raster_dataset->GetRasterBand(1)->GetRasterDataType();
			// loading extents
			double minx = geotransform[0], maxy = geotransform[3];
			double maxx = minx + geotransform[1] * raster_X_size, miny = maxy + geotransform[5] * raster_Y_size;
			bounding_box = Boost_Box_2(Boost_Point_2(minx, miny), Boost_Point_2(maxx, maxy));

			if (!lazy_load) {
				KGDAL2CV kgdal2cv;
				try {
					//raster_data = cv::imread(raster_path, cv::IMREAD_LOAD_GDAL );
					raster_data = kgdal2cv.ImgReadByGDAL(raster_path);
					if (raster_data.empty()) {
						throw std::runtime_error("Empty raster data!");
					}
					pixel_extent = cv::Rect(cv::Point(), raster_data.size());
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

			raster_dataset = create_copy_dataset(raster_path, kgdal2cv.opencv2gdal(raster_data.type()), band_count);
			kgdal2cv.ImgWriteByGDAL(raster_dataset, raster_data,0,0);

		}

		GDALDataset* RasterIO::create_copy_dataset(std::string raster_path, GDALDataType _out_data_type, size_t _band_count){
			GDALDriver* tiff_driver = GetGDALDriverManager()->GetDriverByName("GTiff");
			GDALDataType out_data_type; 
			if (_out_data_type) out_data_type = _out_data_type;
			else out_data_type = kgdal2cv.opencv2gdal(raster_data.type());
			
			size_t out_band_count;
			if (_band_count) out_band_count = _band_count;
			else out_band_count = band_count;
			
			GDALDataset* new_dataset = tiff_driver->Create(raster_path.c_str(), raster_X_size, raster_Y_size, out_band_count, out_data_type, NULL);
			if (new_dataset == NULL) { throw std::exception(fmt::format("Cannot create copy dataset at {}", raster_path).c_str()); }
			if (spatial_refrence) new_dataset->SetSpatialRef(spatial_refrence);
			if (geotransform) new_dataset->SetGeoTransform(geotransform);
			return new_dataset;
		}

		static GDALDataset* create_dataset(std::string& dataset_path, GDALDriver* gdal_driver, GDALDataType gdal_datatype, size_t raster_X_size, size_t raster_Y_size,
			double geotransform[6], size_t band_count, const OGRSpatialReference* srs, char** papzoptions) {
			GDALDataset* new_dataset = gdal_driver->Create(dataset_path.c_str(), raster_X_size, raster_Y_size, band_count, gdal_datatype, papzoptions);
			if (new_dataset == NULL) { throw std::exception("Cannot create copy dataset!"); }
			if (srs) new_dataset->SetSpatialRef(srs);
			if (geotransform) new_dataset->SetGeoTransform(geotransform);
			return new_dataset;
		}

		static GDALDataset* create_dataset(std::string& dataset_path, GDALDriver* gdal_driver, GDALDataType gdal_datatype,
			OGREnvelope raster_extents, double pixel_x_size, double pixel_y_size, size_t band_count, const OGRSpatialReference* srs, char** papzoptions) {

			// transform enveleop and resolution to geotransform and raster_(X||Y)_size
			size_t raster_X_size = static_cast<size_t>((raster_extents.MaxX - raster_extents.MinX) / pixel_x_size);
			size_t raster_Y_size = static_cast<size_t>((raster_extents.MaxY - raster_extents.MinY) / pixel_y_size);
			double geotransform[6] = {0, 0, 0, 0, 0, 0};
			geotransform[0] = raster_extents.MinX; geotransform[3] = raster_extents.MaxY;
			geotransform[1] = pixel_x_size; geotransform[5] = pixel_y_size;

			return create_dataset(dataset_path, gdal_driver, gdal_datatype, raster_X_size, raster_Y_size, geotransform, band_count, srs, papzoptions);

		}

	}
}