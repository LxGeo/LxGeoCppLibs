#pragma once
#include "defs.h"
#include "spatial_weights.h"



namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		class PolygonSpatialWeights : public SpatialWeights<Boost_Polygon_2> {

		public:

			PolygonSpatialWeights():SpatialWeights() {};

			PolygonSpatialWeights(std::vector<Boost_Polygon_2>& input_polygons) :SpatialWeights(input_polygons) {};

			void fill_distance_band_graph(WeightsDistanceBandParams& wdbp) {

				_reset();
				Boost_RTree_2 rtree;
				double& threshold_value = wdbp.threshold;
				std::function<double(double)>& distance_kernel=wdbp.distance_kernel;
				

				for (size_t c_polygon_idx = 0; c_polygon_idx < geometries_container.size(); ++c_polygon_idx) {
					
					Boost_Polygon_2& c_polygon = geometries_container[c_polygon_idx];
					Boost_Box_2 c_polygon_envelop;
					boost::geometry::envelope(c_polygon, c_polygon_envelop);
					Boost_Box_2 c_polygon_envelop_buffered = box_buffer(c_polygon_envelop, threshold_value);

					// filtter polygons within buffered envelop
					std::list<Boost_Value_2> candidates;
					rtree.query(bgi::intersects(c_polygon_envelop_buffered), std::back_inserter(candidates));
					// search for polygons meeting threshomd critirea
					for (auto& c_candidate : candidates) {
						size_t c_candidate_idx = c_candidate.second;
						Boost_Polygon_2& c_candidate_polygon = geometries_container[c_candidate_idx];
						double inter_distance = boost::geometry::distance(c_polygon, c_candidate_polygon);
						if (inter_distance > threshold_value)
							continue;
						else {
							// adding edge
							double edge_weight = !wdbp.binary ? distance_kernel(inter_distance) : 1;
							boost::add_edge(c_polygon_idx, c_candidate_idx, EdgeWeightProperty{edge_weight}, weights_graph);
						}
					}

					// add current polygon to rtree
					rtree.insert(Boost_Value_2(c_polygon_envelop, c_polygon_idx));

				}
			}

			~PolygonSpatialWeights() {};

		};

	}
}
