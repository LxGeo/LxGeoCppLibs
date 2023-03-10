#pragma once
#include "defs.h"



namespace LxGeo
{

	namespace IO_DATA
	{

		struct RProfile {

			int width, height, count;
			std::string driver_name;
			double geotransform[6];
			GDALDataType dtype;
			char* crs_wkt;

			RProfile() {};

			RProfile(int _width, int _height, int _count, const double _geotransform[6], GDALDataType _dtype, char* _crs_wkt=nullptr, std::string _driver_name = "GTiff") :
				width(_width), height(_height), count(_count), dtype(_dtype), crs_wkt(_crs_wkt), driver_name(_driver_name){
				memcpy(geotransform, _geotransform, sizeof(double) * 6);
			}

			// This should be fixed to take into account rotated rasters (where geotransform[2] !=0)
			double gsd() { return abs(geotransform[1]); }

			std::shared_ptr<GDALDataset> to_gdal_dataset(std::string fp) {
				GDALDriver* driver = GetGDALDriverManager()->GetDriverByName(driver_name.c_str());
				GDALDataset* new_dataset = driver->Create(fp.c_str(), width, height, count, dtype, NULL);
				if (new_dataset == NULL) {
					auto exception_description = "Cannot create dataset at {}" + fp;
					throw std::exception(exception_description.c_str());
				}
				new_dataset->SetGeoTransform(geotransform);
				if (crs_wkt) {
					OGRSpatialReference spatial_refrence;
					spatial_refrence.importFromWkt(&crs_wkt);
					new_dataset->SetSpatialRef(&spatial_refrence);
				}
				return std::shared_ptr<GDALDataset>(new_dataset, GDALClose);
			}

			static RProfile from_gdal_dataset(std::shared_ptr<GDALDataset> gdal_dataset) {
				int width = gdal_dataset->GetRasterXSize();
				int height = gdal_dataset->GetRasterYSize();
				int band_count = gdal_dataset->GetRasterCount();
				char* crs_wkt=NULL;
				if (gdal_dataset->GetSpatialRef()) {					
					gdal_dataset->GetSpatialRef()->exportToWkt(&crs_wkt);
				}
				else {
					crs_wkt = nullptr;
				}
				double geotransform[6];
				gdal_dataset->GetGeoTransform(geotransform);
				GDALDataType raster_data_type = gdal_dataset->GetRasterBand(1)->GetRasterDataType();
				return RProfile(width, height, band_count, geotransform, raster_data_type, crs_wkt);				
			}

			Boost_Box_2 to_box_extents() {
				double minx = geotransform[0], maxy = geotransform[3];
				double maxx = minx + geotransform[1] * width, miny = maxy + geotransform[5] * height;
				return Boost_Box_2(Boost_Point_2(minx, miny), Boost_Point_2(maxx, maxy));
			}

		};

	}
}