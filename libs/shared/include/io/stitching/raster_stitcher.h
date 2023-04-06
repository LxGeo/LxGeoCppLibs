#pragma once
#include "defs.h"
#include "io_raster.h"
#include "affine_geometry/affine_transformer.h"
#include "stitchable_geometries/def_stitched_geoms.h"
#include "stitchable_geometries/stitching_options.h"
#include "design_pattern/extended_iterators.h"

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


		template <typename values_type>
		class RasterPixelsStitcher {

		public:
			RasterPixelsStitcher() {};
			RasterPixelsStitcher(RasterIO& _ref_raster) { ref_raster = _ref_raster; };

			/*Basic read operation*/
			std::list<values_type> readBoxPixels(const Boost_Discrete_Box_2& box) {
				return readBoxPixels<values_type>(
					box.min_corner().get<0>(), box.min_corner().get<1>(),
					box.max_corner().get<0>(), box.max_corner().get<1>()
				);
			}

			std::list<values_type> readBoxPixels(int& min_x, int& min_y, int& max_x, int& max_y) {

				std::list<values_type> out_list;

				for (size_t c_col = min_x; c_col > max_x; c_col++) {
					for (size_t c_row = min_y; c_row > max_y; c_row++) {
						cv::Point c_pt_pixel_coord(c_col, c_row);
						out_list.push_back(safe_pixel_read<values_type>(c_pt_pixel_coord));
					}
				}
				return out_list;
			}

			/*Nullable pixel read for out of bounds position*/
			values_type safe_pixel_read(const cv::Point & c_p) {
				if ((c_p.x < 0) || (c_p.x >= ref_raster.raster_data.cols))
					return NULL;
				if ((c_p.y < 0) || (c_p.y >= ref_raster.raster_data.rows))
					return NULL;
				return ref_raster.raster_data.at<values_type>(c_p);
			}

			/*Read elementary pixels applicable for all boost geoemetries (non shape aware)*/
			/*template <typename Input>
			void readElementaryPixels(Input & geometry_in, ElementaryStitchOptions & options) {
				// check if geometry passed
				boost::geometry::concepts::check<Input const>();
				// check if object has pixel values container
				//
				boost::geometry::for_each_point(geometry_in, elementary_pixel_reader<values_type>(this, options));
			}*/

			/*** geometry aware elementary pixels read && can accept corner_area option***/
			//template <typename values_type>
			void readElementaryPixels(Elementary_Pinned_Pixels_Boost_Polygon_2<values_type>&c_polygon, const ElementaryStitchOptions & options) {


				auto epr = elementary_pixel_reader<values_type>(*this, options);

				auto ring_pts_geometry_aware_pixel_reader = [&](auto& c_ring, bool is_outer_ring=true)
				{
					// rings are closed so ignoring last element
					auto& st = c_ring.begin();
					auto& end = std::prev(c_ring.end());
					circularIterator<Elementary_Pinned_Pixels_Boost_Point_2<values_type>, std::vector> circular_ring(st, end, c_ring.size()-1);
					for (auto& mid_pt_iter = c_ring.begin(); mid_pt_iter != end; mid_pt_iter++) {
						auto pts_window = circular_ring.getWindow(mid_pt_iter, 1, 1);
						epr.readCornerPixels(pts_window[1], pts_window[0], pts_window[2], is_outer_ring, options);
					}
				};

				ring_pts_geometry_aware_pixel_reader(c_polygon.outer());
				for(size_t inner_ring_idx=0; inner_ring_idx<c_polygon.inners().size(); inner_ring_idx++)
					ring_pts_geometry_aware_pixel_reader(c_polygon.inners()[inner_ring_idx], false);

			}

			/***Methods below read structural pixels values***/

			/*Segment structural pixels read assigns a list of pixel values for the unique segment*/
			void readStructrualPixels(Structural_Pinned_Pixels_Boost_Segment_2<values_type>&segment) {
				cv::Point st_pt, end_pt;
				ref_raster.get_pixel_coords(segment.first, st_pt);
				ref_raster.get_pixel_coords(segment.second, end_pt);

				cv::LineIterator it(ref_raster.raster_data, st_pt, end_pt, 8);
				for (int i = 0; i < it.count; i++, ++it)
					segment.pinned_pixel.push_back(safe_pixel_read<values_type>(it.pos()));
			}

			/*Polygon structural pixels read assigns a list of pixel values for each constructing ring*/
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
									c_polygon.inners_pinned_pixels[c_ring_idx].push_back(safe_pixel_read(c_pos));
									is_within_hole = true;
									break;
								}
							}
							if (!is_within_hole)
								c_polygon.outer_pinned_pixel.push_back(safe_pixel_read(c_pos));
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
			

		public:
			RasterIO ref_raster;

		};

		template <typename values_type>
		class elementary_pixel_reader {
			
			typedef RasterPixelsStitcher<values_type> RasterPixelsStitcher;

		public:
			RasterPixelsStitcher& rps;
			ElementaryStitchOptions& options;
			std::function<void(Elementary_Pinned_Pixels_Boost_Point_2<values_type>&)> load_functor;

		public:
			// constructor
			elementary_pixel_reader(RasterPixelsStitcher& _rps, ElementaryStitchOptions& _options)
				: rps(_rps), options(_options) {
				/*switch (options.strategy) {
				case ElementaryStitchStrategy::unique:
					load_functor = *read_unique;
					break;
				case ElementaryStitchStrategy::spatial_buffered:
					load_functor = read_spatial_buffered;
					break;
				case ElementaryStitchStrategy::pixel_buffered:
					load_functor = read_pixel_buffered;
					break;
				case ElementaryStitchStrategy::corner_area:
					load_functor = NULL;
					break;
				default:
					throw std::runtime_error("Choose strategy from ElementaryStitchOptions!");
				}*/
			};

			void read_unique(Elementary_Pinned_Pixels_Boost_Point_2<values_type>& p) {
				cv::Point pt_pixel_coord;
				rps.ref_raster.get_pixel_coords(p, pt_pixel_coord);
				p.pinned_pixel.push_back(rps.safe_pixel_read<values_type>(pt_pixel_coord));
			}

			void read_spatial_buffered(Elementary_Pinned_Pixels_Boost_Point_2<values_type>& p) {
				cv::Point min_pixel_corner, max_pixel_corner;
				rps.ref_raster.get_pixel_coords(p - options.spa_left_up_pt, min_pixel_corner);
				rps.ref_raster.get_pixel_coords(p + options.spa_right_down_pt, max_pixel_corner);
				p.pinned_pixel.splice(p.pinned_pixel.end(), rps.readBoxPixels(min_pixel_corner.x, min_pixel_corner.y, max_pixel_corner.x, max_pixel_corner.y));
			}

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
			
			std::list<values_type> readCornerPixels(const Inexact_Point_2& p_before, const Inexact_Point_2& p_mid, const Inexact_Point_2& p_after, bool CCW, ElementaryStitchOptions& options) {

				bool is_outer_angle = CGAL::left_turn(p_before, p_mid, p_after);
				if (!CCW) is_outer_angle = !is_outer_angle;

				// limiting inner search
				auto pt_before_limit_step_vector = Inexact_Vector_2(p_mid, p_before);
				pt_before_limit_step_vector /= vmax(
					vmax(1, abs(pt_before_limit_step_vector.x())/ options.spa_left_up_pt.get<0>()),
					vmax(1, abs(pt_before_limit_step_vector.y())/ options.spa_left_up_pt.get<1>())
					);
				auto pt_after_limit_step_vector = Inexact_Vector_2(p_mid, p_after);
				pt_after_limit_step_vector /= vmax(
					vmax(1, abs(pt_after_limit_step_vector.x())/ options.spa_left_up_pt.get<0>()),
					vmax(1, abs(pt_after_limit_step_vector.y())/ options.spa_left_up_pt.get<1>())
					);

				//continue here
				Inexact_Vector_2 extremities_vector(p_mid + pt_before_limit_step_vector, p_mid + pt_after_limit_step_vector);
				auto centripetal_vector = extremities_vector.perpendicular(CGAL::Orientation::CLOCKWISE);
				Inexact_Point_2 inner_mid_point = p_mid + centripetal_vector;
				/*
				int search_sign = is_outer_angle ? -1 : 1;
				auto inner_mid_point = p_mid + search_sign*(pt_before_limit_step_vector + pt_after_limit_step_vector);
				*/
				Structural_Pinned_Pixels_Boost_Polygon_2<values_type> corner_polygon;
				bg::append(corner_polygon.outer(), transform_C2B_Point(p_mid));
				bg::append(corner_polygon.outer(), transform_C2B_Point(p_mid + pt_before_limit_step_vector));
				bg::append(corner_polygon.outer(), transform_C2B_Point(inner_mid_point));
				bg::append(corner_polygon.outer(), transform_C2B_Point(p_mid + pt_after_limit_step_vector));
				bg::append(corner_polygon.outer(), transform_C2B_Point(p_mid));

				//std::cout << std::setprecision(12) << bg::wkt(corner_polygon) << std::endl;
				
				rps.readStructrualPixels(corner_polygon);
				return corner_polygon.outer_pinned_pixel;
			}

			void readCornerPixels(Elementary_Pinned_Pixels_Boost_Point_2<values_type>& p_mid,
				const Boost_Point_2& p_before, const Boost_Point_2& p_after, bool CCW, 
				ElementaryStitchOptions& options) {
				p_mid.pinned_pixel.splice(p_mid.pinned_pixel.end(), readCornerPixels(
					transform_B2C_Point(p_before),
					transform_B2C_Point(p_mid),
					transform_B2C_Point(p_after),
					CCW,
					options)
				);
			}

			template <typename values_type>
			void operator()(const Elementary_Pinned_Pixels_Boost_Point_2<values_type>& p)
			{
				//load_functor(p);
				switch (options.strategy) {
				case ElementaryStitchStrategy::unique:
					load_functor = *read_unique;
					break;
				case ElementaryStitchStrategy::spatial_buffered:
					load_functor = read_spatial_buffered;
					break;
				case ElementaryStitchStrategy::pixel_buffered:
					load_functor = read_pixel_buffered;
					break;
				default:
					throw std::runtime_error("Choose strategy from ElementaryStitchOptions!");
				}
			}

		};

	
	}
}