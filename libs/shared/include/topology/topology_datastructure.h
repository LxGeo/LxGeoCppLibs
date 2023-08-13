#pragma once
#include "defs.h"
#include <CGAL/Cartesian.h>
#include <CGAL/Arr_non_caching_segment_traits_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include "geometry_lab.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		typedef CGAL::Cartesian<double> Kernel;
		typedef CGAL::Arr_segment_traits_2<EK> Traits;
		typedef Traits::Point_2 Point;
		typedef Traits::X_monotone_curve_2 Segment;
		typedef CGAL::Arrangement_2<Traits> Arrangement;

		template <std::ranges::input_range Range>
		Arrangement ArrangmentFromPolygons(const Range& gwas_container) {
			Arrangement arr;
			std::list<Segment> segments_to_add;
			for (const auto& c_gwa : gwas_container) {
				auto& c_polygon = c_gwa->get_definition();
				auto fixed_polygon = simplify_aberrant_polygon(c_polygon);
				if (!bg::is_valid(c_polygon))
					continue;
					//throw std::runtime_error("Unvalid geometry when creating arrangment!");
				auto c_polygon_vertex_it = fixed_polygon.outer().begin();
				auto prev = c_polygon_vertex_it;
				for (++c_polygon_vertex_it; c_polygon_vertex_it != fixed_polygon.outer().end(); ++c_polygon_vertex_it) {
					Segment c_edge(Point(prev->get<0>(), prev->get<1>()), Point(c_polygon_vertex_it->get<0>(), c_polygon_vertex_it->get<1>()));
					//std::cout << std::setprecision(15) << "linestring( " << c_edge.source().x() << " " << c_edge.source().y() << ", " << c_edge.target().x() << " " << c_edge.target().y() << ")" << std::endl;
					try {
						CGAL::insert(arr, c_edge);
					}
					catch (...){
						std::cout << std::setprecision(20) << "linestring( " << c_edge.source().x() << " " << c_edge.source().y() << ", " << c_edge.target().x() << " " << c_edge.target().y() << ")" << std::endl;
					}
					prev = c_polygon_vertex_it;
				}
			}
			//CGAL::insert(arr, segments_to_add.begin(), segments_to_add.end());
			return arr;
		}

		template <std::ranges::input_range Range>
		Arrangement ArrangmentFromLineStrings(const Range& gwas_container) {
			Arrangement arr;
			for (const auto& c_gwa : gwas_container) {
				auto& c_linestring = c_gwa.get_definition();
				auto c_line_vertex_it = c_linestring.begin();
				auto prev = c_line_vertex_it;
				for (++c_line_vertex_it; c_line_vertex_it != c_linestring.end(); ++c_line_vertex_it) {
					Segment c_edge(Point(prev->get<0>(), prev->get<1>()), Point(c_line_vertex_it->get<0>(), c_line_vertex_it->get<1>()));					
					CGAL::insert(arr, c_edge);					
					prev = c_line_vertex_it;
				}
			}
			return arr;
		}
	}
}