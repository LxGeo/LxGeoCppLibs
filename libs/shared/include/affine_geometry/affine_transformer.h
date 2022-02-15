#pragma once
#include "defs.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		template <typename geometry_type>
		geometry_type translate_geometry(geometry_type& in_geometry, bg::strategy::transform::translate_transformer<double, 2, 2>& trans_obj) {
			geometry_type out_geom;
			boost::geometry::transform(in_geometry, out_geom, trans_obj);
			return out_geom;
		}

		//template<template <class> class container_type, class geometry_type>
		template<typename geometry_type>
		std::vector<geometry_type> translate_geometries(std::vector<geometry_type>& input_container, std::vector<SpatialCoords>& displacement_container) {

			assert(input_container.size() == displacement_container.size(), "Size mismatch of input containers!");
			std::vector<geometry_type> out_container; out_container.reserve(input_container.size());

			std::vector<geometry_type>::iterator it1 = input_container.begin();
			std::vector<SpatialCoords>::iterator it2 = displacement_container.begin();

			while (it1 != input_container.end() && it2 != displacement_container.end()) {
				bg::strategy::transform::translate_transformer<double, 2, 2> trans_obj(it2->yc, it2->xc);
				out_container.push_back(translate_geometry(*it1, trans_obj));
				it1++;
				it2++;
			}
			return out_container;

		}

	}
}