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
#include <algorithm> 
#include <boost/log/trivial.hpp>

#include "defs_common.h"
#include "defs_boost.h"
#include "defs_opencv.h"
#include "defs_cgal.h"
#include "geometry_lab.h"
#include <fmt/core.h>

using namespace LxGeo::LxGeoCommon;
using namespace LxGeo::GeometryFactoryShared;

template<typename T, typename T2>
T get_vector_by_indices(T& input_vector, T2& indices_vector) {
	T output_vector;
	output_vector.reserve(indices_vector.size());
	for (size_t c_index : indices_vector) {
		output_vector.push_back(input_vector[c_index]);
	}
	return output_vector;
}

template<typename T>
int sign(T number) {
	if (number < 0) return -1;
	else if (number > 0) return 1;
	else return 0;
}

namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		int mod(int a, int b);
	}
}


namespace Constants
{
	const double PI = 3.1415926535897832384626;
	const double SQRT_2 = 1.414213562373095;
	const double PREC = 1.0 / (1 << 30) / (1 << 10);
}

# define M_PI 3.14159265358979323846

#define	S_NO_ERROR 0
#define	S_SPATIAL_REF_CONFLICT 1
#define	S_WRONG_SHAPEFILE_PATH 2
#define S_DIRECTORY_CREATION_ERROR 3
#define	S_UNKNOWN_ERROR 4
