#pragma once
#include "defs_ogr.h"
#include "defs_boost.h"
#include "export_io_data.h"
#include "GDAL_OPENCV_IO.h"


namespace LxGeo
{

	namespace IO_DATA
	{

		using namespace LxGeo::GeometryFactoryShared;

		template <typename cv_mat_type>
		struct GeoImage;

		namespace ns_RProfileCompFlags
		{	enum RProfileCompFlags
			{
				width = 1 << 0,
				height = 1 << 1,
				count = 1 << 2,
				driver_name = 1 << 3,
				geotransform = 1 << 4,
				dtype = 1 << 5,
				crs_wkt = 1 << 6,
				gsd = 1 << 7,
				no_data = 1 << 8,
				ALL = width | height | count | driver_name | geotransform | dtype | crs_wkt | gsd | no_data
			};
			inline RProfileCompFlags operator|(RProfileCompFlags a, RProfileCompFlags b)
			{
				return static_cast<RProfileCompFlags>(static_cast<int>(a) | static_cast<int>(b));
			}
		}

		using namespace ns_RProfileCompFlags;

		struct RProfile {

			int width, height, count;
			std::string driver_name;
			double geotransform[6];
			GDALDataType dtype;
			std::string s_crs_wkt;
			std::optional<double> no_data;

			RProfile() {};

			RProfile(int _width, int _height, int _count, const double _geotransform[6],
				GDALDataType _dtype, std::string _s_crs_wkt ="", std::string _driver_name = "GTiff",
				std::optional<double> _no_data = std::optional<double>()) :
				width(_width), height(_height), count(_count), dtype(_dtype), driver_name(_driver_name), s_crs_wkt(_s_crs_wkt), no_data(_no_data){
				memcpy(geotransform, _geotransform, sizeof(double) * 6);
			}

			RProfile(const RProfile& ref_profile) {
				width = ref_profile.width;
				height = ref_profile.height;
				count = ref_profile.count;
				driver_name = ref_profile.driver_name;
				dtype = ref_profile.dtype;
				memcpy(geotransform, ref_profile.geotransform, sizeof(double) * 6);
				s_crs_wkt = std::string(ref_profile.s_crs_wkt);
				no_data = ref_profile.no_data;
			};

			~RProfile() {}

			// This should be fixed to take into account rotated rasters (where geotransform[2] !=0)
			double gsd() const { return abs(geotransform[1]); }

			std::shared_ptr<GDALDataset> to_gdal_dataset(std::string fp) {
				GDALDriver* driver = GetGDALDriverManager()->GetDriverByName(driver_name.c_str());
				GDALDataset* new_dataset = driver->Create(fp.c_str(), width, height, count, dtype, NULL);
				if (new_dataset == NULL) {
					auto exception_description = "Cannot create dataset at {}" + fp;
					throw std::exception(exception_description.c_str());
				}
				new_dataset->SetGeoTransform(geotransform);
				if (!s_crs_wkt.empty()) {
					OGRSpatialReference spatial_refrence;
					spatial_refrence.importFromWkt(const_cast<char*>(s_crs_wkt.c_str()));
					new_dataset->SetSpatialRef(&spatial_refrence);
				}
				if (no_data.has_value()) {
					for (size_t band_idx = 1; band_idx <= count; band_idx++)
						new_dataset->GetRasterBand(band_idx)->SetNoDataValue(no_data.value());
				}
				return std::shared_ptr<GDALDataset>(new_dataset, GDALClose);
			}

			static RProfile from_gdal_dataset(std::shared_ptr<GDALDataset> gdal_dataset) {
				std::string driver_name(gdal_dataset->GetDriverName());
				int width = gdal_dataset->GetRasterXSize();
				int height = gdal_dataset->GetRasterYSize();
				int band_count = gdal_dataset->GetRasterCount();
				std::string crs_wkt="";
				if (gdal_dataset->GetSpatialRef()) {
					char* temp_char;
					gdal_dataset->GetSpatialRef()->exportToWkt(&temp_char);
					crs_wkt = std::string(temp_char);
				}
				double geotransform[6];
				if (gdal_dataset->GetGeoTransform(geotransform) != CE_None){
					// temporary fix for the north facing rasters
					geotransform[5] = -1.0;
				}
				int raster_has_nodata;
				double nodata_value = gdal_dataset->GetRasterBand(1)->GetNoDataValue(&raster_has_nodata);
				std::optional<double> nodata; if (raster_has_nodata) nodata= nodata_value;
				GDALDataType raster_data_type = gdal_dataset->GetRasterBand(1)->GetRasterDataType();
				return RProfile(width, height, band_count, geotransform, raster_data_type, crs_wkt, driver_name, nodata);
			}

			static RProfile from_file(std::string file_path) {
				auto gdal_dataset = load_gdal_dataset_shared_ptr(file_path);
				return RProfile::from_gdal_dataset(gdal_dataset);
			}

			template <typename cv_mat_type>
			static RProfile from_geoimage(const GeoImage<cv_mat_type>& in_gimg) {
				RProfile out_rprofile = RProfile(in_gimg.image.cols, in_gimg.image.rows,
					in_gimg.image.channels(), in_gimg.geotransform,
					KGDAL2CV::opencv2gdal(in_gimg.image.type()), in_gimg.crs_wkt, "GTiff", in_gimg.no_data
					);
				return out_rprofile;
			}

			static RProfile from_extents(Boost_Box_2 extents, double gsd_x, double gsd_y, int count = 1, GDALDataType _dtype = GDALDataType::GDT_Byte, char* _crs_wkt = nullptr, std::string _driver_name = "GTiff") {
				int width = (extents.max_corner().get<0>() - extents.min_corner().get<0>()) / abs(gsd_x);
				int height = (extents.max_corner().get<1>() - extents.min_corner().get<1>()) / abs(gsd_y);
				double geotransform[6] = {
					(sign(gsd_x) == 1) ? extents.min_corner().get<0>() : extents.max_corner().get<0>(),
					gsd_x, 0.0,
					(sign(gsd_y) == 1) ? extents.min_corner().get<1>() : extents.max_corner().get<1>(),
					0.0, gsd_y }; // xoff, a, b, yoff, d, e
				return RProfile(width, height, count, geotransform, _dtype, _crs_wkt, _driver_name);
			}

			Boost_Box_2 to_box_extents() const {

				// This function is not absolute (may be changed to take into account rotated getransforms)
				double x1 = geotransform[0], x2 = x1 + geotransform[1] * width;
				double y1 = geotransform[3], y2 = y1 + geotransform[5] * height;

				double minx = std::min(x1, x2), maxx = std::max(x1, x2);
				double miny = std::min(y1,y2), maxy = std::max(y1,y2);
				return Boost_Box_2(Boost_Point_2(minx, miny), Boost_Point_2(maxx, maxy));
			}

			bool compare(const RProfile& target_profile, int flags = RProfileCompFlags::ALL, int except_flags = 0) {

				flags = flags & ~except_flags;
				// iterate through members || flags == 0 => all members
				if (flags & RProfileCompFlags::width) {
					if (target_profile.width != width) {
						std::cout << "profiles width doesn't match" << std::endl;
						return false;
					}
				}

				if (flags & RProfileCompFlags::height) {
					if (target_profile.height != height) {
						std::cout << "Profiles height doesn't match" << std::endl;
						return false;
					}
				}

				if (flags & RProfileCompFlags::count) {
					if (target_profile.count != count) {
						std::cout << "Profiles band count doesn't match" << std::endl;
						return false;
					}
				}

				if (flags & RProfileCompFlags::crs_wkt) {
					if (!s_crs_wkt.empty() || !target_profile.s_crs_wkt.empty()) {
						OGRSpatialReference spatial_refrence, t_spatial_refrence;
						spatial_refrence.importFromWkt(const_cast<char*>(s_crs_wkt.c_str()));
						char* temp_char = const_cast<char*>(s_crs_wkt.c_str());
						t_spatial_refrence.importFromWkt(&temp_char);
						if (!spatial_refrence.IsSame(&t_spatial_refrence)) {
							std::cout << "Profiles spatial refrence doesn't match" << std::endl;
							return false;
						}
					}
				}

				if (flags & RProfileCompFlags::driver_name) {
					if (target_profile.driver_name.compare(driver_name)) {
						std::cout << "Profiles drivers names doesn't match" << std::endl;
						return false;
					}
				}

				if (flags & RProfileCompFlags::dtype) {
					if (target_profile.dtype != dtype) {
						std::cout << "Profiles data types doesn't match" << std::endl;
						return false;
					}
				}

				if (flags & RProfileCompFlags::geotransform) {
					if (!std::equal(std::begin(target_profile.geotransform), std::end(target_profile.geotransform), std::begin(geotransform))) {
						std::cout << "Profiles geotransform doesn't match" << std::endl;
						return false;
					}
				}

				if (flags & RProfileCompFlags::gsd) {
					if (target_profile.gsd() != this->gsd()) {
						std::cout << "Profiles ground sampling distances doesn't match" << std::endl;
						return false;
					}
				}

				if (flags & RProfileCompFlags::no_data) {
					if (target_profile.no_data.has_value() != this->no_data.has_value() ||
						target_profile.no_data.value() != this->no_data.value()
						) {
						std::cout << "Profiles no_data values doesn't match" << std::endl;
						return false;
					}
				}

				// Might add almost equals for geotransform (where displacmenet between origins is lower than gsd
				return true;
			}

		};

	}
}