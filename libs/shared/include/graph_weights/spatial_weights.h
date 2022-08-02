#pragma once
#include "defs.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include "geometries_with_attributes/linestring_with_attributes.h"


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

		template <typename geometry_type>
		class SpatialWeights {

		public:
			LX_GEO_FACTORY_SHARED_API SpatialWeights() {
				max_neighbors = 0;
				min_neighbors = LONG_MAX;
				mean_neighbors = 0;
				n_components = 0;
				islands = 0;
			};

			LX_GEO_FACTORY_SHARED_API SpatialWeights(std::vector<geometry_type>& input_geometries): SpatialWeights(){
				geometries_container = input_geometries;
				_reset();
			};

			void _reset() {
				weights_graph.clear();
				weights_graph = WeightsGraph(geometries_container.size());
				component_labels.clear();
				component_labels.reserve(geometries_container.size());
				max_neighbors = NULL;
				min_neighbors = NULL;
				mean_neighbors = NULL;
				n_components = NULL;
				islands = NULL;
			}

			LX_GEO_FACTORY_SHARED_API ~SpatialWeights() {};

			LX_GEO_FACTORY_SHARED_API virtual void fill_graph() {};

			LX_GEO_FACTORY_SHARED_API void disconnect_edges(const std::function< bool(double)>& diconnection_lambda) {
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

			LX_GEO_FACTORY_SHARED_API void run_labeling(){
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

			LX_GEO_FACTORY_SHARED_API std::vector<LineString_with_attributes> export_edge_graph_as_LSwithAttr() {
				std::vector<LineString_with_attributes> edges_linestrings;
				edges_linestrings.reserve(geometries_container.size());
				auto es = boost::edges(weights_graph);
				for (auto eit = es.first; eit != es.second; ++eit) {
					vertex_descriptor source_ = boost::source(*eit, weights_graph) , target_ = boost::target(*eit, weights_graph);
					Boost_Point_2 source_rep_pt,target_rep_pt;
					boost::geometry::centroid(geometries_container[source_], source_rep_pt);
					boost::geometry::centroid(geometries_container[target_], target_rep_pt);

					LineString_with_attributes c_edge_container(Boost_LineString_2({source_rep_pt, target_rep_pt}));
					double edge_weight = boost::get(boost::edge_weight_t(), weights_graph, *eit);
					size_t c_label = component_labels[source_];
					c_edge_container.set_double_attribute("weight", edge_weight);
					c_edge_container.set_int_attribute("component", static_cast<int>(c_label));
					edges_linestrings.push_back(c_edge_container);
				}
				return edges_linestrings;
			}
			
		public:			
			std::vector<geometry_type> geometries_container;
			WeightsGraph weights_graph;
			std::vector<size_t> component_labels;
			size_t n_components;
		private:
			size_t max_neighbors;
			size_t min_neighbors;
			double mean_neighbors;
			size_t islands;
		};
				

		class WeightsDistanceBandParams{

		public:
			double threshold; // distance threshold
			bool binary = false; // If true w_{ij}=1 if d_{i,j}<=threshold, otherwise w_{i,j}=0
			double alpha = -1.0f;  // distance decay parameter for weight
			std::function<double(double)> distance_kernel;
		};

		struct WeightsKNNParams {
			size_t K; // number of nearest neighbors
		};

	}
}