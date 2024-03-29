#include "defs.h"
#include "geometry_lab.h"
#include "design_pattern/extended_iterators.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		std::vector<Inexact_Point_2> simplify_aberrant_polygon(const std::vector<Inexact_Point_2>& polygon_ring) {

			// copy open polygon ring
			std::vector<Inexact_Point_2> exterior_coords(polygon_ring.begin(),
														polygon_ring.begin()+polygon_ring.size()-1);

			std::list<Inexact_Point_2> simplified_R;
			int turn_m2_idx = exterior_coords.size()-2;
			int turn_m1_idx = exterior_coords.size()-1;
			int turn_m0_idx = 0;

			while (simplified_R.size() < exterior_coords.size()) {

				if (!pts_collinear_2(exterior_coords[turn_m0_idx], exterior_coords[turn_m1_idx], exterior_coords[turn_m2_idx])) {

					simplified_R.push_back(exterior_coords[turn_m1_idx]);
					turn_m2_idx = turn_m1_idx;
					turn_m1_idx = turn_m0_idx;
					turn_m0_idx = mod(turn_m0_idx + 1, exterior_coords.size());
				}
				else {
					exterior_coords.erase(exterior_coords.begin()+ turn_m1_idx);
					if (!simplified_R.empty()) simplified_R.pop_back();
					if (exterior_coords.size() == 0)
						continue;
					turn_m0_idx = mod(turn_m0_idx - 1, exterior_coords.size());
					turn_m1_idx = mod(turn_m0_idx - 1, exterior_coords.size());
					turn_m2_idx = mod(turn_m1_idx - 1, exterior_coords.size());
				}

			}
			if (simplified_R.size() > 0) simplified_R.push_back(simplified_R.front());
			return std::vector<Inexact_Point_2>(simplified_R.begin(), simplified_R.end());
		}

		std::list<Inexact_Polygon_2> explose_self_intersecting_polygon(const Inexact_Polygon_2& polygon_ring) {
			std::vector<Inexact_Point_2> coordinates_ring(polygon_ring.begin(), polygon_ring.end());
			auto multiple_polygons_coordinates = explose_self_intersecting_polygon(coordinates_ring);
			std::list<Inexact_Polygon_2> multiple_polygons;
			for (auto& c_coordinates_ring : multiple_polygons_coordinates) {
				Inexact_Polygon_2 c_polygon(c_coordinates_ring.begin(), c_coordinates_ring.end());
				multiple_polygons.push_back(c_polygon);
			}
			return multiple_polygons;
		}

		std::list<std::vector<Inexact_Point_2>> explose_self_intersecting_polygon(const std::vector<Inexact_Point_2>& polygon_ring) {

			// copy open ring points
			if (polygon_ring.size() == 0) {
				BOOST_LOG_TRIVIAL(debug) << "empty ring to explode";
				return std::list<std::vector<Inexact_Point_2>>();
			}
			std::vector<Inexact_Point_2> ring_pts(polygon_ring.begin(),
				polygon_ring.begin() + polygon_ring.size() - 1);

			std::set<size_t> lines_to_disconnect_start_pt_indices;
			// step 1 : checking segment points to disconnect
			for (size_t seg_1_idx = 0; seg_1_idx < ring_pts.size() - 1; ++seg_1_idx) {

				for (size_t seg_2_idx = 0; seg_2_idx < ring_pts.size() - 1; ++seg_2_idx) {

					if (seg_1_idx == seg_2_idx) continue;

					double max_dist = max_distance_between_lines(ring_pts[seg_1_idx], ring_pts[seg_1_idx + 1],
						ring_pts[seg_2_idx], ring_pts[seg_2_idx + 1]);

					if (max_dist < 1e-3) {
						lines_to_disconnect_start_pt_indices.insert(seg_1_idx);
						lines_to_disconnect_start_pt_indices.insert(seg_2_idx);
					}
				}
			}

			// step 2 : creating multipart polygons rings
			std::list<std::vector<Inexact_Point_2>> exploded_polygons_pts;

			if (lines_to_disconnect_start_pt_indices.size() == 0) {
				exploded_polygons_pts.push_back(polygon_ring);
				return exploded_polygons_pts;
			}

			for (size_t c_disconnect : lines_to_disconnect_start_pt_indices) {

				std::vector<Inexact_Point_2> c_polygon_pts;

				//start pt definition
				size_t c_pt_idx = c_disconnect + 1;
				c_polygon_pts.push_back(ring_pts[c_pt_idx]);
				size_t next_pt_idx = mod(c_pt_idx + 1, ring_pts.size());
				while (lines_to_disconnect_start_pt_indices.find(next_pt_idx) == lines_to_disconnect_start_pt_indices.end()) {
					c_polygon_pts.push_back(ring_pts[next_pt_idx]);
					next_pt_idx = mod(next_pt_idx + 1, ring_pts.size());
				}
				c_polygon_pts.push_back(ring_pts[next_pt_idx]);
				c_polygon_pts.push_back(c_polygon_pts[0]);

				if (c_polygon_pts.size() > 2) exploded_polygons_pts.push_back(c_polygon_pts);
			}
			return exploded_polygons_pts;
		}

		double max_distance_between_lines(const Inexact_Point_2& p1,
			const Inexact_Point_2& p2,
			const Inexact_Point_2& p3,
			const Inexact_Point_2& p4) {

			double p3_dist = CGAL::squared_distance(Inexact_Line_2(p1, p2), p3);
			double p4_dist = CGAL::squared_distance(Inexact_Line_2(p1, p2), p4);
			return std::max<double>(p3_dist, p4_dist);
		}

		OGRGeometry* BuildMultiLine(OGRGeometry* geometry)
		{
			OGRPolygon* poly = dynamic_cast<OGRPolygon*>(geometry);
			OGRLinearRing* ring = poly->getExteriorRing();

			Inexact_Polygon_2 skeleton;
			for (int i = 0; i < ring->getNumPoints() - 1; i++) {
				skeleton.push_back(Inexact_Point_2(ring->getX(i), ring->getY(i)));
			}
			bool polygon_is_simple = skeleton.is_simple();
			//CGAL::write_polygon_WKT(std::cout << std::setprecision(15), skeleton);
			if (!polygon_is_simple || skeleton.is_counterclockwise_oriented() == 0) {
				skeleton.reverse_orientation(); 
			}
			//std::cout << "Skeletonizing." << std::endl;
			boost::shared_ptr<Inexact_Straight_Skeleton_2> iss = CGAL::create_interior_straight_skeleton_2(skeleton);
			OGRLineString* line = NULL;
			if (iss == NULL) {
				CGAL::write_polygon_WKT(std::cout << std::setprecision(15), skeleton);
			}				
			//std::cout << "Computed Skeleton." << std::endl;
			// And finally append points to our shapefile
			double edge = 0;
			for (Inexact_Straight_Skeleton_2::Halfedge_iterator vi = iss->halfedges_begin(); vi != iss->halfedges_end(); ++vi) {
				OGRPoint point = OGRPoint(vi->vertex()->point().x(), vi->vertex()->point().y());
				OGRPoint npoint = OGRPoint(vi->next()->vertex()->point().x(), vi->next()->vertex()->point().y());
				OGRLineString segment = OGRLineString();
				segment.addPoint(&point);
				segment.addPoint(&npoint);
				if (line == NULL) { line = new OGRLineString; }
				OGRLineString* tmp;
				++edge;
				if (vi->vertex()->is_skeleton() && vi->next()->vertex()->is_skeleton() && segment.Within(poly)) {
					tmp = reinterpret_cast<OGRLineString*>(line->Union(&segment));
					if (tmp != NULL) {
						//std::cout << (int)(edge / (double)iss->size_of_halfedges() * 100.0) << "% ";
						//std::cout.flush();
						delete line;
						line = tmp;
					}
				}
			}
			OGRGeometryFactory::destroyGeometry(poly);
			return line;
		}

	}
}