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
		std::vector<Boost_Box_2> create_rectangular_grid(
			const double& xmin, const double& ymin, const double& xmax, const double& ymax,
			const double& xstep, const double& ystep, const double& xsize, const double& ysize,
			const std::function<bool(const Boost_Box_2&)>& predicate = [](const Boost_Box_2& _) {return true; }) {
			std::vector<double> cols = numcpp::arange<double, std::vector, std::allocator>(xmin, xmax, xstep); //(xmin, xmax + x_step, x_step)
			std::vector<double> rows = numcpp::arange<double, std::vector, std::allocator>(ymin, ymax, ystep);

			std::vector<Boost_Box_2> envelopes; envelopes.reserve(cols.size() * rows.size());
			for (auto& c_col : cols) {
				for (auto& c_row : rows) {
					Boost_Box_2 c_enevelop(
						Boost_Point_2(c_col, c_row),
						Boost_Point_2(c_col + xsize, c_row + ysize)
					);
					if (predicate(c_enevelop))
						envelopes.push_back(c_enevelop);
				}
			}
			return envelopes;
		}

		std::vector<Boost_Box_2> create_rectangular_grid(
			const Boost_Box_2& box,
			const double& xstep, const double& ystep, const double& xsize, const double& ysize,
			const std::function<bool(const Boost_Box_2&)>& predicate = [](const Boost_Box_2& _) {return true; }) {
			const double xmin = box.min_corner().get<0>();
			const double ymin = box.min_corner().get<1>();
			const double xmax = box.max_corner().get<0>();
			const double ymax = box.max_corner().get<1>();
			return create_rectangular_grid(xmin, ymin, xmax, ymax, xstep, ystep, xsize, ysize, predicate);
		}
		std::vector<Polygon_with_attributes> grid_to_geoms_with_attributes(std::vector<Boost_Box_2>& grid_space) {
			std::vector<Boost_Polygon_2> grid_polys;
			auto temp_conversion = [](Boost_Box_2& a) ->Boost_Polygon_2 {Boost_Polygon_2 out_polygon; bg::assign(out_polygon, a);return out_polygon; };
			transform(grid_space.begin(), grid_space.end(), std::back_inserter(grid_polys), temp_conversion);
			auto grid_geoms = transform_to_geom_with_attr(grid_polys);
			return grid_geoms;
		}

		/******************* OGR geometries ******************/
		std::vector<OGREnvelope> create_rectangular_grid(
			const double& xmin, const double& ymin, const double& xmax, const double& ymax,
			const double& xstep, const double& ystep, const double& xsize, const double& ysize, 
			const std::function<bool(const OGREnvelope&)>& predicate= [](const OGREnvelope& _){return true;}){		
			std::vector<double> cols = numcpp::arange<double, std::vector, std::allocator>(xmin, xmax, xstep); //(xmin, xmax + x_step, x_step)
			std::vector<double> rows = numcpp::arange<double, std::vector, std::allocator>(ymin, ymax, ystep);

			std::vector<OGREnvelope> envelopes; envelopes.reserve(cols.size() * rows.size());
			for (auto& c_col : cols) {
				for (auto& c_row : rows) {
					OGREnvelope c_enevelop; c_enevelop.MinX = c_col; c_enevelop.MinY = c_row; c_enevelop.MaxX = c_col+ xsize; c_enevelop.MaxY = c_row+ ysize;
					if (predicate(c_enevelop))
						envelopes.push_back(c_enevelop);
				}
			}
			return envelopes;
		}

		std::vector<OGREnvelope> create_rectangular_grid(
			const Boost_Box_2& box,
			const double& xstep, const double& ystep, const double& xsize, const double& ysize,
			const std::function<bool(const OGREnvelope&)>& predicate = [](const OGREnvelope& _) {return true; }) {
			const double xmin = box.min_corner().get<0>();
			const double ymin = box.min_corner().get<1>();
			const double xmax = box.max_corner().get<0>();
			const double ymax = box.max_corner().get<1>();
			return create_rectangular_grid(xmin, ymin, xmax, ymax, xstep, ystep, xsize, ysize, predicate);
		}

		std::vector<Polygon_with_attributes> grid_to_geoms_with_attributes(std::vector<OGREnvelope>& grid_space) {
			std::vector<Boost_Polygon_2> grid_polys;
			auto temp_conversion = [](OGREnvelope& a) ->Boost_Polygon_2 {return transform_OGR2B_Polygon(&envelopeToPolygon(a)); };
			transform(grid_space.begin(), grid_space.end(), std::back_inserter(grid_polys), temp_conversion); 
			auto grid_geoms = transform_to_geom_with_attr(grid_polys);
			return grid_geoms;
		}


	}
}