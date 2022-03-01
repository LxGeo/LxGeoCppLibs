#pragma once
#include "defs.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		template<typename child_type>
		struct compositionStrucure {
			std::vector<size_t> parents_indices;
			std::vector<child_type> children;
			size_t parent_count;


			template<typename child_proprety, typename parent_proprety>
			std::vector<parent_proprety> aggregate_children_to_parent(std::vector<child_proprety>& children_properties,
				std::function< parent_proprety(std::list<child_proprety>) > properties_aggregator) {

				std::vector<parent_proprety> parents_properties; parents_properties.reserve(parent_count);

				std::map<size_t, std::list<child_proprety>> parent_to_children_map;
				// filling parent_to_children_map
				for (size_t child_idx = 0; child_idx < parents_indices.size(); ++child_idx) {
					size_t& parent_idx = parents_indices[child_idx];
					if (parent_to_children_map.find(parent_idx) == parent_to_children_map.end()) {
						parent_to_children_map[parent_idx] = { children_properties[child_idx] };
					}
					else {
						parent_to_children_map[parent_idx].push_back(children_properties[child_idx]);
					}
				}

				std::transform(parent_to_children_map.begin(), parent_to_children_map.end(), std::back_inserter(parents_properties),
					[&properties_aggregator](auto& map_it)->parent_proprety {return properties_aggregator(map_it.second); });

				return parents_properties;
			};
		
			
			template<typename child_proprety, typename parent_proprety>
			std::vector<parent_proprety> aggregate_children_to_parent(std::vector<child_proprety>& children_properties, std::vector<double>& children_weights,
				std::function< parent_proprety(std::list<child_proprety>, std::list<double>) > properties_aggregator) {

				std::vector<parent_proprety> parents_properties; parents_properties.reserve(parent_count);

				std::map<size_t, std::list<child_proprety>> parent_to_children_map;
				std::map<size_t, std::list<double>> parent_to_children_weight_map;
				// filling parent_to_children_map
				for (size_t child_idx = 0; child_idx < parents_indices.size(); ++child_idx) {
					size_t& parent_idx = parents_indices[child_idx];
					if (parent_to_children_map.find(parent_idx) == parent_to_children_map.end()) {
						parent_to_children_map[parent_idx] = { children_properties[child_idx] };
						parent_to_children_weight_map[parent_idx] = { children_weights[child_idx] };
					}
					else {
						parent_to_children_map[parent_idx].push_back(children_properties[child_idx]);
						parent_to_children_weight_map[parent_idx].push_back(children_weights[child_idx]);
					}
				}

				for (auto& c_map_iter : parent_to_children_map) {
					parents_properties.push_back(properties_aggregator(c_map_iter.second, parent_to_children_weight_map[c_map_iter.first]));
				}

				return parents_properties;
			};
		
		};

	}
}