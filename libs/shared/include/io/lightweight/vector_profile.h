#pragma once
#include "defs.h"



namespace LxGeo
{

	namespace IO_DATA
	{

		struct LayerDef {
			std::list<std::string> int_attributes_names;
			std::list<std::string> double_attributes_names;
			std::list<std::string> string_attributes_names;
			OGRwkbGeometryType wkb_type;
			Boost_Box_2 layer_extent;

			LayerDef():wkb_type(wkbUnknown){}
			LayerDef(const LayerDef& in_layer_attr_def): wkb_type(in_layer_attr_def.wkb_type), layer_extent(in_layer_attr_def.layer_extent){
				std::copy(in_layer_attr_def.int_attributes_names.begin(), in_layer_attr_def.int_attributes_names.end(), std::back_inserter(int_attributes_names));
				std::copy(in_layer_attr_def.double_attributes_names.begin(), in_layer_attr_def.double_attributes_names.end(), std::back_inserter(double_attributes_names));
				std::copy(in_layer_attr_def.string_attributes_names.begin(), in_layer_attr_def.string_attributes_names.end(), std::back_inserter(string_attributes_names));
			}

			std::unordered_set<std::string> get_fields_names() const {
				std::unordered_set<std::string> output_fields_names;
				for (auto& l : { int_attributes_names, double_attributes_names, string_attributes_names }) {
					output_fields_names.insert(l.begin(), l.end());
				}
				return output_fields_names;
			}

			static LayerDef from_ogr_layer(OGRLayer* in_layer){
				LayerDef c_layer_def;
				c_layer_def.wkb_type = in_layer->GetGeomType();
				OGREnvelope temp_ogr_extents; in_layer->GetExtent(&temp_ogr_extents);
				c_layer_def.layer_extent = transform_OGR2B_Envelope(temp_ogr_extents);
				OGRFeatureDefn* c_layer_defintion = in_layer->GetLayerDefn();
				for (int field_idx = 0; field_idx < c_layer_defintion->GetFieldCount(); field_idx++) {
					OGRFieldDefn* c_field_defintion = c_layer_defintion->GetFieldDefn(field_idx);
					switch (c_field_defintion->GetType())
					{
					case OFTInteger:
						c_layer_def.int_attributes_names.push_back(c_field_defintion->GetNameRef());
						break;
					case OFTInteger64:
						c_layer_def.int_attributes_names.push_back(c_field_defintion->GetNameRef());
						break;
					case OFTReal:
						c_layer_def.double_attributes_names.push_back(c_field_defintion->GetNameRef());
						break;
					case OFTString:
						c_layer_def.string_attributes_names.push_back(c_field_defintion->GetNameRef());
						break;
					default:
						std::cout << "Only Integer, double and string is supported for layer schema definition" << std::endl;
						break;
					}
				}
				return c_layer_def;
			}

			template <typename geom_type>
			static LayerDef from_geometry_wa(const Geometries_with_attributes<geom_type>& g_wa) {
				LayerDef out_layer_def;
				if constexpr (std::is_same<geom_type, Boost_Point_2>::value)
					out_layer_def.wkb_type = wkbPoint;
				else if constexpr (std::is_same<geom_type, Boost_LineString_2>::value)
					out_layer_def.wkb_type = wkbLineString;
				else if constexpr (std::is_same<geom_type, Boost_Polygon_2>::value)
					out_layer_def.wkb_type = wkbPolygon;
				else
					out_layer_def.wkb_type = wkbUnknown;				
				g_wa.get_list_of_int_attributes(out_layer_def.int_attributes_names);
				g_wa.get_list_of_double_attributes(out_layer_def.double_attributes_names);
				g_wa.get_list_of_string_attributes(out_layer_def.string_attributes_names);
				return out_layer_def;
			}

		};

		struct VProfile {

			std::string driver_name;
			std::string s_crs_wkt;
			std::map<std::string, LayerDef> layers_def;

			VProfile(): driver_name("ESRI Shapefile"), s_crs_wkt("") {}

			VProfile(const std::map<std::string, LayerDef>& _layers_def, char* _crs_wkt = nullptr, std::string _driver_name = "ESRI Shapefile") :
				layers_def(_layers_def), driver_name(_driver_name) {
				s_crs_wkt = (_crs_wkt == nullptr)? "" : std::string(_crs_wkt);
			}

			VProfile(const VProfile& _copy_vprofile) :
				layers_def(_copy_vprofile.layers_def), driver_name(_copy_vprofile.driver_name), s_crs_wkt(_copy_vprofile.s_crs_wkt) {}

			std::shared_ptr<GDALDataset> to_gdal_dataset(std::string fp) {
				GDALDriver* driver = GetGDALDriverManager()->GetDriverByName(driver_name.c_str());
				GDALDataset* new_dataset = driver->Create(fp.c_str(), 0, 0, 0, GDT_Unknown, NULL);
				if (new_dataset == NULL) {
					auto exception_description = "Cannot create dataset at {}" + fp;
					throw std::exception(exception_description.c_str());
				}
				
				OGRSpatialReference spatial_refrence;
				if (!s_crs_wkt.empty()) {
					spatial_refrence.importFromWkt(const_cast<char*>(s_crs_wkt.c_str()));
				}
				for (auto& c_layer_def : layers_def) {
					OGRLayer* new_layer = new_dataset->CreateLayer(c_layer_def.first.c_str(), &spatial_refrence, c_layer_def.second.wkb_type, NULL);
															
					for (const std::string& name : c_layer_def.second.int_attributes_names) {
						OGRFieldDefn o_field(name.c_str(), OFTInteger);
						if (new_layer->CreateField(&o_field) != OGRERR_NONE) {
							throw std::logic_error("Error : field creation failed.");
						}
					}
					for (const std::string& name : c_layer_def.second.double_attributes_names) {
						OGRFieldDefn o_field(name.c_str(), OFTReal);
						if (new_layer->CreateField(&o_field) != OGRERR_NONE) {
							throw std::logic_error("Error : field creation failed.");
						}
					}
					for (const std::string& name : c_layer_def.second.string_attributes_names) {
						OGRFieldDefn o_field(name.c_str(), OFTString);
						if (new_layer->CreateField(&o_field) != OGRERR_NONE) {
							throw std::logic_error("Error : field creation failed.");
						}
					}
				}

				return std::shared_ptr<GDALDataset>(new_dataset, GDALClose);
			}

			void update_gdal_dataset(std::shared_ptr<GDALDataset> vector_dataset) {

				int layer_idx = 0;
				for (auto& c_layer_def : layers_def) {
					// TODO: change layer acquisiation to be by name instead of index
					OGRLayer* layer_to_update = vector_dataset->GetLayer(layer_idx);
					if (layer_to_update->GetFeatureCount() > 0) {
						assert(layer_to_update->GetGeomType() == c_layer_def.second.wkb_type && "Different geometry type!");
						// TODO: check spatial reference system
						/*OGRSpatialReference* to_update_srs = layer_to_update->GetSpatialRef(); 
						const OGRSpatialReference* new_srs = vector_dataset->GetSpatialRef();
						assert(to_update_srs->IsSame(new_srs) && "Different spatial reference system!");
						*/
					}

					for (const std::string& name : c_layer_def.second.int_attributes_names) {
						OGRFieldDefn o_field(name.c_str(), OFTInteger);
						if (layer_to_update->CreateField(&o_field) != OGRERR_NONE) {
							throw std::logic_error("Error : field creation failed.");
						}
					}
					for (const std::string& name : c_layer_def.second.double_attributes_names) {
						OGRFieldDefn o_field(name.c_str(), OFTReal);
						if (layer_to_update->CreateField(&o_field) != OGRERR_NONE) {
							throw std::logic_error("Error : field creation failed.");
						}
					}
					for (const std::string& name : c_layer_def.second.string_attributes_names) {
						OGRFieldDefn o_field(name.c_str(), OFTString);
						if (layer_to_update->CreateField(&o_field) != OGRERR_NONE) {
							throw std::logic_error("Error : field creation failed.");
						}
					}
					layer_idx++;
				}

			}

			static VProfile from_gdal_dataset(std::shared_ptr<GDALDataset> gdal_dataset) {
				
				std::map<std::string, LayerDef> out_layers_def;
				char* crs_wkt =nullptr;
				std::string driver_name(gdal_dataset->GetDriverName());

				for (OGRLayer* c_layer : gdal_dataset->GetLayers()) {
					OGRSpatialReference* temp_spatial_ref = c_layer->GetSpatialRef();
					if (temp_spatial_ref)
						temp_spatial_ref->exportToWkt(&crs_wkt);
					std::string c_layer_name = c_layer->GetName();
					LayerDef c_layer_def =LayerDef::from_ogr_layer(c_layer);					
					out_layers_def[c_layer_name] = c_layer_def;
				}
				return VProfile(out_layers_def, crs_wkt, driver_name);
			}

			template <typename geom_type>
			static VProfile from_geometries_wa(const std::map<std::string, Geometries_with_attributes<geom_type>>& layers_geometries_map, std::string s_crs_wkt="", std::string driver_name="ESRI Shapefile") {
				VProfile out_profile;
				out_profile.s_crs_wkt = s_crs_wkt;
				out_profile.driver_name = driver_name;
				for (auto& kv : layers_geometries_map)
					out_profile.layers_def[kv.first] = LayerDef::from_geometry_wa(kv.second);
				return out_profile;
			}

		};

	}
}