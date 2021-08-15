#include "defs.h"
#include "defs_cgal.h"
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

	}
}

int mod(int a, int b)
{
	return 0;
}
