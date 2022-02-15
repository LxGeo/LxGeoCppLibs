#include "defs.h"
#include "defs_cgal.h"
#include "defs_boost.h"
#include "defs_common.h"
#include "export_shared.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		int mod(int a, int b) {
			int c = a % b;
			return (c < 0) ? c + b : c;
		}
		
		Inexact_Point_2 transform_B2C_Point(const Boost_Point_2& boost_point) {
			return Inexact_Point_2(bg::get<0>(boost_point), bg::get<1>(boost_point));
		};

		Boost_Point_2 transform_C2B_Point(Inexact_Point_2& cgal_point) {
			return Boost_Point_2(cgal_point.x(), cgal_point.y());
		};

		void container_transform_B2C_Points(std::vector<Boost_Point_2>& input_container, std::vector<Inexact_Point_2>& output_container) {
			assert(output_container.empty());
			output_container.reserve(input_container.size());

			for (Boost_Point_2 c_point : input_container) {
				Inexact_Point_2 cgal_point = transform_B2C_Point(c_point);
				output_container.push_back(cgal_point);
			}

		};

		void container_transform_OGRRING2vector_Points(OGRLinearRing* input_ring, std::vector<Inexact_Point_2>& output_vector) {

			output_vector.reserve(input_ring->getNumPoints());

			for (size_t c_point_idx = 0; c_point_idx < input_ring->getNumPoints(); ++c_point_idx) {
				output_vector.push_back(Inexact_Point_2(input_ring->getX(c_point_idx), input_ring->getY(c_point_idx)));
			}
		}

		std::vector<Inexact_Point_2> container_transform_OGRRING2vector_Points(OGRLinearRing* input_ring) {

			std::vector<Inexact_Point_2> aux_vector;
			container_transform_OGRRING2vector_Points(input_ring, aux_vector);
			return aux_vector;
		}

		OGRLinearRing container_transform_vector_Points2OGRRING(const std::vector<Inexact_Point_2>& input_vector) {
			
			OGRLinearRing aux_ogr_ring;
			for (const Inexact_Point_2& c_point : input_vector) aux_ogr_ring.addPoint(&OGRPoint(c_point.x(), c_point.y()));
			return aux_ogr_ring;
		}

		Boost_Polygon_2 transform_OGR2B_Polygon(OGRPolygon* ogr_polygon) {
			Boost_Polygon_2 aux_boost_polygon;

			// fill exterior
			OGRLinearRing* exterior_ring = ogr_polygon->getExteriorRing();
			for (size_t v_idx = 0; v_idx < exterior_ring->getNumPoints(); ++v_idx) {
				OGRPoint pt;
				exterior_ring->getPoint(v_idx, &pt);
				double x = pt.getX(), y = pt.getY();
				aux_boost_polygon.outer().push_back(Boost_Point_2(x, y));
			}
			// fill interior
			for (size_t int_ring_idx = 0; int_ring_idx < ogr_polygon->getNumInteriorRings(); ++int_ring_idx) {
				OGRLinearRing* c_int_ring = ogr_polygon->getInteriorRing(int_ring_idx);
				Boost_Ring_2 R;
				int ring_size = c_int_ring->getNumPoints();
				R.reserve(ring_size);
				for (int u = 0; u < ring_size; ++u) { //-1
					OGRPoint pt;
					c_int_ring->getPoint(u, &pt);
					double x = pt.getX(), y = pt.getY();
					//CT->Transform(1, &x, &y);
					R.push_back(Boost_Point_2(x, y));
				}
				aux_boost_polygon.inners().push_back(R);
			}
			return aux_boost_polygon;

		}

		double angle3p(const Boost_Point_2& p_m, const Boost_Point_2& p_0, const Boost_Point_2& p_1) {
			
			double m0x = p_0.get<0>() - p_m.get<0>(), m0y = p_0.get<1>() - p_m.get<1>(),
				m1x = p_1.get<0>() - p_m.get<0>(), m1y = p_1.get<1>() - p_m.get<1>();

			double angleRadians = atan2(m0x * m1x + m0y * m1y, m0y * m1x - m0x * m1y);
			angleRadians = std::fmod(angleRadians + M_PI, M_PI);
			return angleRadians;
		}

		Boost_Box_2 box_buffer(Boost_Box_2& in_box, double buff) {
			Boost_Box_2 out_box;
			out_box.min_corner().set<0>(in_box.min_corner().get<0>() - buff);
			out_box.min_corner().set<1>(in_box.min_corner().get<1>() - buff);
			out_box.max_corner().set<0>(in_box.max_corner().get<0>() + buff);
			out_box.max_corner().set<1>(in_box.max_corner().get<1>() + buff);
			return out_box;
		}

		OGRPoint transform_B2OGR_Point(Boost_Point_2& in_point) {
			return OGRPoint(in_point.get<0>(), in_point.get<1>());
		}

		OGRLinearRing transform_B2OGR_Ring(Boost_Ring_2& in_ring) {
			OGRLinearRing ogr_ring;
			for (Boost_Point_2& c_pt : in_ring) {
				ogr_ring.addPoint(&transform_B2OGR_Point(c_pt));
			}
			return ogr_ring;
		}

		OGRPolygon transform_B2OGR_Polygon(Boost_Polygon_2& in_polygon) {

			OGRPolygon out_polygon;						

			if (in_polygon.outer().empty()) return out_polygon;

			OGRLinearRing ogr_ext_ring = transform_B2OGR_Ring(in_polygon.outer());

			std::list<OGRLinearRing> ogr_int_rings;
			for (auto& c_int_ring : in_polygon.inners()) {
				ogr_int_rings.push_back(transform_B2OGR_Ring(c_int_ring));
			}

			out_polygon.addRing(&ogr_ext_ring);
			for (OGRLinearRing ring : ogr_int_rings) out_polygon.addRing(&ring);
		}

		OGRLineString transform_B2OGR_LineString(Boost_LineString_2& in_linestring) {
			OGRLineString out_linestring;
			for (auto& c_pt : in_linestring) out_linestring.addPoint(&transform_B2OGR_Point(c_pt));
			return out_linestring;
		}

	}
}

int mod(int a, int b)
{
	return 0;
}
