#pragma once
#include "defs.h"
#include <boost/numeric/ublas/operation.hpp>
#include "export_shared.h"


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

		template <typename geometry_type>
		geometry_type translate_geometry(const geometry_type& in_geometry, const std::pair<double, double>& xy_pair ) {
			geometry_type out_geom;
			bg::strategy::transform::translate_transformer<double, 2, 2> trans_obj(xy_pair.first, xy_pair.second);
			boost::geometry::transform(in_geometry, out_geom, trans_obj);
			return out_geom;
		}
		
		template <typename in_geometry_type, typename out_geometry_type>
		out_geometry_type affine_transform_geometry(const in_geometry_type& in_geometry, const bg::strategy::transform::matrix_transformer<double, 2, 2>& trans_obj) {
			out_geometry_type out_geom;
			boost::geometry::transform(in_geometry, out_geom, trans_obj);
			return out_geom;
		}


		LX_GEO_FACTORY_SHARED_API bg::strategy::transform::matrix_transformer<double, 2, 2> geotransform_to_matrix_transformer(const double geotransform[6], double x_shift = 0.0, double y_shift = 0.0);


		LX_GEO_FACTORY_SHARED_API bg::strategy::transform::inverse_transformer<double, 2, 2> geotransform_to_inv_matrix_transformer(const double geotransform[6]);

	}
}