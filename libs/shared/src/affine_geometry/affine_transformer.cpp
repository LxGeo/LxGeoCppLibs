#include "affine_geometry/affine_transformer.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		bg::strategy::transform::matrix_transformer<double, 2, 2> geotransform_to_matrix_transformer(double geotransform[6], double x_shift, double y_shift) {
			return bg::strategy::transform::matrix_transformer<double, 2, 2>(geotransform[1], geotransform[2], geotransform[0] + x_shift,
				geotransform[4], geotransform[5], geotransform[3] + y_shift,
				0.0, 0.0, 1.0);
		}


		bg::strategy::transform::inverse_transformer<double, 2, 2> geotransform_to_inv_matrix_transformer(double geotransform[6]) {
			return bg::strategy::transform::inverse_transformer<double, 2, 2>(geotransform_to_matrix_transformer(geotransform));
		}

	}
}