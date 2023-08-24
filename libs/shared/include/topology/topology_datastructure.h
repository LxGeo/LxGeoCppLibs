#pragma once
#include "defs.h"
#include <CGAL/Cartesian.h>
#include <CGAL/Arr_non_caching_segment_traits_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include "geometry_lab.h"

#include <CGAL/Arr_polyline_traits_2.h>
#include <CGAL/Arr_curve_data_traits_2.h>
#include <CGAL/Arr_consolidated_curve_data_traits_2.h>
#include <CGAL/Arr_landmarks_point_location.h>
#include <CGAL/Arr_observer.h>

namespace LxGeo
{
    using namespace IO_DATA;
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


		template <typename kernel>
		class LinearTopology {

        public:
			struct SegmentIdentification {
				size_t parent_id;
				float position;

				SegmentIdentification() {
					parent_id = -1;
					position = INFINITY;
				}

				SegmentIdentification(const SegmentIdentification& other) {
					parent_id = other.parent_id;
					position = other.position;
				}

				bool operator==(const SegmentIdentification& lhs) const
				{
					return (lhs.parent_id == parent_id) && (lhs.position == position);
				};
			};

			typedef CGAL::Arr_segment_traits_2<kernel>                Segment_traits_2;
			typedef Segment_traits_2::Curve_2                         Segment_2;
			typedef CGAL::Arr_consolidated_curve_data_traits_2
				<Segment_traits_2, SegmentIdentification>             Traits_2;
			typedef Traits_2::Point_2                                 Point_2;
			typedef Traits_2::Curve_2                                 Identifiable_segment_2;
			typedef CGAL::Arrangement_2<Traits_2>                     Arrangement_2;
			typedef CGAL::Arr_landmarks_point_location<Arrangement_2> Landmarks_pl;
            typedef CGAL::Arrangement_2<Traits_2>::Vertex_const_iterator               Vertex_const_iterator;

        private:
            class insertion_observer : public CGAL::Arr_observer<Arrangement_2>
            {
            public:

                insertion_observer(Arrangement_2& arr) :
                    CGAL::Arr_observer<Arrangement_2>(arr)
                {}

                virtual void after_split_edge(Arrangement_2::Halfedge_handle e1, Arrangement_2::Halfedge_handle e2)
                {

                    bool e1_at_extremity = (e1 == e1->next()->twin() || e1 == e1->prev()->twin());
                    bool e2_at_extremity = (e2 == e2->next()->twin() || e2 == e2->prev()->twin());

                    auto e1_data = SegmentIdentification(e1->curve().data().front());
                    auto e2_data = SegmentIdentification(e2->curve().data().front());

                    // Case of a unqiue segment being split
                    if (e1_at_extremity && e2_at_extremity) {
                        e1_data.position = 0.0;  e1->curve().set_data(e1_data);
                        e2_data.position = 1.0;  e2->curve().set_data(e2_data);
                        return;
                    }

                    if (!e2_at_extremity) {
                        auto e2_next_same_parent = e2->next();
                        while (e2_data.parent_id != e2_next_same_parent->curve().data().front().parent_id) {
                            e2_next_same_parent = e2_next_same_parent->next();
                        }

                        auto e2_prev_same_parent = e2->prev();
                        while (e2_data.parent_id != e2_prev_same_parent->curve().data().front().parent_id) {
                            e2_prev_same_parent = e2_prev_same_parent->prev();
                        }

                        if (e2_next_same_parent->curve().data().front().position == e2_data.position) {
                            e2_data.position = (e2_data.position + e2_prev_same_parent->curve().data().front().position) / 2.0;
                        }
                        if (e2_prev_same_parent->curve().data().front().position == e2_data.position) {
                            e2_data.position = (e2_data.position + e2_next_same_parent->curve().data().front().position) / 2.0;
                        }
                        e2->curve().set_data(e2_data);
                    }
                    if (!e1_at_extremity) {
                        auto e1_next_same_parent = e1->next();
                        while (e1_data.parent_id != e1_next_same_parent->curve().data().front().parent_id) {
                            e1_next_same_parent = e1_next_same_parent->next();
                        }

                        auto e1_prev_same_parent = e1->prev();
                        while (e1_data.parent_id != e1_prev_same_parent->curve().data().front().parent_id) {
                            e1_prev_same_parent = e1_prev_same_parent->prev();
                        }

                        if (e1_next_same_parent->curve().data().front().position == e1_data.position) {
                            e1_data.position = (e1_data.position + e1_prev_same_parent->curve().data().front().position) / 2.0;
                        }
                        if (e1_prev_same_parent->curve().data().front().position == e1_data.position) {
                            e1_data.position = (e1_data.position + e1_next_same_parent->curve().data().front().position) / 2.0;
                        }
                        e1->curve().set_data(e1_data);
                    }
                    return;

                    // This solution maybe will fail if segment is unique
                    bool at_extremity = e2->next()->twin() == e2;
                    if (!at_extremity) {
                        SegmentIdentification data_copy2(e2->curve().data().front());
                        double e2_next_position = e2->next()->curve().data().front().position;
                        data_copy2.position = (data_copy2.position + e2_next_position) / 2.0;
                        e2->curve().set_data(data_copy2);
                    }
                    else {
                        SegmentIdentification data_copy1(e1->curve().data().front());
                        double e1_next_position = e1->prev()->curve().data().front().position;
                        data_copy1.position = (data_copy1.position + e1_next_position) / 2.0;
                        e1->curve().set_data(data_copy1);
                    }
                }

                virtual void after_create_edge(Arrangement_2::Halfedge_handle e) {

                    SegmentIdentification data_copy(e->curve().data().front());

                    auto previous_same_parent = e->prev();
                    while (data_copy.parent_id != previous_same_parent->curve().data().front().parent_id) {
                        previous_same_parent = previous_same_parent->prev();
                    }

                    if (previous_same_parent == e || previous_same_parent->twin() == e)
                        return;

                    // check for next edge
                    auto next_same_parent = e->next();
                    while (data_copy.parent_id != next_same_parent->curve().data().front().parent_id) {
                        next_same_parent = next_same_parent->next();
                    }

                    double previous_pos = previous_same_parent->curve().data().front().position;
                    double next_pos = next_same_parent->curve().data().front().position;

                    if (previous_pos == next_pos) {
                        data_copy.position += 0.5;
                    }
                    else {
                        if (previous_pos == data_copy.position) {
                            data_copy.position = (data_copy.position + next_pos) / 2.0;
                        }
                        if (next_pos == data_copy.position) {
                            data_copy.position = (data_copy.position + previous_pos) / 2.0;
                        }
                    }
                    e->curve().set_data(data_copy);
                }

            };

        public:

            static Arrangement_2 arrangmentFromLineStringGeovector(const GeoVector<Boost_LineString_2>& in_gvec, std::function<SegmentIdentification(const Geometries_with_attributes<Boost_LineString_2>&)>& transformer_fn) {
                Arrangement_2 arr;
                insertion_observer obs(arr);
                Landmarks_pl pl(arr);
                for (const auto& gwa : in_gvec.geometries_container) {
                    SegmentIdentification c_gwa_data = transformer_fn(gwa);
                    const auto& c_linestring_geom = gwa.get_definition();
                    Segment_2 c_seg(Point_2(c_linestring_geom.at(0).get<0>(), c_linestring_geom.at(0).get<1>()), Point_2(c_linestring_geom.at(1).get<0>(), c_linestring_geom.at(1).get<1>()));
                    CGAL::insert(arr, Identifiable_segment_2(c_seg, c_gwa_data), pl);
                }
                return arr;
            }

            static GeoVector<Boost_LineString_2> geovectorFromArrangment(const Arrangement_2& arr) {
                //std::function<Geometries_with_attributes<Boost_LineString_2>(Arrangement_2::Halfedge_handle)> untransformer_fn
                GeoVector<Boost_LineString_2> out_gvec;
                for (auto c_edge = arr.edges_begin(); c_edge != arr.edges_end(); c_edge++) {
                    Geometries_with_attributes<Boost_LineString_2> c_gwa;
                    Boost_LineString_2 c_geom;
                    c_geom.push_back({ PointTraits<Point_2>::getX(c_edge->curve().source()), PointTraits<Point_2>::getY(c_edge->curve().source()) });
                    c_geom.push_back({ PointTraits<Point_2>::getX(c_edge->curve().target()), PointTraits<Point_2>::getY(c_edge->curve().target()) });
                    c_gwa.set_definition(c_geom);
                    c_gwa.set_int_attribute("pid", c_edge->curve().data().front().parent_id);
                    c_gwa.set_double_attribute("pos", c_edge->curve().data().front().position);
                    out_gvec.add_geometry(c_gwa);
                }
                return out_gvec;
            }

            static GeoVector<Boost_LineString_2> FilteredDisplacedGeovectorFromArrangment(const Arrangement_2& arr,
                std::function<bool (Vertex_const_iterator)> filter_predicate,
                std::unordered_map<Vertex_const_iterator, std::pair<double, double>>& displacement_map={}) {
                //std::function<Geometries_with_attributes<Boost_LineString_2>(Arrangement_2::Halfedge_handle)> untransformer_fn
                GeoVector<Boost_LineString_2> out_gvec;
                for (auto c_edge = arr.edges_begin(); c_edge != arr.edges_end(); c_edge++) {
                    auto& edge_data = c_edge->curve().data().front();
                    auto source_v = c_edge->source();
                    auto target_v = c_edge->target();
                    if (!filter_predicate(source_v) && !filter_predicate(target_v))
                        continue;
                    std::pair<double, double> source_disp = (displacement_map.empty()) ? std::make_pair<double,double>(0,0) : displacement_map[source_v];
                    std::pair<double, double> target_disp = (displacement_map.empty()) ? std::make_pair<double, double>(0, 0) : displacement_map[target_v];
                    Geometries_with_attributes<Boost_LineString_2> c_gwa;
                    Boost_LineString_2 c_geom;
                    c_geom.push_back({ PointTraits<Point_2>::getX(source_v->point()) + source_disp.first, PointTraits<Point_2>::getY(source_v->point()) + source_disp.second });
                    c_geom.push_back({ PointTraits<Point_2>::getX(target_v->point()) + target_disp.first, PointTraits<Point_2>::getY(target_v->point()) + target_disp.second });
                    c_gwa.set_definition(c_geom);
                    c_gwa.set_int_attribute("pid", edge_data.parent_id);
                    c_gwa.set_double_attribute("pos", edge_data.position);
                    out_gvec.add_geometry(c_gwa);
                }
                return out_gvec;
            }

		};
	}
}