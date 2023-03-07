#pragma once
#include "CLI/CLI.hpp"
#include "cli/base_parameters.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		
		void add_grid_options(CLI::App& app, baseParameters& params) {

			double DEFAULT_GRID_XSTEP = 1000.0;
			double DEFAULT_GRID_YSTEP = 1000.0;
			double DEFAULT_GRID_XSIZE = 1000.0;
			double DEFAULT_GRID_YSIZE = 1000.0;

			auto add_single_argument = [&](std::string& arg_name, double& arg_default_value) {
				params.optional_numerical_parameters[arg_name] = arg_default_value;
				app.add_option("--GRID_"+ arg_name, params.optional_numerical_parameters[arg_name], "Grid generation options (" + arg_name + ")");
			};
			std::pair<std::string, double> arguments_name_value_pairs[4] = {
				{"xstep", DEFAULT_GRID_XSTEP}, {"ystep", DEFAULT_GRID_YSTEP}, {"xsize", DEFAULT_GRID_XSIZE}, {"ysize", DEFAULT_GRID_YSIZE} 
			};
			for (auto& iter : arguments_name_value_pairs) 
				add_single_argument(iter.first, iter.second);

		};

	}
}