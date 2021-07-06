#pragma once
#include <iostream>
#include <utility>
#include <set>
#include <map>
#include <list>
#include <deque>
#include <vector>
#include <valarray>
#include <string>
#include <queue>
#include <boost/log/trivial.hpp>

#include "defs_common.h"
#include "defs_boost.h"
#include "defs_opencv.h"
#include <fmt/core.h>

using namespace LxGeo::LxGeoCommon;
using namespace LxGeo::GeometryFactoryShared;
// move this to common
typedef GDALDataset GDALDataset;

template<typename T>
T get_vector_by_indices(T& input_vector, std::vector<size_t>& indices_vector) {
	T output_vector;
	output_vector.reserve(indices_vector.size());
	for (size_t c_index : indices_vector) {
		output_vector.push_back(input_vector[c_index]);
	}
	return output_vector;
}

# define M_PI 3.14159265358979323846
