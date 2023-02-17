#pragma once
#include "CLI/CLI.hpp"
#include "cli/base_parameters.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		enum class Device : int { CPU, GPU, COMBINED };

		std::map<std::string, Device> device_map{ {"cpu", Device::CPU}, {"gpu", Device::GPU}, {"combined", Device::COMBINED}};

		void add_device_option(CLI::App& app, baseParameters& params) {
			params.optional_str_parameters["device"] = "cpu";
			app.add_option("-d,--device", params.optional_str_parameters["device"], "Device choice (CPU, GPU or COMBINED)")
				->transform(CLI::CheckedTransformer(device_map, CLI::ignore_case));
		}

	}
}
