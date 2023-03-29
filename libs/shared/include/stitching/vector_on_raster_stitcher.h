#pragma once
#include "defs.h"
#include "affine_geometry/affine_transformer.h"
#include "lightweight/geoimage.h"
#include "geometries_with_attributes/geometries_with_attributes.h"
#include "export_shared.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		enum class RasterPixelsStitcherStartegy {
			vertex_only = 1 << 0,
			vertex_and_mid_point = 1 << 1,
			constant_walker = 1 << 2,
			contours = 1 << 3,
			filled_polygon = 1 << 4
		};

		using namespace LxGeo::IO_DATA;

		class RasterPixelsStitcher {

		public:
			//RasterPixelsStitcher() {};
			RasterPixelsStitcher(GeoImage<cv::Mat>& _ref_gimg): ref_gimg(_ref_gimg), inv_transformer_matrix(geotransform_to_inv_matrix_transformer(ref_gimg.geotransform))
			{};
			
			double RasterPixelsStitcher::readPolygonPixels(Boost_Polygon_2& resp_polygon, RasterPixelsStitcherStartegy strategy) {
				if (strategy == RasterPixelsStitcherStartegy::contours) {

					double total_obj = 0;
					const double out_of_extent_ground = 1e2;
					auto line_iter_reader = [&out_of_extent_ground](cv::LineIterator& it, matrix& ref_mat)->double {
						double sum = 0;
						for (int i = 0; i < it.count; i++, ++it) {
							sum += ref_mat.at<float>(it.pos());
						}
						return sum;// / double(it.count);
					};

					std::list<std::list<cv::Point>> polygons_rings_pixels_coords;
					// rings pixels coords extraction
					std::list<Boost_Ring_2*> c_polygon_rings;
					c_polygon_rings.push_back(&resp_polygon.outer());
					for (Boost_Ring_2& c_inner_ring : resp_polygon.inners()) c_polygon_rings.push_back(&c_inner_ring);

					// transform spatial points to pixel coords points
					for (auto* c_p_ring : c_polygon_rings) {
						std::list<cv::Point> c_ring_pixels;
						for (size_t c_pt_idx = 0; c_pt_idx < c_p_ring->size(); ++c_pt_idx) {
							cv::Point c_pixel_pt;
							ref_gimg._calc_pixel_coords(c_p_ring->at(c_pt_idx).get<0>(), c_p_ring->at(c_pt_idx).get<1>(), c_pixel_pt.x, c_pixel_pt.y);
							c_ring_pixels.push_back(c_pixel_pt);
						}
						polygons_rings_pixels_coords.push_back(c_ring_pixels);
					}

					// Reading pixels using cv LineIterator
					for (auto pixel_ring : polygons_rings_pixels_coords) {
						auto pts_iter = pixel_ring.begin();
						for (size_t iter_count = 0; iter_count < pixel_ring.size() - 1; ++iter_count) {
							cv::Point st_pt = (*pts_iter), end_pt = *next(pts_iter);
							cv::LineIterator it(ref_gimg.image, st_pt, end_pt, 8);
							total_obj += (*it != nullptr) ? line_iter_reader(it, ref_gimg.image) : out_of_extent_ground;
							pts_iter++;
						}
					}

					return total_obj;

				}

				if (strategy == RasterPixelsStitcherStartegy::filled_polygon) {
					double total_obj = 0;
					using pixel_type = cv::Vec<float, 1>;

					auto ring_pixels_aggregator = [](std::list<pixel_type>& values_list) -> float {
						// (temporary) returns sum with nan turned to max_val
						int null_count = 0;
						float max_val = 0.0;
						float sum = 0.0;

						for (auto& c_pixel : values_list) {
							float c_val = c_pixel[0];
							if (isnan(c_val)) null_count += 1;
							else {
								sum += c_val;
								if (c_val > max_val) max_val = c_val;
							}
						}
						return sum + max_val * null_count;
					};

					Boost_Discrete_Polygon_2 resp_polygon_pixel_coords = affine_transform_geometry<Boost_Polygon_2, Boost_Discrete_Polygon_2>(
						resp_polygon, inv_transformer_matrix
						);

					Boost_Discrete_Box_2 envelop;
					boost::geometry::envelope(resp_polygon_pixel_coords, envelop);
					Boost_Discrete_Point_2 min_pixel_corner = envelop.min_corner();
					Boost_Discrete_Point_2 max_pixel_corner = envelop.max_corner();

					std::list<pixel_type> loaded_pixels;
					for (size_t c_row = min_pixel_corner.get<1>(); c_row <= max_pixel_corner.get<1>(); c_row++) {

						if (c_row < 0 || c_row >= ref_gimg.image.rows)
							continue;

						for (size_t c_col = min_pixel_corner.get<0>(); c_col <= max_pixel_corner.get<0>(); c_col++) {

							if (c_col < 0 || c_col >= ref_gimg.image.cols)
								continue;

							cv::Point c_pos(c_col, c_row);
							Boost_Discrete_Point_2 c_p(c_col, c_row);

							// within exterior ring
							if (boost::geometry::within(c_p, resp_polygon_pixel_coords.outer())) {
								bool is_within_hole = false;
								for (size_t c_ring_idx = 0; c_ring_idx < resp_polygon_pixel_coords.inners().size(); c_ring_idx++) {
									auto& c_interior_ring = resp_polygon_pixel_coords.inners()[c_ring_idx];
									// within interior ring
									if (boost::geometry::within(c_p, c_interior_ring))
									{
										//loaded_pixels.push_back(ref_gimg.image.ptr<pixel_type>(c_row)[c_col]);
										is_within_hole = true;
										break;
									}
								}
								if (!is_within_hole)
									loaded_pixels.push_back(ref_gimg.image.ptr<pixel_type>(c_row)[c_col]);
							}
						}
					}


					total_obj += ring_pixels_aggregator(loaded_pixels);
					return total_obj;

				}

				else
					throw std::exception("Only contours and filled_polygon strategies are implemented!");
			}

		public:
			GeoImage<cv::Mat> ref_gimg;
			bg::strategy::transform::inverse_transformer<double, 2, 2> inv_transformer_matrix;

		};
	}
}