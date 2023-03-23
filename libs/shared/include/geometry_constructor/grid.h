#pragma once

#include "defs.h"
#include "defs_cgal.h"
#include "defs_common.h"
#include "numcpp/arrays.h"
#include "geometries_with_attributes/polygon_with_attributes.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		/******************* Boost geometries ******************/
		template <typename envelope_type>
		std::vector<envelope_type> create_rectangular_grid(
			const double& xmin, const double& ymin, const double& xmax, const double& ymax,
			const double& xstep, const double& ystep, const double& xsize, const double& ysize,
			const std::function<bool(const envelope_type&)>& predicate = [](const envelope_type& _) {return true; }) {
			std::vector<double> cols = numcpp::arange<double, std::vector, std::allocator>(xmin, xmax, xstep); //(xmin, xmax + x_step, x_step)
			std::vector<double> rows = numcpp::arange<double, std::vector, std::allocator>(ymin, ymax, ystep);

			std::vector<envelope_type> envelopes; envelopes.reserve(cols.size() * rows.size());
			for (auto& c_col : cols) {
				for (auto& c_row : rows) {
					envelope_type c_enevelop = create_envelope<envelope_type>(c_col, c_row, c_col + xsize, c_row + ysize);
					if (predicate(c_enevelop))
						envelopes.push_back(c_enevelop);
				}
			}
			return envelopes;
		}

		template <typename return_envelope_type, typename in_envelope_type>
		std::vector<return_envelope_type> create_rectangular_grid(
			const in_envelope_type& box,
			const double& xstep, const double& ystep, const double& xsize, const double& ysize,
			const std::function<bool(const return_envelope_type&)>& predicate = [](const return_envelope_type& _) {return true; }) {
			auto abstract_envelope = envelopeGet<in_envelope_type>(&box);
			const double xmin = abstract_envelope.getMinX();
			const double ymin = abstract_envelope.getMinY();
			const double xmax = abstract_envelope.getMaxX();
			const double ymax = abstract_envelope.getMaxY();
			return create_rectangular_grid<return_envelope_type>(xmin, ymin, xmax, ymax, xstep, ystep, xsize, ysize, predicate);
		}

	}
}