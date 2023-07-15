#pragma once
#include "lightweight/geoimage.h"
#include "lightweight/raster_profile.h"
#include "io_raster.h"
#include "defs_opencv.h"
#include "graph_weights/spatial_weights.h"
#include "gdal_algs_wrap/gdal_rasterize.h"
#include "export_shared.h"
#include "lightweight/geovector.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		// Returns a pair of geoimage of rasterized geometries and a vector of respective labels
		template <typename geometry_type, typename cv_mat_type>
		std::pair<IO_DATA::GeoImage<cv_mat_type>, std::vector<size_t>> rasterize_geometries_masks(IO_DATA::GeoVector<geometry_type>& gvector, const IO_DATA::RProfile& reference_raster_profile) {

			std::vector<GeoImage<cv_mat_type>> output_masks; output_masks.reserve(gvector.length());

			// map of rasterization label for each geoemetry (labels are of type 2**i)
			std::vector<size_t> geometries_labels_vector;

			// since labels count should not be high (limited by the value type) \\ for example; for 64 geometries we will need 2**64 labels
			// we reset labels for every connected component
			std::unordered_map<size_t, size_t> in_component_labels;			
			SpatialWeights<geometry_type> PSW = SpatialWeights<geometry_type>(gvector.geometries_container, gvector.rtree);
			WeightsDistanceBandParams wdbp = { 0, true, -1, [](double x)->double { return x; } };
			PSW.fill_distance_band_graph(wdbp);
			PSW.run_labeling();

			for (size_t geometry_idx = 0; geometry_idx < gvector.length(); geometry_idx++) {
				size_t label_value = 1 << in_component_labels[PSW.component_labels[geometry_idx]];
				geometries_labels_vector.push_back(label_value);
				in_component_labels[PSW.component_labels[geometry_idx]] += 1;
			}


			//***************** Create memory shapefile ******************//
			std::string temp_shapefile_path = random_string(5, "/vsimem/", ".shp");
			GDALDriver* driver = (GDALDriver*)GDALGetDriverByName("ESRI Shapefile");
			if (driver == NULL)
			{
				printf("%s driver not available.\n", "Memory");
				exit(1);
			}
			GDALDataset* datasource = driver->Create(temp_shapefile_path.c_str(), 0, 0, 0, GDT_Unknown, NULL);
			if (datasource == NULL)
			{
				printf("Creation of output file failed.\n");
				exit(1);
			}

			if (!reference_raster_profile.s_crs_wkt.empty()) {
				OGRSpatialReference srs;
				srs.importFromWkt(reference_raster_profile.s_crs_wkt.c_str());
				datasource->SetSpatialRef(&srs);
			}

			OGRwkbGeometryType wkb_type;
			if constexpr (std::is_same_v<geometry_type, Boost_Polygon_2>) {
				wkb_type = wkbPolygon;
			}
			else {
				static_assert(false); //only polygon is implemented
			}
			OGRLayer* layer = datasource->CreateLayer("", NULL, wkb_type, NULL);
			if (layer == NULL)
			{
				printf("Layer creation failed.\n");
				exit(1);
			}

			OGRFieldDefn oField("label", OFTInteger64);
			if (layer->CreateField(&oField) != OGRERR_NONE)
			{
				printf("Creating Name field failed.\n");
				exit(1);
			}
			for (size_t geometry_idx = 0; geometry_idx < gvector.length(); geometry_idx++) {
				OGRFeature* poFeature;
				poFeature = OGRFeature::CreateFeature(layer->GetLayerDefn());
				poFeature->SetField("label", int(geometries_labels_vector[geometry_idx]));

				auto ogr_polygon = transform_B2OGR_Polygon(gvector[geometry_idx]);
				poFeature->SetGeometry(&ogr_polygon);

				if (layer->CreateFeature(poFeature) != OGRERR_NONE)
				{
					printf("Failed to create feature in shapefile.\n");
					exit(1);
				}

				OGRFeature::DestroyFeature(poFeature);
			}
			GDALClose(datasource);

			

			//**************** Create meomory output raster **************//
			std::string raster_output_path = random_string(5, "/vsimem/", ".tif");			
			
			OGREnvelope output_envelope = transform_B2OGR_Envelope(reference_raster_profile.to_box_extents());
			const std::list<std::string> extra_options = {
				//"-at", //all touched
				"-add", //necessary to differentiate between rasterized polygons
				"-a", "label"
			};
			double no_data_value = 0.0;
			const std::string output_datatype = "UInt16";
			
			rasterize_shapefile(
				raster_output_path, temp_shapefile_path, &output_envelope,
				std::abs(reference_raster_profile.geotransform[1]), std::abs(reference_raster_profile.geotransform[5]),
				extra_options, &no_data_value, output_datatype
			);

			IO_DATA::GeoImage<cv_mat_type> masks_geoimage = IO_DATA::GeoImage<cv_mat_type>::from_file(raster_output_path);
			masks_geoimage.set_geotransform(reference_raster_profile.geotransform);
				/*for (size_t geometry_idx = 0; geometry_idx < gvector.size(); geometry_idx++) {
					Boost_Box_2 c_geometry_bounds;
					boost::geometry::envelope(gvector[geometry_idx], c_geometry_bounds);
					GeoImage<cv_mat_type> c_geom_mask = masks_geoimage.get_view_spatial<cv_mat_type>(
						c_geometry_bounds.min_corner().get<0>(), c_geometry_bounds.min_corner().get<1>(),
						c_geometry_bounds.max_corner().get<0>(), c_geometry_bounds.max_corner().get<1>()
					);


					cv_mat_type temp_mat = cv_mat_type(c_geom_mask.image.size(), c_geom_mask.image.type()); temp_mat.setTo(int(geometries_labels_vector[geometry_idx]));
					cv_mat_type out_mat(c_geom_mask.image.size(), c_geom_mask.image.type());
					if constexpr (std::is_same_v<cv_mat_type, cv::cuda::GpuMat>)
						cv::cuda::bitwise_and(c_geom_mask.image.clone(), temp_mat, out_mat);
					else
						cv::bitwise_and(c_geom_mask.image, temp_mat, out_mat);

					c_geom_mask.set_image( out_mat );
					output_masks.push_back(c_geom_mask);
				}*/
			auto output_pair = std::make_pair(masks_geoimage, geometries_labels_vector);
			return output_pair;
		}
	};
}