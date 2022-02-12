#include "io_shapefile.h"


namespace LxGeo
{

	namespace IO_DATA
	{
		/*template <typename geometry_type>
		bool ShapefileIO<geometry_type>::load_shapefile(std::string shapefile_path, bool lazy_load) {

			vector_dataset = (GDALDataset*)GDALOpenEx(shapefile_path.c_str(), GDAL_OF_VECTOR | GDAL_OF_UPDATE, NULL, NULL, NULL);
			if (vector_dataset == NULL)
			{
				BOOST_LOG_TRIVIAL(fatal) << fmt::format("Error loading shapefile at {} !", shapefile_path);
				return false;
			}

			vector_layer = dataset->GetLayer(0);
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
			vector_layer->(&layer_extent);
			bounding_box = Boost_Box_2(
				Boost_Point_2(layer_extent.MinX(), layer_extent.MinY()),
				Boost_Point_2(layer_extent.MaxX(), layer_extent.MaxY())
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

		template <typename geometry_type>
		void ShapefileIO<geometry_type>::load_geometries() {

			if (boost::is_same<geometry_type, Boost_Point_2>::value) {
				assert(geom_type == wkbPoint && "Error! Loading Points geometries from non Point geometry layer!");
				geometries_container = fill_point_container();
			}

			if (boost::is_same<geometry_type, Boost_LineString_2>::value) {
				assert(geom_type == wkbLineString && "Error! Loading linestring geometries from non linestring geometry layer!");
				geometries_container = fill_linestring_container();
			}

			if (boost::is_same<geometry_type, Boost_Polygon_2>::value) {
				assert(geom_type == wkbLineString && "Error! Loading Polygon geometries from non Polygon geometry layer!");
				geometries_container = fill_polygon_container();
			}

		}
		
		template <typename geometry_type>
		std::vector<Boost_Point_2> ShapefileIO<geometry_type>::fill_point_container(){
			std::vector<Boost_Point_2> loaded_points;
			return loaded_points;
		}

		template <typename geometry_type>
		std::vector<Boost_LineString_2> ShapefileIO<geometry_type>::fill_linestring_container() {
			std::vector<Boost_LineString_2> loaded_linestring;
			return loaded_linestring;
		}

		template <typename geometry_type>
		std::vector<Boost_Polygon_2> ShapefileIO<geometry_type>::fill_polygon_container() {
		
			std::vector<Boost_Polygon_2> loaded_polygons;
			loaded_polygons.reserve(feature_count);

			for (size_t j = 0; j < feature_count; ++j) {
				OGRFeature* feat = layer->GetNextFeature();
				if (feat == NULL) continue;

				OGRGeometry* geom = feat->GetGeometryRef();

				// Assumes the shapefile only contains OGRPolygons
				if (OGRPolygon* P = dynamic_cast<OGRPolygon*>(geom)) {
					loaded_polygons.push_back(transform_OGR2B_Polygon(P));
				}
			}
			loaded_polygons.shrink_to_fit();

		}*/

	}
}