#pragma once
#include "defs.h"

namespace LxGeo
{
	namespace numcpp
	{

        template<typename T,
            template<typename, typename> class Container,
            template<typename> class Allocator = std::allocator>
        Container<T, Allocator<T>> linspace(const T& start_in, const T& end_in, const size_t& num_in) {

            Container<T, Allocator<T>> linspaced; linspaced.reserve(num_in);

            double start = static_cast<double>(start_in);
            double end = static_cast<double>(end_in);
            double num = static_cast<double>(num_in);

            if (num == 0) { return linspaced; }
            if (num == 1)
            {
                linspaced.push_back(start);
                return linspaced;
            }

            double delta = (end - start) / (num - 1);

            for (int i = 0; i < num - 1; ++i)
            {
                linspaced.push_back(start + delta * i);
            }
            linspaced.push_back(end); // I want to ensure that start and end
                                      // are exactly the same as the input
            return linspaced;
        }

		template<typename T,
			template<typename, typename> class Container,
			template<typename> class Allocator = std::allocator>
		Container<T, Allocator<T>> arange(const T& start, const T& stop, const T& step = 1) {
            assert(step != 0);
			Container<T, Allocator<T>> values;
			for (T value = start; value < stop; value += step)
				values.push_back(value);
			return values;
		}

	}
}