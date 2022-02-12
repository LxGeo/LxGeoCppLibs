#pragma once
#include "defs.h"
#include "export_io_data.h"

namespace LxGeo
{

	namespace IO_DATA
	{
		template <typename geometry_type>
		class ShapefileIO {

		public:

			IO_DATA_API ShapefileIO() {
				feature_count = 0;
				bounding_box = Boost_Box_2();
				spatial_refrence = NULL;
				vector_dataset = NULL;
				geometries_container = std::vector<geometry_type>();
			};

			
			IO_DATA_API bool ShapefileIO<geometry_type>::load_shapefile(std::string shapefile_path, bool lazy_load) {

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
				vector_layer->GetExtent(&layer_extent);
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

			IO_DATA_API virtual void ShapefileIO::load_geometries() {};

			IO_DATA_API std::vector<Boost_Point_2> ShapefileIO::fill_point_container() {
				std::vector<Boost_Point_2> loaded_points;
				return loaded_points;
			}

			IO_DATA_API std::vector<Boost_LineString_2> ShapefileIO::fill_linestring_container() {
				std::vector<Boost_LineString_2> loaded_linestring;
				return loaded_linestring;
			}

			IO_DATA_API std::vector<Boost_Polygon_2> ShapefileIO::fill_polygon_container() {

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

			

		public:
			GDALDataset* vector_dataset;
			OGRLayer* vector_layer;
			size_t feature_count;
			OGRwkbGeometryType geom_type;
			Boost_Box_2 bounding_box;
			OGRSpatialReference* spatial_refrence;
			
			std::vector<geometry_type> geometries_container;
		};

		class PolygonsShapfileIO : public ShapefileIO<Boost_Polygon_2> {
			public:
				IO_DATA_API void PolygonsShapfileIO::load_geometries() {
					assert(geom_type == wkbPolygon && "Error! Loading Polygon geometries from non Polygon geometry layer!");
					geometries_container = fill_polygon_container();
				}
		};

		class LineStringShapfileIO : ShapefileIO<Boost_LineString_2> {
		public:
			IO_DATA_API void LineStringShapfileIO::load_geometries() {
				assert(geom_type == wkbLineString && "Error! Loading Linestring geometries from non Linestring geometry layer!");
				geometries_container = fill_linestring_container();
			}
		};

		class PointsShapfileIO : ShapefileIO<Boost_Point_2> {
		public:
			IO_DATA_API void PointsShapfileIO::load_geometries() {
				assert(geom_type == wkbPoint && "Error! Loading Points geometries from non Point geometry layer!");
				geometries_container = fill_point_container();
			}
		};

	}
}