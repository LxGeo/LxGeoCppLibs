#pragma once
#include "lightweight/geoimage.h"
#include "io_raster.h"
#include "defs_opencv.h"
#include "graph_weights/polygons_spatial_weights.h"
#include "gdal_algs_wrap/gdal_rasterize.h"
#include "export_shared.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		// parent dataset is used to get SRS & geotransform for correct overlay
		template <typename geometry_type, typename cv_mat_type>
		std::vector<IO_DATA::GeoImage<cv_mat_type>> rasterize_geometries_masks(std::vector<geometry_type>& geometry_container, IO_DATA::RasterIO& reference_raster) {

			std::vector<GeoImage<cv_mat_type>> output_masks; output_masks.reserve(geometry_container.size());

			// map of rasterization label for each geoemetry (labels are of type 2**i)
			std::vector<size_t> geometries_labels_vector;

			// since labels count should not be high (limited by the value type) \\ for example; for 64 geometries we will need 2**64 labels
			// we reset labels for every connected component
			std::unordered_map<size_t, size_t> in_component_labels;
			// TODO: implement all geometries spatial weights
			PolygonSpatialWeights PSW = PolygonSpatialWeights(geometry_container);
			WeightsDistanceBandParams wdbp = { 0, true, -1, [](double x)->double { return x; } };
			PSW.fill_distance_band_graph(wdbp);
			PSW.run_labeling();

			for (size_t geometry_idx = 0; geometry_idx < geometry_container.size(); geometry_idx++) {
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
			for (size_t geometry_idx = 0; geometry_idx < geometry_container.size(); geometry_idx++) {
				OGRFeature* poFeature;
				poFeature = OGRFeature::CreateFeature(layer->GetLayerDefn());
				poFeature->SetField("label", int(geometries_labels_vector[geometry_idx]));


				poFeature->SetGeometry(&transform_B2OGR_Polygon(geometry_container[geometry_idx]));

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
			OGREnvelope output_envelope = transform_B2OGR_Envelope(reference_raster.bounding_box);
			const std::list<std::string> extra_options = {
				"-at", //all touched
				"-add", //necessary to differentiate between rasterized polygons
				"-a", "label"
			};
			double no_data_value = 0.0;
			const std::string output_datatype = "UInt16";
			
			rasterize_shapefile(
				raster_output_path, temp_shapefile_path, &output_envelope,
				reference_raster.get_pixel_width(), reference_raster.get_pixel_height(),
				extra_options, &no_data_value, output_datatype
			);

			/*
			GDALDataset* pSrcDataset = static_cast<GDALDataset*>(GDALOpen(raster_output_path.c_str(), GA_ReadOnly));
			GDALDriver* ESRIDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
			GDALDataset* jpeg_ds = ESRIDriver->CreateCopy("C:/DATA_SANDBOX/maskedMatching/LA/sample1/matching_output/result.tif", pSrcDataset, false, NULL, GDALDummyProgress, NULL);
			GDALClose(jpeg_ds);
			*/

			IO_DATA::RasterIO masks_raster(raster_output_path, GA_ReadOnly, false);
			GeoImage<cv::cuda::GpuMat> masks_geoimage(masks_raster.raster_data, masks_raster.geotransform);
			for (size_t geometry_idx = 0; geometry_idx < geometry_container.size(); geometry_idx++) {				
				Boost_Box_2 c_geometry_bounds;
				boost::geometry::envelope(geometry_container[geometry_idx], c_geometry_bounds);
				GeoImage<cv::cuda::GpuMat> c_geom_mask = masks_geoimage.get_view_spatial<cv::cuda::GpuMat>(
					c_geometry_bounds.min_corner().get<0>(), c_geometry_bounds.min_corner().get<1>(),
					c_geometry_bounds.max_corner().get<0>(), c_geometry_bounds.max_corner().get<1>()
				);

				//cv::cuda::GpuMat image1(81, 38, c_geom_mask.image.type(), 64), temp_mat1(81, 38, c_geom_mask.image.type(), 2621), out_mat1(81, 38, c_geom_mask.image.type(),17);
				//image1.setTo(2); temp_mat1.setTo(6); out_mat1.setTo(0);
				//cv::cuda::bitwise_and(image1, temp_mat1, out_mat1);

				cv_mat_type temp_mat = cv_mat_type(c_geom_mask.image.size(), c_geom_mask.image.type()); temp_mat.setTo(int(geometries_labels_vector[geometry_idx]));
				cv_mat_type out_mat(c_geom_mask.image.size(), c_geom_mask.image.type());
				cv::cuda::bitwise_and(c_geom_mask.image.clone(), temp_mat, out_mat);
				c_geom_mask.set_image( out_mat );
				GeoImage<cv_mat_type> to_push_geoimage(out_mat, c_geom_mask.geotransform);
				output_masks.push_back(to_push_geoimage);
			}
			return output_masks;
		}
	}
}