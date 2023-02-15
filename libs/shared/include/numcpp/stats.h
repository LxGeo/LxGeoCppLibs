#pragma once

#include "defs.h"

namespace LxGeo
{
	namespace numcpp
	{

		template<typename T,
			template<typename, typename> class Container,
			template<typename> class Allocator = std::allocator>
		double circular_mean(Container<T, Allocator<T>>& container, double min_val=0, double max_val=360) {
			size_t len = container.size();
			double polar_x_sum = 0, polar_y_sum=0;
			for (auto& c_val : container) {
				double c_rad_val = (c_val - min_val) / (max_val/2) * M_PI;
				polar_x_sum += std::sin(c_rad_val);
				polar_y_sum += std::cos(c_rad_val);
			}
			double mean_rad = std::atan2(polar_x_sum / len, polar_y_sum / len);
			double mean_deg = mean_rad / M_PI * (max_val / 2) + min_val;
			return mean_deg;
		}
			

	}
}