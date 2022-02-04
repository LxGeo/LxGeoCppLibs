#include "polygonizer_graph.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		void PolygonizerGraph::add_line_string(const Boost_LineString_2& line_to_add) {

			vertex_descriptor p1_vertex, p2_vertex;
			Boost_Point_2 p1 = line_to_add.at(0);
			Boost_Point_2 p2 = line_to_add.at(1);
			bool add_p1 = true, add_p2 = true;
			
			vertex_iterator v, vend;
			boost::tie(v, vend) = boost::vertices(PG);
			while (  (add_p1 | add_p2 ) & (v != vend) ) {
				Boost_Point_2 c_point = PG[*v].p;
				if (add_p1 & (bg::distance(c_point, p1) < point_space_tolerance)) {
					add_p1 = false;
					p1_vertex = *v;
					continue;
				}
				if (add_p2 & (bg::distance(c_point, p2) < point_space_tolerance)) {
					add_p2 = false;
					p2_vertex = *v;
				}
				++v;
			}

			if (add_p1) {
				p1_vertex = boost::add_vertex(PG);
				PG[p1_vertex].p = p1;
			}
			if (add_p2) {
				p2_vertex = boost::add_vertex(PG);
				PG[p2_vertex].p = p2;
			}

			boost::add_edge(p1_vertex, p2_vertex, PG);

		}

		std::list<std::vector<Boost_Point_2>> PolygonizerGraph::construct_rings() {

			std::list<std::vector<Boost_Point_2>> all_rings;
			return all_rings;
			std::vector<Boost_Point_2> c_ring;
			vertex_descriptor c_vertex = *(boost::vertices(PG).first);

		}

	}
}
