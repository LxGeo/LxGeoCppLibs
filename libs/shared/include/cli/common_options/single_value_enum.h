#pragma once
#include "CLI/CLI.hpp"
#include "cli/base_parameters.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		template <typename enum_type>
		void add_single_value_enum_option(CLI::App& app, enum_type& enum_sett, const std::map<std::string, enum_type>& enum_map,
			const std::string& opt_specifier, const std::string& opt_description) {
			app.add_option(opt_specifier, enum_sett, opt_description)
				->transform(CLI::CheckedTransformer(enum_map, CLI::ignore_case));
		};

	}
}