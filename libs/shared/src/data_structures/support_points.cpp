#include "data_structures/support_points.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		SupportPoints decompose_polygons(std::vector<Boost_Polygon_2>& input_polygons, const SupportPointsGenOptions& supp_p_options) {

			SupportPoints out_support_points(supp_p_options.strategy);

			if (out_support_points.strategy == SupportPointsStrategy::vertex_and_mid_point) {
				// reserve vectors memory
				out_support_points.parents_indices.reserve(input_polygons.size() * Constants::MEAN_PTS_PER_POLYGON * 2);
				out_support_points.children.reserve(input_polygons.size() * Constants::MEAN_PTS_PER_POLYGON * 2);

				size_t c_polygon_idx = 0;
				for (auto& c_polygon : input_polygons) {
					std::list<Boost_Ring_2*> c_polygon_rings;
					c_polygon_rings.push_back(&c_polygon.outer());
					//for (Boost_Ring_2& c_inner_ring : c_polygon.inners()) c_polygon_rings.push_back(&c_inner_ring);

					for (auto c_ring : c_polygon_rings) {

						for (size_t c_pt_idx = 0; c_pt_idx < c_ring->size() - 1; ++c_pt_idx) {
							Boost_Point_2& edge_start = c_ring->at(c_pt_idx);
							Boost_Point_2& edge_end = c_ring->at(c_pt_idx + 1);
							Boost_Point_2 edge_mid(edge_start.get<0>() + (edge_end.get<0>() - edge_start.get<0>()) / 2.0f,
								edge_start.get<1>() + (edge_end.get<1>() - edge_start.get<1>()) / 2.0f);
							out_support_points.parents_indices.insert(out_support_points.parents_indices.end(), 3, c_polygon_idx);
							out_support_points.children.insert(out_support_points.children.end(), { edge_start, edge_mid, edge_end });
						}
					}
					c_polygon_idx++;
				}
				out_support_points.parents_indices.shrink_to_fit(); out_support_points.children.shrink_to_fit();
				out_support_points.parent_count = c_polygon_idx - 1;
				return out_support_points;
			}
			else if ((supp_p_options.strategy == SupportPointsStrategy::constant_walker)) {
				// reserve vectors memory
				out_support_points.parents_indices.reserve(Constants::MEAN_PTS_PER_POLYGON * 10);
				out_support_points.children.reserve(Constants::MEAN_PTS_PER_POLYGON * 10);

				size_t c_polygon_idx = 0;
				for (auto& c_polygon : input_polygons) {
					std::list<Boost_Ring_2*> c_polygon_rings;
					c_polygon_rings.push_back(&c_polygon.outer());
					//for (Boost_Ring_2& c_inner_ring : c_polygon.inners()) c_polygon_rings.push_back(&c_inner_ring);

					for (auto& c_ring : c_polygon_rings) {

						Boost_LineString_2 c_linestring(c_ring->begin(), c_ring->end());
						Boost_MultiPoint_2 densified_ring;
						bg::line_interpolate(c_linestring, supp_p_options.STEP_LENGTH, densified_ring);
						out_support_points.parents_indices.insert(out_support_points.parents_indices.end(), densified_ring.size(), c_polygon_idx);
						out_support_points.children.insert(out_support_points.children.end(), densified_ring.begin(), densified_ring.end());

					}
					c_polygon_idx++;
				}
				out_support_points.parents_indices.shrink_to_fit(); out_support_points.children.shrink_to_fit();
				out_support_points.parent_count = c_polygon_idx - 1;
				return out_support_points;

			}
			else if (supp_p_options.strategy == SupportPointsStrategy::vertex_only) {
				// reserve vectors memory
				out_support_points.parents_indices.reserve(Constants::MEAN_PTS_PER_POLYGON);
				out_support_points.children.reserve(Constants::MEAN_PTS_PER_POLYGON);
				size_t c_polygon_idx = 0;
				for (auto& c_polygon : input_polygons) {
					std::list<Boost_Ring_2*> c_polygon_rings;
					c_polygon_rings.push_back(&c_polygon.outer());
					//for (Boost_Ring_2& c_inner_ring : c_polygon.inners()) c_polygon_rings.push_back(&c_inner_ring);

					for (auto& c_ring : c_polygon_rings) {
						out_support_points.parents_indices.insert(out_support_points.parents_indices.end(), c_ring->size(), c_polygon_idx);
						out_support_points.children.insert(out_support_points.children.end(), c_ring->begin(), c_ring->end());

					}
					c_polygon_idx++;
				}
				out_support_points.parents_indices.shrink_to_fit(); out_support_points.children.shrink_to_fit();
				out_support_points.parent_count = c_polygon_idx - 1;
				return out_support_points;

			}
			else {
				std::cout << "Only vertex_only & vertex_and_mid_point & constant_walker are implemented!" << std::endl;
				throw std::exception("Only vertex_only & vertex_and_mid_point & constant_walker are implemented!");
			}

		}
	}
}