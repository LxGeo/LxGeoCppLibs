#pragma once
#include "defs.h"
#include "export_io_data.h"
#include "GDAL_OPENCV_IO.h"
#include "coords.h"
#include "spatial_coord_transformer.h"
#include "geometries_with_attributes/geometries_with_attributes.h"
#include "lightweight/vector_profile.h"
#include "spatial_index/spatial_indexed_geometry_container.h"

namespace LxGeo
{

	namespace IO_DATA
	{
		/**
		A struct modeling a set of geometries with attributes.
		*/
		template <typename geom_type>
		struct GeoVector :public SpatialIndexedGeometryContainer<geom_type> {

			using Boost_Value = typename SpatialIndexedGeometryContainer<geom_type>::Boost_Value;
			using Boost_RTree = typename SpatialIndexedGeometryContainer<geom_type>::Boost_RTree;

			using SpatialIndexedGeometryContainer<geom_type>::rtree;
			using SpatialIndexedGeometryContainer<geom_type>::init_rtree;

			using ogr_geom_type = std::conditional_t<
				std::is_same_v<geom_type, Boost_Point_2>,
				OGRPoint, std::conditional_t<
				std::is_same_v<geom_type, Boost_LineString_2>,
				OGRLineString, OGRPolygon>>;

		public:
			inline static const std::string ID_FIELD_NAME = "__ID";

			GeoVector() {};

			GeoVector(const std::vector<Geometries_with_attributes<geom_type>>& _geometries_container) :SpatialIndexedGeometryContainer<geom_type>() {
				geometries_container.reserve(_geometries_container.size());
				std::copy(_geometries_container.begin(), _geometries_container.end(), std::back_inserter(geometries_container));
				init_rtree();
			}

			GeoVector(std::vector<Geometries_with_attributes<geom_type>>&& _geometries_container) :SpatialIndexedGeometryContainer<geom_type>() {
				geometries_container = std::move(_geometries_container);
				init_rtree();
			}

			GeoVector(const std::vector<geom_type>& _geometries_container) :SpatialIndexedGeometryContainer<geom_type>() {
				geometries_container.reserve(_geometries_container.size());
				for (const auto& c_geom : _geometries_container) {
					add_geometry(c_geom);
				}
				init_rtree();
			}

			GeoVector(GeoVector&& other) noexcept : SpatialIndexedGeometryContainer<geom_type>(std::move(other))  {
				geometries_container = std::move(other.geometries_container);				
			}

			GeoVector(const GeoVector& other) : SpatialIndexedGeometryContainer<geom_type>(other) {
				geometries_container = other.geometries_container;
			}

			GeoVector& operator=(GeoVector&& other) noexcept {
				if (this != &other) {
					geometries_container = std::move(other.geometries_container);
				}
				return *this;
			}

			size_t length() const override {
				return geometries_container.size();
			}

			geom_type& operator[](int offset) override { return geometries_container[offset].get_definition(); }

			const geom_type& operator[](int offset) const override { return geometries_container[offset].get_definition(); }

			Boost_Box_2 bounds() const {
				Boost_Box_2 bounds;
				if (!rtree.empty())
					bounds = rtree.bounds();
				else if (length()!=0) {
					bg::envelope(this->operator[](0), bounds);
					for (size_t c_idx = 0; c_idx < length(); c_idx++) {
						const auto& c_geom = this->operator[](c_idx);
						Boost_Box_2 c_geom_envelope; bg::envelope(c_geom, c_geom_envelope);
						merge_bounds(c_geom_envelope, bounds);
					}
				}
				return bounds;
			}

			void add_geometry(geom_type in_geometry) {
				geometries_container.push_back(Geometries_with_attributes<geom_type>(in_geometry));
				int c_idx = geometries_container.size() - 1;
				geometries_container[c_idx].set_int_attribute(ID_FIELD_NAME, c_idx);
			}

			void add_geometry(Geometries_with_attributes<geom_type> in_geometry_wa) {
				geometries_container.push_back(in_geometry_wa);
				// check if current index in tree else add Boost value to tree
			}

		public:

			template <typename envelope_type>
			GeoVector get_view_spatial(const envelope_type& spatial_envelope) const {
				Boost_Box_2 search_envelope;
				GeoVector out_geovector;
				if constexpr (std::is_same_v<envelope_type, OGREnvelope>) {
					search_envelope = Boost_Box_2({ spatial_envelope.MinX, spatial_envelope.MinY }, { spatial_envelope.MaxX, spatial_envelope.MaxY });
				}
				else if constexpr (std::is_same_v<envelope_type, Boost_Box_2>) {
					search_envelope = Boost_Box_2(spatial_envelope);
				}

				std::list<Boost_Value> candidates;
				rtree.query(bgi::intersects(search_envelope), std::back_inserter(candidates));

				for (auto& c_candidate : candidates) {
					size_t c_candidate_idx = c_candidate.second;
					const auto& c_candidate_geometry = geometries_container[c_candidate_idx];
					bool is_within = boost::geometry::intersects(search_envelope, c_candidate_geometry.get_definition());
					if (is_within)
						out_geovector.add_geometry(c_candidate_geometry);
				}
				out_geovector.init_rtree();

				return std::move(out_geovector);
			}

			GeoVector get_view_spatial(const double& xmin, const double& ymin, const double& xmax, const double& ymax) const {
				return get_view_spatial(Boost_Box_2({ xmin, ymin }, { xmax, ymax }));
			}


			template <typename envelope_type>
			static GeoVector from_file(const std::string& in_file, std::string layer_name = "", const envelope_type& _spatial_envelope = envelope_type(),
				const std::unordered_set<std::string>& included_fields = {}, const std::unordered_set<std::string>& excluded_fields = {}, const std::unordered_map<std::string, std::string>& field_rename = {}) {

				Boost_Polygon_2 spatial_envelope;
				if constexpr (std::is_same_v<envelope_type, Boost_Box_2>) {
					bg::assign(spatial_envelope, _spatial_envelope);
				}
				else if constexpr (std::is_same_v<envelope_type, OGREnvelope>) {
					auto env_polygon = envelopeToPolygon(_spatial_envelope);
					spatial_envelope = transform_OGR2B_Polygon(&env_polygon);
				}
				return GeoVector::from_file(in_file, layer_name, spatial_envelope, included_fields, excluded_fields, field_rename);
			}


			static GeoVector from_file(const std::string& in_file, std::string layer_name = "", const Boost_Polygon_2& spatial_filter = Boost_Polygon_2(),
				const std::unordered_set<std::string>& included_fields = {}, const std::unordered_set<std::string>& excluded_fields = {}, const std::unordered_map<std::string, std::string>& _field_rename = {}) {

				assert(included_fields.empty() | excluded_fields.empty() && "Both inculde and exclude fields are provided!");

				GeoVector loaded_gvec;
				auto vector_dataset = load_gdal_vector_dataset_shared_ptr(in_file);
				VProfile vector_dataset_profile = VProfile::from_gdal_dataset(vector_dataset);
				loaded_gvec.crs_wkt = vector_dataset_profile.s_crs_wkt;

				OGRLayer* to_load_layer;
				if (!layer_name.empty()) {
					to_load_layer = vector_dataset->GetLayerByName(layer_name.c_str());
					if (to_load_layer == NULL) {
						std::cout << "Layer with name " + layer_name + " is not found in dataset with path: " + in_file << std::endl;
						throw std::exception("Missing Layer");
					}
				}
				else {
					to_load_layer = vector_dataset->GetLayer(0);
					if (to_load_layer == NULL) {
						std::cout << "Layer at index 0 cannot be loaded from dataset with path: " + in_file << std::endl;
						throw std::exception("Missing Layer");
					}
				}

				// TODO: code uncesseray below as layer def is already loaded using VPROFILE
				LayerDef layer_definition = LayerDef::from_ogr_layer(to_load_layer);
				OGRPolygon spatial_filter_ogr = transform_B2OGR_Polygon(spatial_filter);
				if (!spatial_filter_ogr.IsEmpty())
					to_load_layer->SetSpatialFilter(&spatial_filter_ogr);


				auto layer_fields_names = layer_definition.get_fields_names();
				std::unordered_set<std::string> to_load_fields;
				if (!included_fields.empty()) {
					std::set_intersection(layer_fields_names.begin(), layer_fields_names.end(), included_fields.begin(), included_fields.begin(), std::inserter(to_load_fields, to_load_fields.begin()));
				}
				else if (!excluded_fields.empty()) {
					std::set_difference(layer_fields_names.begin(), layer_fields_names.end(), excluded_fields.begin(), excluded_fields.begin(), std::inserter(to_load_fields, to_load_fields.begin()));
				}
				else {
					to_load_fields = layer_fields_names;
				}
				auto field_rename = _field_rename;
				for (auto& field_name : to_load_fields) {
					if (field_rename.find(field_name) == field_rename.end())
						field_rename[field_name] = field_name;
				}

				loaded_gvec.geometries_container.reserve(to_load_layer->GetFeatureCount());
				//vector_layer->ResetReading();
				for (auto& c_feature : to_load_layer) {
					if (c_feature == NULL) continue;
					OGRGeometry* geom = c_feature->GetGeometryRef();
					if (ogr_geom_type* ogr_in_geometry = dynamic_cast<ogr_geom_type*>(geom))
					{
						if (!ogr_in_geometry->IsValid())
							continue;
						geom_type c_geometry = transform_OGR2B_geometry<ogr_geom_type,geom_type>(ogr_in_geometry);
						if (!spatial_filter.outer().empty() & !bg::intersects(c_geometry, spatial_filter))
							continue;
						Geometries_with_attributes<geom_type> c_geometry_wa(c_geometry);
						c_geometry_wa.set_int_attribute(ID_FIELD_NAME, c_feature->GetFID());

						for (auto&& c_field : *c_feature)
						{
							std::string c_field_name = c_field.GetName();

							if (to_load_fields.find(c_field_name) == to_load_fields.end())
								continue;

							std::string c_field_changed_name = field_rename[c_field_name];

							bool is_unset = c_field.IsUnset(), is_null = c_field.IsNull();
							if (is_null | is_unset)
								continue; // May add different action!

							switch (c_field.GetType())
							{
							case OFTInteger:
								c_geometry_wa.set_int_attribute(c_field_changed_name, c_field.GetInteger());
								break;
							case OFTInteger64:
								c_geometry_wa.set_int_attribute(c_field_changed_name, c_field.GetInteger64());
								break;
							case OFTReal:
								c_geometry_wa.set_double_attribute(c_field_changed_name, c_field.GetDouble());
								break;
							case OFTString:
								c_geometry_wa.set_string_attribute(c_field_changed_name, c_field.GetString());
								break;
							default:
								break;
							}
						}
						loaded_gvec.add_geometry(c_geometry_wa);
					}
				}
				loaded_gvec.geometries_container.shrink_to_fit();
				loaded_gvec.init_rtree();
				return loaded_gvec;
			}


			void to_dataset(
				GDALDataset* vector_dataset, WriteMode wm ,std::string out_layer_name = "",
				const std::function<bool(const Geometries_with_attributes<geom_type>&)>& filter_fn = [](const Geometries_with_attributes<geom_type>& _) {return true; }
			) const {

				OGRLayer* out_layer;
				if (out_layer_name.empty())
					out_layer = vector_dataset->GetLayer(0);
				else
					out_layer = vector_dataset->GetLayerByName(out_layer_name.c_str());

				for (size_t i = 0; i < geometries_container.size(); ++i) {
					const Geometries_with_attributes<geom_type>& gwa = geometries_container[i];
					if (filter_fn(gwa))
						save_geometry_wa_in_layer(gwa, wm, out_layer);
				}
				out_layer->SyncToDisk();

			}

			/**
			This will check if a feature with the same id already exists, else create a new one
			*/
			void save_geometry_wa_in_layer(const Geometries_with_attributes<geom_type>& gwa, WriteMode wm, OGRLayer* out_layer) const {
				OGRFeature* feature;
				bool new_feature_to_create=true;
				if (wm == WriteMode::update) {
					feature = out_layer->GetFeature(gwa.get_int_attribute(GeoVector<geom_type>::ID_FIELD_NAME));
					new_feature_to_create = (feature == NULL);
					if (new_feature_to_create)
						feature = OGRFeature::CreateFeature(out_layer->GetLayerDefn());
				}else{
					feature = OGRFeature::CreateFeature(out_layer->GetLayerDefn());
				}
				ogr_geom_type ogr_geometry = transform_B2OGR_geometry<geom_type, ogr_geom_type>(gwa.get_definition());
				feature->SetGeometry(&ogr_geometry);

				std::list<std::string> int_attributes, double_attributes, string_attributes;
				gwa.get_list_of_int_attributes(int_attributes);
				gwa.get_list_of_double_attributes(double_attributes);
				gwa.get_list_of_string_attributes(string_attributes);
				for (const std::string& name : int_attributes) {
					int x = gwa.get_int_attribute(name);
					feature->SetField(name.c_str(), x);
				}
				for (const std::string& name : double_attributes) {
					double x = gwa.get_double_attribute(name);
					feature->SetField(name.c_str(), x);
				}
				for (const std::string& name : string_attributes) {
					std::string x = gwa.get_string_attribute(name);
					feature->SetField(name.c_str(), x.c_str());
				}

				if (new_feature_to_create) {
					// Writes new feature
					OGRErr error = out_layer->CreateFeature(feature);
					if (error != OGRERR_NONE) std::cout << "Error code : " << int(error) << std::endl;
					OGRFeature::DestroyFeature(feature);
				}
				else {
					out_layer->SetFeature(feature);
				}

			}

			void to_file(const std::string& out_file, const OGRSpatialReference* enforce_spatial_refrence = nullptr) {
				if (geometries_container.empty()) {
					std::cout << "Empty Geovector! No file saved!" << std::endl;
					return;
				}
				std::string s_crs_wkt(crs_wkt);
				if (enforce_spatial_refrence) {
					char* pszWKT = NULL;
					enforce_spatial_refrence->exportToWkt(&pszWKT);
					s_crs_wkt = std::string(pszWKT);
				}
				std::string out_layer_name = "";
				VProfile out_profile = VProfile::from_geometries_wa<geom_type>({ { out_layer_name, geometries_container[0]} }, s_crs_wkt);
				auto vector_dataset = out_profile.to_gdal_dataset(out_file);
				OGRLayer* out_layer;
				if (out_layer_name.empty())
					out_layer = vector_dataset->GetLayer(0);
				else
					out_layer = vector_dataset->GetLayerByName(out_layer_name.c_str());

				for (size_t i = 0; i < geometries_container.size(); ++i) {
					Geometries_with_attributes<geom_type>& gwa = geometries_container[i];
					save_geometry_wa_in_layer(gwa, WriteMode::create, out_layer);
				}
				out_layer->SyncToDisk();

			};

			auto begin() {
				return geometries_container.begin();
			}

			auto end() {
				return geometries_container.end();
			}

			void apply_transform(std::function<geom_type(const geom_type&)> geom_transformer) {
				for (Geometries_with_attributes<geom_type>& c_gwa : geometries_container) {
					c_gwa.set_definition(geom_transformer(c_gwa.get_definition()));
				}
				init_rtree();
			}

		public:
			std::vector<Geometries_with_attributes<geom_type>> geometries_container;
			std::string crs_wkt;

		};

		template <typename in_geom_type, typename out_geom_type>
		void transform_geovector(const GeoVector<in_geom_type>& in_geovector, GeoVector<out_geom_type>& out_geovector,
			const std::function<Geometries_with_attributes<out_geom_type>(const Geometries_with_attributes<in_geom_type>&)>& transformer_fn) {
			for (const Geometries_with_attributes<in_geom_type>& gwa : in_geovector.geometries_container) {
				out_geovector.add_geometry(transformer_fn(gwa));
			}
			out_geovector.init_rtree();
		}

		template <typename in_geom_type, typename out_geom_type>
		void unpool_geovector(const GeoVector<in_geom_type>& in_geovector, GeoVector<out_geom_type>& out_geovector,
			const std::function<std::list<Geometries_with_attributes<out_geom_type>>(const Geometries_with_attributes<in_geom_type>&)>& transformer_fn) {
			for (const Geometries_with_attributes<in_geom_type>& gwa : in_geovector.geometries_container) {
				auto children_geoms = transformer_fn(gwa);
				for (const auto& child: children_geoms)
					out_geovector.add_geometry(child);
			}
			out_geovector.crs_wkt = in_geovector.crs_wkt;
			out_geovector.init_rtree();
		}

		template <typename in_geom_type, typename out_geom_type>
		void pool_geovector(const GeoVector<in_geom_type>& in_geovector, GeoVector<out_geom_type>& out_geovector,
			std::function<Geometries_with_attributes<out_geom_type> (std::list<Geometries_with_attributes<in_geom_type>>&)> transformer_fn) {
			
			// map of parent idx and value as a list of children
			std::map<size_t, std::list<Geometries_with_attributes<out_geom_type>>> hierarchy_map;

			for (const Geometries_with_attributes<in_geom_type>& gwa : in_geovector.geometries_container) {
				size_t pid = gwa.get_int_attribute("pid");
				if (hierarchy_map.find(pid) == hierarchy_map.end())
					hierarchy_map[pid] = { gwa };
				else
					hierarchy_map[pid].push_back(gwa);
			}
			for (auto& entry : hierarchy_map) {
				auto pooled_geom = transformer_fn(entry.second);
				pooled_geom.set_int_attribute(out_geovector.ID_FIELD_NAME, entry.first);
				out_geovector.add_geometry(pooled_geom);
			}
			out_geovector.crs_wkt = in_geovector.crs_wkt;
			out_geovector.init_rtree();
		}

	}
}