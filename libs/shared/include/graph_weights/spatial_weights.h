#pragma once
#include "defs.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include "geometries_with_attributes/geometries_with_attributes.h"
#include "geometries_with_attributes/linestring_with_attributes.h"
#include "spatial_index/spatial_indexed_geometry_container.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		typedef boost::property<boost::edge_weight_t, double> EdgeWeightProperty;
		typedef boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS, boost::no_property, EdgeWeightProperty> WeightsGraph;
				
		typedef boost::graph_traits<WeightsGraph>::vertex_iterator vertex_iterator;
		typedef boost::graph_traits<WeightsGraph>::vertex_descriptor vertex_descriptor;
		typedef boost::graph_traits<WeightsGraph>::edge_descriptor edge_descriptor;
		typedef boost::graph_traits<WeightsGraph>::adjacency_iterator adjacency_iterator;
		typedef boost::graph_traits<WeightsGraph>::out_edge_iterator out_edge_iterator;
		typedef boost::graph_traits<WeightsGraph>::edge_iterator edge_iterator;


		enum WeightsStrategy {
			DISTANCE_BAND,
			KNN,
			ROOK_NEIGHBOURS,
			QUEEN_NEIGHBOURS
		};


		class WeightsDistanceBandParams {
		public:
			double threshold; // distance threshold
			bool binary = false; // If true w_{ij}=1 if d_{i,j}<=threshold, otherwise w_{i,j}=0
			double alpha = -1.0f;  // distance decay parameter for weight
			std::function<double(double)> distance_kernel;
		};

		struct WeightsKNNParams {
			size_t K; // number of nearest neighbors
		};

		template <typename geom_type>
		class SpatialWeights : public SpatialIndexedGeometryContainer<geom_type> {

			using Boost_Value = typename SpatialIndexedGeometryContainer<geom_type>::Boost_Value;

		public:
			SpatialWeights(): SpatialIndexedGeometryContainer<geom_type>() {};

			SpatialWeights(const std::vector<Geometries_with_attributes<geom_type>>& input_geometries): SpatialIndexedGeometryContainer<geom_type>(), geometries_container(input_geometries){
				_reset();
				init_rtree();
			};

			SpatialWeights(const std::vector<geom_type>& input_geometries) : SpatialIndexedGeometryContainer<geom_type>() {
				_reset();
				geometries_container.reserve(input_geometries.size());
				for (auto& c_geom : input_geometries)
					geometries_container.push_back(Geometries_with_attributes<geom_type>(c_geom));
				init_rtree();
			};

			SpatialWeights(const std::vector<Geometries_with_attributes<geom_type>>& input_geometries, Boost_RTree& ref_rtree) :
				SpatialIndexedGeometryContainer<geom_type>(ref_rtree), geometries_container(input_geometries) {
				_reset();
			};

			~SpatialWeights() {};

			void _reset() {
				weights_graph.clear();
				weights_graph = WeightsGraph(length());
				component_labels.clear();
				component_labels.reserve(length());
				max_neighbors = NULL;
				min_neighbors = NULL;
				mean_neighbors = NULL;
				n_components = NULL;
				islands = NULL;
			}

			void fill_distance_band_graph(WeightsDistanceBandParams& wdbp) {

				_reset();
				double& threshold_value = wdbp.threshold;
				std::function<double(double)>& distance_kernel = wdbp.distance_kernel;

				for (size_t c_geom_idx = 0; c_geom_idx < length(); ++c_geom_idx) {

					geom_type& c_geom = this->operator[](c_geom_idx);
					Boost_Box_2 c_geom_envelop;
					boost::geometry::envelope(c_geom, c_geom_envelop);
					Boost_Box_2 c_geom_envelop_buffered = box_buffer(c_geom_envelop, threshold_value);

					// filtter polygons within buffered envelop
					std::list<Boost_Value> candidates;
					rtree.query(bgi::intersects(c_geom_envelop_buffered), std::back_inserter(candidates));
					// search for polygons meeting threshomd critirea
					for (auto& c_candidate : candidates) {
						size_t c_candidate_idx = c_candidate.second;
						geom_type& c_candidate_geom = this->operator[](c_candidate_idx);
						double inter_distance = boost::geometry::distance(c_geom, c_candidate_geom);
						if (inter_distance > threshold_value)
							continue;
						else {
							// adding edge
							double edge_weight = !wdbp.binary ? distance_kernel(inter_distance) : 1;
							if (!boost::edge(c_candidate_idx, c_geom_idx, weights_graph).second)
								boost::add_edge(c_geom_idx, c_candidate_idx, EdgeWeightProperty{ edge_weight }, weights_graph);
						}
					}

				}
			}

			void disconnect_edges(const std::function< bool(double)>& diconnection_lambda) {
				auto edge_weight_map = boost::get(boost::edge_weight, weights_graph);
				
				edge_iterator vi, vi_end, next;
				boost::tie(vi, vi_end) = boost::edges(weights_graph);
				for (next = vi; vi != vi_end; vi = next) {
					++next;
					if (diconnection_lambda(edge_weight_map[*vi]))
					{
						boost::remove_edge(*vi, weights_graph);
					}
				}
			};

			void run_labeling(){
				//connected component
				component_labels = std::vector<size_t>(boost::num_vertices(weights_graph));
				n_components = boost::connected_components(weights_graph, &component_labels[0]);

				size_t sum_degrees = 0;
				vertex_iterator v, vend;
				for (boost::tie(v, vend) = boost::vertices(weights_graph); v != vend; ++v) {
					size_t c_v_degrees = boost::degree(*v, weights_graph);
					sum_degrees += c_v_degrees;
					if (c_v_degrees > max_neighbors) max_neighbors = c_v_degrees;
					if (c_v_degrees < min_neighbors) min_neighbors = c_v_degrees;
					if (c_v_degrees == 0) islands+=1;
				}
				mean_neighbors = double(sum_degrees) / boost::num_vertices(weights_graph);
			}

			std::vector<LineString_with_attributes> export_edge_graph_as_LSwithAttr() {
				std::vector<LineString_with_attributes> edges_linestrings;
				edges_linestrings.reserve(length());
				auto es = boost::edges(weights_graph);
				for (auto eit = es.first; eit != es.second; ++eit) {
					vertex_descriptor source_ = boost::source(*eit, weights_graph) , target_ = boost::target(*eit, weights_graph);
					Boost_Point_2 source_rep_pt,target_rep_pt;
					boost::geometry::centroid(this->operator[](source_), source_rep_pt);
					boost::geometry::centroid(this->operator[](target_), target_rep_pt);

					LineString_with_attributes c_edge_container(Boost_LineString_2({source_rep_pt, target_rep_pt}));
					double edge_weight = boost::get(boost::edge_weight_t(), weights_graph, *eit);
					size_t c_label = component_labels[source_];
					c_edge_container.set_double_attribute("weight", edge_weight);
					c_edge_container.set_int_attribute("component", static_cast<int>(c_label));
					edges_linestrings.push_back(c_edge_container);
				}
				return edges_linestrings;
			}

		private:
			size_t length() const override { return geometries_container.size(); }
			geom_type& operator[](int offset) override { return geometries_container[offset].get_definition(); }
			const geom_type& operator[](int offset) const override { return geometries_container[offset].get_definition(); }
			
		public:			
			std::vector<Geometries_with_attributes<geom_type>> geometries_container;
			WeightsGraph weights_graph;
			std::vector<size_t> component_labels;
			size_t n_components=0;
		private:
			size_t max_neighbors=0;
			size_t min_neighbors=0;
			double mean_neighbors=0;
			size_t islands=0;
		};
				


	}
}