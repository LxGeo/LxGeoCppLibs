#pragma once
#include "defs.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		class baseParameters
		{

		public:
			std::unordered_map<std::string, double> optional_numerical_parameters;
			std::unordered_map<std::string, std::string> optional_str_parameters;

		};

	}
}
