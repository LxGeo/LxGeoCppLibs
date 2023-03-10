#pragma once
#include "defs.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{


		class AffineTransformerBase {

		public:
			AffineTransformerBase(const double* _geotransform) {
				geotransform = _geotransform;
			}

			template <typename coord_type>
			void _calc_pixel_coords(const double& sc_x, const double& sc_y, coord_type& px_col, coord_type& px_row) const {
				const double& px = geotransform[0];
				const double& py = geotransform[3];
				const double& rx = geotransform[1];
				const double& ry = geotransform[5];
				const double& nx = geotransform[2];
				const double& ny = geotransform[4];
				//px_col = static_cast<coord_type>((sc_x - px) / rx);
				//px_row = static_cast<coord_type>((sc_y - py) / ry);

				px_col = (ny) ? static_cast<coord_type>((sc_x - px) / rx + (sc_y - py) / ny) : static_cast<coord_type>((sc_x - px) / rx);
				px_row = (nx) ? static_cast<coord_type>((sc_y - py) / ry + (sc_x - px) / nx) : static_cast<coord_type>((sc_y - py) / ry);
			}

			void _calc_spatial_coords(const int& px_col, const int& px_row, double& sc_x, double& sc_y) const {
				const double& px = geotransform[0];
				const double& py = geotransform[3];
				const double& rx = geotransform[1];
				const double& ry = geotransform[5];
				const double& nx = geotransform[2];
				const double& ny = geotransform[4];
				sc_x = px_col * rx + px + px_row * nx;
				sc_y = px_row * ry + py + px_col * ny;
			}

		public:
			const double* geotransform; // xoff, a, b, yoff, d, e = geotransform

		};


	}
}