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
#include <random>
#include <boost/log/trivial.hpp>

#include "defs_common.h"
#include "defs_boost.h"
#include "defs_opencv.h"
#include "defs_cgal.h"
#include "defs_ogr.h"
#include "geometry_lab.h"
#include <fmt/core.h>
#include "tqdm/tqdm.h"
#include "coords.h"

//using namespace LxGeo::LxGeoCommon;
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

		template<typename T>
		T vmin(T&& t)
		{
			return std::forward<T>(t);
		}

		template<typename T0, typename T1, typename... Ts>
		typename std::common_type<
			T0, T1, Ts...
		>::type vmin(T0&& val1, T1&& val2, Ts&&... vs)
		{
			if (val2 < val1)
				return vmin(val2, std::forward<Ts>(vs)...);
			else
				return vmin(val1, std::forward<Ts>(vs)...);
		}

		template<typename T>
		T vmax(T&& t)
		{
			return std::forward<T>(t);
		}

		template<typename T0, typename T1, typename... Ts>
		typename std::common_type<
			T0, T1, Ts...
		>::type vmax(T0&& val1, T1&& val2, Ts&&... vs)
		{
			if (val2 > val1)
				return vmax(val2, std::forward<Ts>(vs)...);
			else
				return vmax(val1, std::forward<Ts>(vs)...);
		}

		LX_GEO_FACTORY_SHARED_API std::string random_string(const size_t& length = 10, const std::string& prefix = "", const std::string& suffix = "");

		inline double RADS(const double& in_deg){return in_deg * (M_PI / 180);}
		inline double DEGS(const double& in_rad) { return in_rad * (180/ M_PI); }

		LX_GEO_FACTORY_SHARED_API std::shared_ptr<GDALDataset> load_gdal_dataset_shared_ptr(const std::string& raster_file_path);

	}
}


namespace Constants
{
	const double PI = 3.1415926535897832384626;
	const double SQRT_2 = 1.414213562373095;
	const double PREC = 1.0 / (1 << 30) / (1 << 10);
	const size_t MEAN_PTS_PER_POLYGON = 7;
}

namespace gdalConstants {
	const std::set<std::string> gdal_types_set = {
		"Byte", "Int8", "UInt16", "Int16", "UInt32", "Int32", "UInt64", "Int64", "Float32", "Float64", "CInt16", "CInt32", "CFloat32" , "CFloat64"
	};
}

# define M_PI 3.14159265358979323846

#define	S_NO_ERROR 0
#define	S_SPATIAL_REF_CONFLICT 1
#define	S_WRONG_SHAPEFILE_PATH 2
#define S_DIRECTORY_CREATION_ERROR 3
#define	S_UNKNOWN_ERROR 4
