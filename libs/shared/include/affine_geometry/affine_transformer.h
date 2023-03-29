#pragma once
#include "defs.h"
#include <boost/numeric/ublas/operation.hpp>


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		template <typename geometry_type>
		geometry_type translate_geometry(const geometry_type& in_geometry, bg::strategy::transform::translate_transformer<double, 2, 2>& trans_obj) {
			geometry_type out_geom;
			boost::geometry::transform(in_geometry, out_geom, trans_obj);
			return out_geom;
		}

		//template<template <class> class container_type, class geometry_type>
		template<typename geometry_type>
		std::vector<geometry_type> translate_geometries(std::vector<geometry_type>& input_container, std::vector<SpatialCoords>& displacement_container) {

			assert(input_container.size() == displacement_container.size() || displacement_container.size()==1, "Size mismatch of input containers!");
			std::vector<geometry_type> out_container; out_container.reserve(input_container.size());

			auto it1 = input_container.begin();
			auto it2 = displacement_container.begin();

			while (it1 != input_container.end() && it2 != displacement_container.end()) {
				bg::strategy::transform::translate_transformer<double, 2, 2> trans_obj(it2->xc, it2->yc);
				geometry_type transformed_geom = translate_geometry(*it1, trans_obj);
				out_container.push_back(transformed_geom);
				it1++;
				if (displacement_container.size() != 1) it2++;
			}
			return out_container;

		}


		template<typename geometry_type>
		std::list<geometry_type> translate_geometries(std::list<geometry_type>& input_container, std::list<SpatialCoords>& displacement_container) {

			assert(input_container.size() == displacement_container.size() || displacement_container.size() == 1, "Size mismatch of input containers!");
			std::list<geometry_type> out_container;
			
			std::list<geometry_type>::iterator it1 = input_container.begin();
			std::list<SpatialCoords>::iterator it2 = displacement_container.begin();

			while (it1 != input_container.end() && it2 != displacement_container.end()) {
				bg::strategy::transform::translate_transformer<double, 2, 2> trans_obj(it2->xc, it2->yc);
				out_container.push_back(translate_geometry(*it1, trans_obj));
				it1++;
				if (displacement_container.size() != 1) it2++;
			}
			return out_container;
		}

		/*template <typename geometry_type>
		LX_GEO_FACTORY_SHARED_API geometry_type affine_transform_geometry(geometry_type& in_geometry, bg::strategy::transform::matrix_transformer<double, 2, 2>& trans_obj) {
			geometry_type out_geom;
			boost::geometry::transform(in_geometry, out_geom, trans_obj);
			return out_geom;
		}
		*/
		template<typename geometry_type>
		std::vector<geometry_type> affine_transform_geometries(std::vector<geometry_type>& input_container,
			std::vector<bg::strategy::transform::matrix_transformer<double, 2, 2>>& transformation_container) {

			assert(input_container.size() == transformation_container.size() || transformation_container.size() == 1, "Size mismatch of input containers!");
			std::vector<geometry_type> out_container; out_container.reserve(input_container.size());

			auto it1 = input_container.begin();
			auto it2 = transformation_container.begin();

			while (it1 != input_container.end() && it2 != transformation_container.end()) {
				bg::strategy::transform::matrix_transformer<double, 2, 2>& trans_obj = *it1;
				geometry_type transformed_geom = translate_geometry(*it1, trans_obj);
				out_container.push_back(transformed_geom);
				it1++;
				if (displacement_container.size() != 1) it2++;
			}
			return out_container;

		}
		

		template <typename in_geometry_type, typename out_geometry_type>
		out_geometry_type affine_transform_geometry(in_geometry_type& in_geometry, bg::strategy::transform::matrix_transformer<double, 2, 2>& trans_obj) {
			out_geometry_type out_geom;
			boost::geometry::transform(in_geometry, out_geom, trans_obj);
			return out_geom;
		}

		auto geotransform_to_matrix_transformer(double geotransform[6], double x_shift = 0.0, double y_shift = 0.0) {
			return bg::strategy::transform::matrix_transformer<double, 2, 2>(geotransform[1], geotransform[2], geotransform[0] + x_shift,
				geotransform[4], geotransform[5], geotransform[3] + y_shift,
				0.0, 0.0, 1.0);
		}

		auto geotransform_to_inv_matrix_transformer(double geotransform[6]) {
			return bg::strategy::transform::inverse_transformer<double, 2, 2>(geotransform_to_matrix_transformer(geotransform));
		}

	}
}