#pragma once
#include "defs.h"
#include "spatial_datasets/patchified_dataset.h"
#include "lightweight/raster_profile.h"
#include "lightweight/geoimage.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		template <typename cv_mat_type>
		class RPRasterDataset {

		public:

			// Constructor from filepath using predfined ContinuousPatchifiedDataset
			RPRasterDataset(std::string _raster_file_path, const ContinuousPatchifiedDataset& _cpd) : raster_file_path(_raster_file_path), cpd(_cpd) {
				raster_dataset = load_gdal_dataset_shared_ptr(raster_file_path);
				raster_profile = IO_DATA::RProfile::from_gdal_dataset(raster_dataset);

				pixel_patch_size = std::rint(cpd.patchified_dst_parameters.spatial_patch_size / raster_profile.gsd());
				pixel_patch_overlap = std::rint(cpd.patchified_dst_parameters.spatial_patch_overlap / raster_profile.gsd());
			}

			// Constructor from filepath and pixel patchified parameters
			RPRasterDataset(std::string _raster_file_path, int _pixel_patch_size, int _pixel_patch_overlap, int _pixel_pad_size=0, const Boost_Polygon_2& _boundary_geometry=Boost_Polygon_2()):
				raster_file_path(_raster_file_path),pixel_patch_size(_pixel_patch_size), pixel_patch_overlap(_pixel_patch_overlap), pixel_pad_size(_pixel_pad_size)
			{
				raster_dataset = load_gdal_dataset_shared_ptr(raster_file_path);
				RProfile temp_profile = IO_DATA::RProfile::from_gdal_dataset(raster_dataset);
				raster_profile = temp_profile;
				
				Boost_Polygon_2 correct_boundary_geometry;
				if (_boundary_geometry.outer().empty()) {
					bg::assign(correct_boundary_geometry, raster_profile.to_box_extents());
				}
				else {
					bg::assign(correct_boundary_geometry, _boundary_geometry);
				}

				double spatial_patch_size = double(pixel_patch_size) * raster_profile.gsd();
				double spatial_patch_overlap = double(pixel_patch_overlap) * raster_profile.gsd();
				double spatial_pad_size = double(pixel_pad_size) * raster_profile.gsd();
				cpd = ContinuousPatchifiedDataset({ spatial_patch_size, spatial_patch_overlap, spatial_pad_size, correct_boundary_geometry });
			}

			size_t length() {
				return cpd.length();
			}

			IO_DATA::RProfile get_rprofile() {
				return raster_profile;
			}

			IO_DATA::GeoImage<cv_mat_type> operator[](const int& offset) {
				auto& patch_box = cpd.grid_boxes[offset];
				OGREnvelope patch_envelope = transform_B2OGR_Envelope(patch_box);
				return IO_DATA::GeoImage<cv_mat_type>::from_file(raster_file_path, patch_envelope);
			}

		private:
			std::string raster_file_path;
			std::shared_ptr<GDALDataset> raster_dataset;
			IO_DATA::RProfile raster_profile;
			ContinuousPatchifiedDataset cpd;
			int pixel_patch_size, pixel_patch_overlap, pixel_pad_size;
		};

	}
}