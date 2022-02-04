#pragma once

#include "defs.h"
#include "export_shared.h"
#include "polygonizer_graph_wrapper.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		
		class PolygonizerGraph {

		public:
			PolygonizerGraph(){}
			void add_line_string(const Boost_LineString_2& line_to_add);
			void add_line_string(const std::vector<Boost_LineString_2>& lines_to_add) {
				for (Boost_LineString_2 c_line : lines_to_add) {
					add_line_string(c_line);
				}
			}
			std::list<std::vector<Boost_Point_2>> construct_rings();


		public:
			BoostPolygonizerGraph PG;
			const double point_space_tolerance = 1e-3;

		};

	}
}