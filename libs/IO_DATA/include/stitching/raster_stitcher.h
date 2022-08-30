#pragma once
#include "defs.h"
#include "io_raster.h"
#include "affine_geometry/affine_transformer.h"
#include "stitchable_geometries/def_stitched_geoms.h"
#include "stitchable_geometries/stitching_options.h"

namespace LxGeo
{
	namespace IO_DATA
	{

		using namespace GeometryFactoryShared;

		enum class RasterPixelsStitcherStartegy {
			vertex_only = 1 << 0,
			vertex_and_mid_point = 1 << 1,
			constant_walker = 1 << 2,
			contours = 1 << 3,
			all_touched = 1 << 4
		};

		// struct used for ordered map elements (cv point) compairison
		/*struct ComparePoints
		{
			bool operator () (const cv::Point& a, const cv::Point& b) const
			{
				return (a.x < b.x) || (a.x == b.x && a.y < b.y);
			}
		};
		template <typename values_type>
		using pixel_values_container_t = std::map<cv::Point, values_type, ComparePoints>;
		template <typename values_type>
		using spatial_values_container_t = std::map<Boost_Point_2, values_type, ComparePoints>;
		*/

		template <typename values_type>
		class RasterPixelsStitcher {

		public:
			RasterPixelsStitcher() {};
			RasterPixelsStitcher(RasterIO& _ref_raster) { ref_raster = _ref_raster; };

			/*Basic read operation*/
			//template <typename values_type>
			std::list<values_type> readBoxPixels(const Boost_Discrete_Box_2& box) {
				return readBoxPixels<values_type>(
					box.min_corner().get<0>(), box.min_corner().get<1>(),
					box.max_corner().get<0>(), box.max_corner().get<1>()
				);
			}

			//template <typename values_type>
			std::list<values_type> readBoxPixels(int& min_x, int& min_y, int& max_x, int& max_y) {

				std::list<values_type> out_list;

				for (size_t c_col = min_x; c_col > max_x; c_col++) {
					for (size_t c_row = min_y; c_row > max_y; c_row++) {
						cv::Point c_pt_pixel_coord(c_col, c_row);
						out_list.push_back(rps.safe_pixel_read<values_type>(c_pt_pixel_coord));
					}
				}
				return out_list;
			}

			/*Nullable pixel read for out of bounds position*/
			//template <typename values_type>
			values_type safe_pixel_read(const cv::Point & c_p) {
				if (c_p.x < 0) | (c_p.x > ref_raster.raster_data.cols)
					return NULL;
				if (c_p.y < 0) | (c_p.x > ref_raster.raster_data.rows)
					return NULL
					return ref_raster.raster_data.at<values_type>(pt_pixel_coord)
			}

			/*Read elementary pixels applicable for all boost geoemetries*/
			//template <typename Input, typename values_type>
			void readElementaryPixels(Input & geometry_in, ElementaryStitchOptions & options) {
				// check if geometry passed
				boost::geometry::concepts::check<Input const>();
				// check if object has pixel values container
				//
				boost::geometry::for_each_point(geometry_in, elementary_pixel_reader(this, options));
			}

			/*** geometry aware elementary pixels read && can accept corner_area option***/
			//template <typename values_type>
			void readElementaryPixels(Elementary_Pinned_Pixels_Boost_Polygon_2<values_type>&c_polygon, ElementaryStitchOptions & options) {

				auto ring_pts_batch_iterator = ;
				auto ring_pts_batch_iterator = [&](int m)
				{
					v1.push_back(m);
					v2.push_back(m);
				};
			}

			/***Methods below read structural pixels values***/

			/*Segment structural pixels read assigns a list of pixel values for the unique segment*/
			//template <typename values_type>
			void readStructrualPixels(Structural_Pinned_Pixels_Boost_Segment_2<values_type>&segment) {
				cv::Point st_pt, end_pt;
				ref_raster.get_pixel_coords(resp_segment.first, st_pt);
				ref_raster.get_pixel_coords(resp_segment.second, end_pt);

				cv::LineIterator it(ref_raster.raster_data, st_pt, end_pt, 8);
				for (int i = 0; i < it.count; i++, ++it)
					segment.pinned_pixel.push_back(safe_pixel_read<values_type>(it.pos()));
			}

			/*Polygon structural pixels read assigns a list of pixel values for each constructing ring*/
			//template <typename values_type>
			void readStructrualPixels(Structural_Pinned_Pixels_Boost_Polygon_2<values_type>&c_polygon) {

				// transform spatial polygon position to image discrete position
				Boost_Discrete_Polygon_2 resp_polygon_pixel_coords = affine_transform_geometry<Structural_Pinned_Pixels_Boost_Polygon_2<values_type>, Boost_Discrete_Polygon_2>(
					c_polygon, ref_raster.get_inv_matrix_transformer()
					);

				// get envelop to limit search
				Boost_Discrete_Box_2 envelop;
				boost::geometry::envelope(resp_polygon_pixel_coords, envelop);
				Boost_Discrete_Point_2 min_pixel_corner = envelop.min_corner();
				Boost_Discrete_Point_2 max_pixel_corner = envelop.max_corner();

				for (size_t c_col = min_pixel_corner.get<0>(); c_col <= max_pixel_corner.get<0>(); c_col++) {
					for (size_t c_row = min_pixel_corner.get<1>(); c_row <= max_pixel_corner.get<1>(); c_row++) {

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
									c_polygon.inners_pinned_pixels[c_interior_ring].push_back(safe_pixel_read<values_type>(c_pos));
									is_within_hole = true;
								}
							}
							if (!is_within_hole)
								c_polygon.outer_pinned_pixel.push_back(safe_pixel_read<values_type>(c_pos));
						}
					}
				}

			}


			/*
			Methods below read<>Pixels read pixels covered by each geometry and returns a map of (pixel position) as key and (pixel value) as value

			template <typename values_type>
			pixel_values_container_t<values_type> readPointPixel(Boost_Point_2& resp_point) {
				pixel_values_container_t<values_type> values_container;

				cv::Point pt_pixel_coord;
				ref_raster.get_pixel_coords(resp_point, pt_pixel_coord);
				values_container[pt_pixel_coord] = ref_raster.raster_data.at<values_type>(pt_pixel_coord);
				return values_container;
			}

			template <typename values_type>
			pixel_values_container_t<values_type> readSegmentPixels(Boost_Segment_2& resp_segment) {
				pixel_values_container_t<values_type> values_container;

				cv::Point st_pt, end_pt;
				ref_raster.get_pixel_coords(resp_segment.first, st_pt);
				ref_raster.get_pixel_coords(resp_segment.second, end_pt);

				cv::LineIterator it(ref_raster.raster_data, st_pt, end_pt, 8);
				for (int i = 0; i < it.count; i++, ++it)
					values_container[it.pos()] = ref_raster.raster_data.at<values_type>(it.pos());

				return values_container;

			}

			template <typename values_type>
			pixel_values_container_t<double> readCornerPixels(Boost_Point_2& p_before, Boost_Point_2& p_mid, Boost_Point_2& p_after, bool CCW = false) {
				return readCornerPixels(
					transform_B2C_Point(p_before),
					transform_B2C_Point(p_mid),
					transform_B2C_Point(p_after),
					CCW = CCW);
			}

			template <typename values_type>
			pixel_values_container_t<double> readCornerPixels(Inexact_Point_2& p_before, Inexact_Point_2& p_mid, Inexact_Point_2& p_after, bool CCW = true) {

				bool is_outer_angle = CGAL::left_turn(p_before, p_mid, p_after); if (!CCW) is_outer_angle = !is_outer_angle;

				auto inner_p_before = p_before + is_outer_angle * Inexact_Vector_2(p_mid, p_before);
				auto inner_p_after = p_after + is_outer_angle * Inexact_Vector_2(p_mid, p_after);
				auto inner_mid_point = CGAL::midpoint(inner_p_after, inner_p_before);

				Boost_Polygon_2 corner_polygon;
				GeometryFactoryShared::boost_fill_ring(corner_polygon.outer(),
					transform_C2B_Point(p_mid),
					transform_C2B_Point(inner_p_before),
					transform_C2B_Point(inner_mid_point),
					transform_C2B_Point(inner_p_after),
					transform_C2B_Point(p_mid)
				);

				return readPolygonPixels<values_type>(corner_polygon);
			}

			template <typename values_type>
			pixel_values_container_t<values_type> readPolygonPixels(Boost_Polygon_2& resp_polygon) {

				pixel_values_container_t<values_type> values_container;

				// transform spatial polygon position to image discrete position
				Boost_Discrete_Polygon_2 resp_polygon_pixel_coords = affine_transform_geometry<Boost_Polygon_2, Boost_Discrete_Polygon_2>(
					resp_polygon, ref_raster.get_inv_matrix_transformer()
				);

				// get envelop to limit search
				Boost_Discrete_Box_2 envelop;
				boost::geometry::envelope(resp_polygon_pixel_coords, envelop);
				Boost_Discrete_Point_2 min_pixel_corner = envelop.min_corner();
				Boost_Discrete_Point_2 max_pixel_corner = envelop.max_corner();

				for (size_t c_col = min_pixel_corner.get<0>(); c_col <= max_pixel_corner.get<0>(); c_col++) {
					for (size_t c_row = min_pixel_corner.get<1>(); c_row <= max_pixel_corner.get<1>(); c_row++) {
						cv::Point c_pos( c_col, c_row );
						Boost_Discrete_Point_2 c_p( c_col, c_row );
						if (boost::geometry::within(c_p, resp_polygon_pixel_coords))
							values_container[c_pos] = ref_raster.raster_data.at<values_type>(c_pos);
					}
				}
				return values_container;

			};
			*/

			double readPolygonsPixels(std::list<Boost_Polygon_2>&resp_polygons, RasterPixelsStitcherStartegy strategy);

		public:
			RasterIO ref_raster;

		};

		template <typename values_type>
		class elementary_pixel_reader {

		public:
			RasterPixelsStitcher rps;
			ElementaryStitchOptions options;
			std::function<void(Elementary_Pinned_Pixels_Boost_Point_2<values_type>)> load_functor;

		public:
			// constructor
			elementary_pixel_reader(RasterPixelsStitcher& _rps, ElementaryStitchOptions& _options)
				: rps(_rps), options(_options) {
				switch (options.strategy) {
				case ElementaryStitchStrategy::unique:
					load_functor = read_unique;
					break;
				case ElementaryStitchStrategy::spatial_buffered:
					load_functor = read_spatial_buffered;
					break;
				case ElementaryStitchStrategy::pixel_buffered:
					load_functor = read_pixel_buffered;
					break;
				default:
					throw std::runtime_error("Only non geomtery aware strategies are implemented! (unique | spatial and pixel buffered strategies)");
				}
			};

			//template <typename values_type>
			void read_unique(Elementary_Pinned_Pixels_Boost_Point_2<values_type>& p) {
				cv::Point pt_pixel_coord;
				rps.ref_raster.get_pixel_coords(p, pt_pixel_coord);
				p.pinned_pixel.push_back(rps.safe_pixel_read<values_type>(pt_pixel_coord));
			}

			//template <typename values_type>
			void read_spatial_buffered(Elementary_Pinned_Pixels_Boost_Point_2<values_type>& p) {
				cv::Point min_pixel_corner, max_pixel_corner;
				rps.ref_raster.get_pixel_coords(p - options.spa_left_up_pt, min_pixel_corner);
				rps.ref_raster.get_pixel_coords(p + options.spa_right_down_pt, max_pixel_corner);
				p.pinned_pixel.splice(p.pinned_pixel.end(), rps.readBoxPixels(min_pixel_corner.x, min_pixel_corner.y, max_pixel_corner.x, max_pixel_corner.y));
			}

			//template <typename values_type>
			void read_pixel_buffered(Elementary_Pinned_Pixels_Boost_Point_2<values_type>& p) {
				cv::Point min_pixel_corner, max_pixel_corner;
				rps.ref_raster.get_pixel_coords(p, min_pixel_corner);
				rps.ref_raster.get_pixel_coords(p, max_pixel_corner);
				p.pinned_pixel.splice(p.pinned_pixel.end(),
					rps.readBoxPixels(
						min_pixel_corner.x - options.pix_left_up_pt.get<0>(), min_pixel_corner.y - options.pix_left_up_pt.get<1>(),
						max_pixel_corner.x + options.pix_right_down_pt.get<0>(), max_pixel_corner.y + options.pix_right_down_pt.get<1>()
					)
				);
			}
			
			std::list<values_type> readCornerPixels(Inexact_Point_2& p_before, Inexact_Point_2& p_mid, Inexact_Point_2& p_after, bool CCW = true) {

				bool is_outer_angle = CGAL::left_turn(p_before, p_mid, p_after); if (!CCW) is_outer_angle = !is_outer_angle;

				auto inner_p_before = p_before + is_outer_angle * Inexact_Vector_2(p_mid, p_before);
				auto inner_p_after = p_after + is_outer_angle * Inexact_Vector_2(p_mid, p_after);
				auto inner_mid_point = CGAL::midpoint(inner_p_after, inner_p_before);

				Structural_Pinned_Pixels_Boost_Polygon_2 corner_polygon;
				GeometryFactoryShared::boost_fill_ring(corner_polygon.outer(),
					transform_C2B_Point(p_mid),
					transform_C2B_Point(inner_p_before),
					transform_C2B_Point(inner_mid_point),
					transform_C2B_Point(inner_p_after),
					transform_C2B_Point(p_mid)
				);

				return rps.readStructrualPixels(corner_polygon);
			}

			//template <typename values_type>
			void read_pixel_corner_area(Elementary_Pinned_Pixels_Boost_Point_2<values_type>& p_mid, const Boost_Point_2& p_before, const Boost_Point_2& p_after, bool CCW = false) {
				p_mid.pinned_pixel.splice(p_mid.pinned_pixel.end(), readCornerPixels(
					transform_B2C_Point(p_before),
					transform_B2C_Point(p_mid),
					transform_B2C_Point(p_after),
					CCW = CCW)
				);
			}



			template <typename values_type>
			void operator()(Elementary_Pinned_Pixels_Boost_Point_2<values_type>& p)
			{
				load_functor(p);
			}

		};

	
	}
}