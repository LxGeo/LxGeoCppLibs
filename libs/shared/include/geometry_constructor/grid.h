#pragma once

#pragma once

#include "defs.h"
#include "defs_cgal.h"
#include "defs_common.h"
#include "numcpp/arrays.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		std::vector<OGREnvelope> create_rectangular_grid(
			const double& xmin, const double& ymin, const double& xmax, const double& ymax,
			const double& xstep, const double& ystep, const double& xsize, const double& ysize, 
			const std::function<bool(const OGREnvelope&)>& predicate= [](const OGREnvelope& _){return true;}){
		
			std::vector<double> cols = numcpp::arange<double, std::vector, std::allocator>(xmin, xmax, xstep); //(xmin, xmax + x_step, x_step)
			std::vector<double> rows = numcpp::arange<double, std::vector, std::allocator>(ymin, ymax, ystep);

			std::vector<OGREnvelope> envelopes; envelopes.reserve(cols.size() * rows.size());
			for (auto& c_col : cols) {
				for (auto& c_row : rows) {
					OGREnvelope c_enevelop; c_enevelop.MinX = c_col; c_enevelop.MinY = c_row; c_enevelop.MaxX = c_col+ xstep; c_enevelop.MaxY = c_row+ ystep;
					if (predicate(c_enevelop))
						envelopes.push_back(c_enevelop);
				}
			}
			return envelopes;
		}

	}
}