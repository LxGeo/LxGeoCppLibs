#pragma once
#include "defs.h"
#include "defs_boost.h"
#include "defs_ogr.h"
#include "defs_common.h"
#include "export_io_data.h"
#include "geometries_with_attributes/geometries_with_attributes.h"
#include "geometries_with_attributes/point_with_attribute.h"
#include "geometries_with_attributes/linestring_with_attributes.h"
#include "geometries_with_attributes/polygon_with_attributes.h"

namespace LxGeo
{

	namespace IO_DATA
	{
		using namespace LxGeo::GeometryFactoryShared;

		template <typename geometry_type>
		class ShapefileIO {

		public:

			ShapefileIO() {
				feature_count = 0;
				bounding_box = Boost_Box_2();
				spatial_refrence = NULL;
				vector_dataset = NULL;
				geometries_container = std::vector<geometry_type>();
			};

			ShapefileIO(std::string& out_path, OGRSpatialReference* copy_srs):ShapefileIO(){								
				
				try {
					const std::string driver_name = "ESRI Shapefile";

					GDALDriver* driver = GetGDALDriverManager()->GetDriverByName(driver_name.c_str());
					if (driver == NULL) {
						throw std::logic_error("Error : ESRI Shapefile driver not available.");
					}

					// Step 1.
					// Writes target file

					vector_dataset = driver->Create(out_path.c_str(), 0, 0, 0, GDT_Unknown, NULL);
					if (vector_dataset == NULL) {
						throw std::logic_error("Error : creation of output file failed.");
					}
				}
				catch (std::exception& e) {
					std::cout << e.what() << std::endl;
				}
				if (vector_dataset == NULL) return;
				
				if (copy_srs)
					spatial_refrence = copy_srs->Clone();
				else
					spatial_refrence = nullptr;

				//create_layer();
			}

			~ShapefileIO() {
				if (vector_dataset != NULL) GDALClose(vector_dataset);
			}

			bool create_layer() {
				OGRwkbGeometryType wkb_type;
				if constexpr (std::is_same_v<geometry_type, Boost_Polygon_2>) {
					wkb_type = wkbPolygon;
				}
				else if constexpr (std::is_same_v<geometry_type, Boost_Point_2>) {
					wkb_type = wkbPoint;
				}
				else if constexpr (std::is_same_v<geometry_type, Boost_LineString_2>) {
					wkb_type = wkbLineString;
				}
				else {
					static_assert(false); //only point,linestring,polygon is implemented
				}

				try {
					vector_layer = vector_dataset->CreateLayer("", spatial_refrence, wkb_type, NULL);
					if (vector_layer == NULL) {
						throw std::logic_error("Error : layer creation failed.");
					}
				}
				catch (std::exception& e) {
					std::cout << e.what() << std::endl;
				}
				if (vector_layer == NULL) return false;
				return true;
			}

			bool create_layer_attributes(std::list<std::string>& int_attributes,
				std::list<std::string>& double_attributes,
				std::list<std::string>& string_attributes
			) {
				for (const std::string& name : int_attributes) {
					OGRFieldDefn o_field(name.c_str(), OFTInteger);
					if (vector_layer->CreateField(&o_field) != OGRERR_NONE) {
						throw std::logic_error("Error : field creation failed.");
					}
				}
				for (const std::string& name : double_attributes) {
					OGRFieldDefn o_field(name.c_str(), OFTReal);
					if (vector_layer->CreateField(&o_field) != OGRERR_NONE) {
						throw std::logic_error("Error : field creation failed.");
					}
				}
				for (const std::string& name : string_attributes) {
					OGRFieldDefn o_field(name.c_str(), OFTString);
					if (vector_layer->CreateField(&o_field) != OGRERR_NONE) {
						throw std::logic_error("Error : field creation failed.");
					}
				}
			}
			
			bool load_shapefile(std::string shapefile_path, bool lazy_load=true, OGRGeometry* spatial_filter=nullptr) {

				vector_dataset = (GDALDataset*)GDALOpenEx(shapefile_path.c_str(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, NULL, NULL, NULL);
				if (vector_dataset == NULL)
				{
					BOOST_LOG_TRIVIAL(fatal) << fmt::format("Error loading shapefile at {} !", shapefile_path);
					return false;
				}

				vector_layer = vector_dataset->GetLayer(0);
				if (vector_layer == NULL)
				{
					BOOST_LOG_TRIVIAL(fatal) << fmt::format("Error loading shapefile first layer at {} !", shapefile_path);
					return false;
				}

				feature_count = size_t(vector_layer->GetFeatureCount());
				if (feature_count < 1) {
					std::cout << fmt::format("Empty shapefile at {} !", shapefile_path);
					return false;
				}
				geom_type = vector_layer->GetGeomType();

				OGREnvelope layer_extent;
				if (spatial_filter == nullptr) {
					vector_layer->GetExtent(&layer_extent);
				}
				else {
					vector_layer->SetSpatialFilter(spatial_filter);
					OGR_G_GetEnvelope(spatial_filter, &layer_extent);
				}
				bounding_box = Boost_Box_2(
					Boost_Point_2(layer_extent.MinX, layer_extent.MinY),
					Boost_Point_2(layer_extent.MaxX, layer_extent.MaxY)
				);

				spatial_refrence = vector_layer->GetSpatialRef();

				if (!lazy_load) {
					try {
						load_geometries();
					}
					catch (std::exception& e) {
						BOOST_LOG_TRIVIAL(debug) << e.what();
						BOOST_LOG_TRIVIAL(fatal) << "Fatal error while load shapefile data !";
						return false;
					}
				}

				return true;
			}

			virtual void load_geometries() =0;

			void write_shapefile() {
				assert(geom_type == geometry_type && "Geometry type issues!");
				//transform geometries_container to Geometries_with_attributes container
				throw std::exception("Not implemented yet!");
				std::vector<Geometries_with_attributes<geometry_type>> transformed;
				write_shapefile(transformed);
			}

			void write_geometries(std::list<OGRGeometryH>& geoms_to_write) {

				for (OGRGeometryH& c_geom : geoms_to_write) {
					OGRFeature* feature;
					feature = OGRFeature::CreateFeature(vector_layer->GetLayerDefn());
					OGRGeometry* geom_to_set = OGRGeometry::FromHandle(c_geom);
					feature->SetGeometry(geom_to_set);
					// Writes new feature
					OGRErr error = vector_layer->CreateFeature(feature);
					if (error != OGRERR_NONE) std::cout << "Error code : " << int(error) << std::endl;
					OGRFeature::DestroyFeature(feature);
				}
				vector_layer->SyncToDisk();
			};
			

		public:
			GDALDataset* vector_dataset;
			OGRLayer* vector_layer;
			size_t feature_count;
			OGRwkbGeometryType geom_type;
			Boost_Box_2 bounding_box;
			OGRSpatialReference* spatial_refrence;
			
			std::vector<geometry_type> geometries_container;
		};


		//********************* PolygonsShapfileIO ********************//
		class PolygonsShapfileIO : public ShapefileIO<Boost_Polygon_2> {
			public:
				PolygonsShapfileIO() : ShapefileIO<Boost_Polygon_2>() {};
				PolygonsShapfileIO(std::string out_path, OGRSpatialReference* copy_srs) : ShapefileIO<Boost_Polygon_2>(out_path, copy_srs) { create_layer(); };

				void load_geometries() override {
					assert(geom_type == wkbPolygon && "Error! Loading Polygon geometries from non Polygon geometry layer!");
					geometries_container = fill_polygon_container();
				}

				std::vector<Boost_Polygon_2> fill_polygon_container() {

					std::vector<Boost_Polygon_2> loaded_polygons;
					loaded_polygons.reserve(feature_count);
					vector_layer->ResetReading();
					for (size_t j = 0; j < feature_count; ++j) {
						OGRFeature* feat = vector_layer->GetNextFeature();
						if (feat == NULL) continue;

						OGRGeometry* geom = feat->GetGeometryRef();

						// Assumes the shapefile only contains OGRPolygons
						if (OGRPolygon* P = dynamic_cast<OGRPolygon*>(geom)) {
							loaded_polygons.push_back(transform_OGR2B_Polygon(P));
						}
					}
					loaded_polygons.shrink_to_fit();
					return loaded_polygons;
				}

				void write_polygon_shapefile(std::vector<Polygon_with_attributes>& polygon_container) {
					if (polygon_container.empty()) {
						std::cout << "Warning : empty vector of linestrings. No output written." << std::endl;
						return;
					}

					Polygon_with_attributes P_ref = polygon_container[0];
					std::list<std::string> int_attributes, double_attributes, string_attributes;
					P_ref.get_list_of_int_attributes(int_attributes);
					P_ref.get_list_of_double_attributes(double_attributes);
					P_ref.get_list_of_string_attributes(string_attributes);
					bool layer_created_success = create_layer_attributes(int_attributes, double_attributes, string_attributes);
					if (!layer_created_success) {
						std::cout << "Error creating out layer!\n";
						throw std::exception("Layer creation error!");
					}

					for (size_t i = 0; i < polygon_container.size(); ++i) {
						Polygon_with_attributes& p = polygon_container[i];
						OGRPolygon ogr_polygon = transform_B2OGR_Polygon(p.get_definition());

						OGRFeature* feature;
						feature = OGRFeature::CreateFeature(vector_layer->GetLayerDefn());

						feature->SetGeometry(&ogr_polygon);
						for (const std::string& name : int_attributes) {
							int x = p.get_int_attribute(name);
							feature->SetField(name.c_str(), x);
						}

						for (const std::string& name : double_attributes) {
							double x = p.get_double_attribute(name);
							feature->SetField(name.c_str(), x);
						}
						for (const std::string& name : string_attributes) {
							std::string x = p.get_string_attribute(name);
							feature->SetField(name.c_str(), x.c_str());
						}

						// Writes new feature
						OGRErr error = vector_layer->CreateFeature(feature);
						if (error != OGRERR_NONE) std::cout << "Error code : " << int(error) << std::endl;
						OGRFeature::DestroyFeature(feature);
					}
				};

				void write_shapefile(std::vector<Polygon_with_attributes>& polygon_container) {
					try {
						write_polygon_shapefile(polygon_container);
						vector_layer->SyncToDisk();
					}
					catch (const std::exception& e) // reference to the base of a polymorphic object
					{
						std::cout << e.what(); // information from length_error printed
					}
				};
		};


		//********************* LineStringShapfileIO ********************//
		class LineStringShapfileIO : public ShapefileIO<Boost_LineString_2> {
		public:
			LineStringShapfileIO() : ShapefileIO<Boost_LineString_2>() {};
			LineStringShapfileIO(std::string& out_path, OGRSpatialReference* copy_srs) : ShapefileIO<Boost_LineString_2>(out_path, copy_srs) { create_layer(); };

			void load_geometries() override {
				assert(geom_type == wkbLineString && "Error! Loading Linestring geometries from non Linestring geometry layer!");
				geometries_container = fill_linestring_container();
			}

			std::vector<Boost_LineString_2> fill_linestring_container() {
				std::vector<Boost_LineString_2> loaded_linestring;

				loaded_linestring.reserve(feature_count);
				vector_layer->ResetReading();
				for (size_t j = 0; j < feature_count; ++j) {
					OGRFeature* feat = vector_layer->GetNextFeature();
					if (feat == NULL) continue;

					OGRGeometry* geom = feat->GetGeometryRef();

					// Assumes the shapefile only contains OGRLineString
					if (OGRLineString* P = dynamic_cast<OGRLineString*>(geom)) {
						loaded_linestring.push_back(transform_OGR2B_Linestring(P));
					}
				}
				loaded_linestring.shrink_to_fit();

				return loaded_linestring;
			}

			void write_linestring_shapefile(std::vector<LineString_with_attributes>& linestring_container) {

				if (linestring_container.empty()) {
					std::cout << "Warning : empty vector of linestrings. No output written." << std::endl;
					return;
				}

				LineString_with_attributes L_ref = linestring_container[0];
				std::list<std::string> int_attributes, double_attributes, string_attributes;
				L_ref.get_list_of_int_attributes(int_attributes);
				L_ref.get_list_of_double_attributes(double_attributes);
				L_ref.get_list_of_string_attributes(string_attributes);
				create_layer_attributes(int_attributes, double_attributes, string_attributes);

				for (size_t i = 0; i < linestring_container.size(); ++i) {
					LineString_with_attributes& l = linestring_container[i];
					OGRLineString ogr_linestring = transform_B2OGR_LineString(l.get_definition());

					OGRFeature* feature;
					feature = OGRFeature::CreateFeature(vector_layer->GetLayerDefn());

					feature->SetGeometry(&ogr_linestring);
					for (const std::string& name : int_attributes) {
						int x = l.get_int_attribute(name);
						feature->SetField(name.c_str(), x);
					}
					for (const std::string& name : double_attributes) {
						double x = l.get_double_attribute(name);
						feature->SetField(name.c_str(), x);
					}
					for (const std::string& name : string_attributes) {
						std::string x = l.get_string_attribute(name);
						feature->SetField(name.c_str(), x.c_str());
					}

					// Writes new feature
					OGRErr error = vector_layer->CreateFeature(feature);
					if (error != OGRERR_NONE) std::cout << "Error code : " << int(error) << std::endl;
					OGRFeature::DestroyFeature(feature);
				}
				vector_layer->SyncToDisk();

			};

			void write_shapefile(std::vector<LineString_with_attributes>& lines_container) {
				write_linestring_shapefile(lines_container);
			};
		};


		//********************* PointsShapfileIO ********************//
		class PointsShapfileIO : public ShapefileIO<Boost_Point_2> {
		public:
			PointsShapfileIO() : ShapefileIO<Boost_Point_2>() {};
			PointsShapfileIO(std::string& out_path, OGRSpatialReference* copy_srs) : ShapefileIO<Boost_Point_2>(out_path, copy_srs) { create_layer(); };

			void load_geometries() override {
				assert(geom_type == wkbPoint && "Error! Loading Points geometries from non Point geometry layer!");
				geometries_container = fill_point_container();
			}

			std::vector<Boost_Point_2> fill_point_container() {
				std::vector<Boost_Point_2> loaded_points;
				return loaded_points;
			}

			void write_point_shapefile(std::vector<Point_with_attributes>& pts_container) {
				if (pts_container.empty()) {
					std::cout << "Warning : empty vector of point. No output written." << std::endl;
					return;
				}

				Point_with_attributes pt_ref = pts_container[0];
				std::list<std::string> int_attributes, double_attributes, string_attributes;
				pt_ref.get_list_of_int_attributes(int_attributes);
				pt_ref.get_list_of_double_attributes(double_attributes);
				pt_ref.get_list_of_string_attributes(string_attributes);
				create_layer_attributes(int_attributes, double_attributes, string_attributes);

				for (size_t i = 0; i < pts_container.size(); ++i) {
					Point_with_attributes& p = pts_container[i];
					OGRPoint ogr_point = transform_B2OGR_Point(p.get_definition());

					OGRFeature* feature;
					feature = OGRFeature::CreateFeature(vector_layer->GetLayerDefn());

					feature->SetGeometry(&ogr_point);
					for (const std::string& name : int_attributes) {
						int x = p.get_int_attribute(name);
						feature->SetField(name.c_str(), x);
					}
					for (const std::string& name : double_attributes) {
						double x = p.get_double_attribute(name);
						feature->SetField(name.c_str(), x);
					}
					for (const std::string& name : string_attributes) {
						std::string x = p.get_string_attribute(name);
						feature->SetField(name.c_str(), x.c_str());
					}

					// Writes new feature
					OGRErr error = vector_layer->CreateFeature(feature);
					if (error != OGRERR_NONE) std::cout << "Error code : " << int(error) << std::endl;
					OGRFeature::DestroyFeature(feature);
				}
			};

			void write_shapefile(std::vector<Point_with_attributes>& pts_container) {
				write_point_shapefile(pts_container);
			};
		};

	}
}