#pragma once
#include "defs.h"
#include "spatial_datasets/patchified_dataset.h"
#include "lightweight/vector_profile.h"
#include "lightweight/geovector.h"
#include "spatial_coord_transformer.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		using namespace LxGeo::IO_DATA;
				

		class WPVectorDataset {

		public:

			WPVectorDataset() {};

			WPVectorDataset(std::string _vector_file_path, VProfile _vector_profile, WriteMode _wm = WriteMode::create) :
				vector_file_path(_vector_file_path), vector_profile(_vector_profile), wm(_wm){

				std::string error_message = "WPRasterDataset with mode 'create' for exisiting path !";
				bool c_error_bool = (wm != WriteMode::create || !boost::filesystem::exists(vector_file_path));
				assert(c_error_bool && error_message.c_str());

				if (wm == WriteMode::create || wm == WriteMode::overwrite) {
					auto out_dirname = boost::filesystem::path(_vector_file_path).parent_path();
					if (!boost::filesystem::exists(out_dirname))
						boost::filesystem::create_directories(out_dirname);
					vector_dataset = vector_profile.to_gdal_dataset(vector_file_path);
				}
				else {
					vector_dataset = load_gdal_vector_dataset_shared_ptr(vector_file_path, GDAL_OF_UPDATE);
					vector_profile.update_gdal_dataset(vector_dataset);
				}
			}

			template <typename geom_type>
			void write_geovector(const GeoVector<geom_type>& gvec) {
				auto already_saved_filter = [this](const Geometries_with_attributes<geom_type>& gwa)->bool {
					size_t c_geom_id = gwa.get_int_attribute(GeoVector<geom_type>::ID_FIELD_NAME);
					return (saved_ids.find(c_geom_id) == saved_ids.end());
				};
				gvec.to_dataset(vector_dataset.get(), wm, "", already_saved_filter);
				std::transform(gvec.geometries_container.begin(),
					gvec.geometries_container.end(),
					std::inserter(saved_ids, saved_ids.begin()),
					[](const auto& gwa) {return gwa.get_int_attribute(GeoVector<geom_type>::ID_FIELD_NAME); });
			}


		private:
			std::string vector_file_path;
			WriteMode wm;
			std::shared_ptr<GDALDataset> vector_dataset;
			VProfile vector_profile;
			std::set<size_t> saved_ids;
		};

	}
}