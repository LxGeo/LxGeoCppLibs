#pragma once
#include "defs.h"

struct SpatialCoords {
	double xc;
	double yc;

	SpatialCoords operator+(const SpatialCoords& sp2) { return { xc+sp2.xc , sp2.yc }; }
	SpatialCoords& operator+=(const SpatialCoords& rhs) { xc += rhs.xc; yc += rhs.yc; return *this; }
	SpatialCoords& operator/=(const double& N) { xc /= N; yc /= N; return *this; }
	SpatialCoords operator/(const double& N) { return { xc / N, yc / N }; }
};

struct PixelCoords {
	size_t col;
	size_t row;
};


auto spatial_coords_mean_aggregator = [](std::list<SpatialCoords>& pts_coords) {
	SpatialCoords mean_coord = { 0.0,0.0 };
	for (auto& c_coord : pts_coords) mean_coord += (c_coord / pts_coords.size());
	return mean_coord;
};

auto median = [](std::vector<double>& values)->double {
	if (values.empty())
		return 0.0;

	if (values.size() % 2 == 0) {
		const auto median_it1 = values.begin() + values.size() / 2 - 1;
		const auto median_it2 = values.begin() + values.size() / 2;

		std::nth_element(values.begin(), median_it1, values.end());
		const auto e1 = *median_it1;

		std::nth_element(values.begin(), median_it2, values.end());
		const auto e2 = *median_it2;

		return (e1 + e2) / 2;

	}
	else {
		const auto median_it = values.begin() + values.size() / 2;
		std::nth_element(values.begin(), median_it, values.end());
		return *median_it;
	}
};

auto spatial_coords_median_aggregator = [](std::list<SpatialCoords>& pts_coords) {

	std::vector<double> xcs, ycs; xcs.reserve(pts_coords.size()); ycs.reserve(pts_coords.size());

	for(auto& sc: pts_coords) {
		if (sc.xc!=0) xcs.push_back(sc.xc);
		if (sc.yc != 0) ycs.push_back(sc.yc);
	}

	SpatialCoords median_coord = { median(xcs), median(ycs) };
	return median_coord;
};
