#pragma once
#include "defs.h"

struct SpatialCoords {
	double xc;
	double yc;

	SpatialCoords operator+(const SpatialCoords& sp2) { return { xc+sp2.xc , sp2.yc }; }
	SpatialCoords& operator+=(const SpatialCoords& rhs) { xc += rhs.xc; yc += rhs.yc; return *this; }
	SpatialCoords& operator/=(const double& N) { xc /= N; yc /= N; return *this; }
	SpatialCoords operator/(const double& N) { return { xc / N, yc / N }; }
	SpatialCoords operator*(const double& N) { return { xc * N, yc * N }; }
};

struct PixelCoords {
	size_t col;
	size_t row;
};


inline std::function<SpatialCoords(std::list<SpatialCoords>&)> spatial_coords_mean_aggregator = [](std::list<SpatialCoords>& pts_coords)->SpatialCoords {
	SpatialCoords mean_coord = { 0.0,0.0 };
	for (auto& c_coord : pts_coords) mean_coord += (c_coord / pts_coords.size());
	return mean_coord;
};

inline std::function<SpatialCoords(std::list<SpatialCoords>&, std::list<double>&)> spatial_coords_weighted_mean_aggregator = [](std::list<SpatialCoords>& pts_coords, std::list<double>& pts_weights) {
	SpatialCoords mean_coord = { 0.0,0.0 };
	auto coords_iter = pts_coords.begin();
	auto weight_iter = pts_weights.begin();
	double sum_weight = 0;
	for (; coords_iter != pts_coords.end() && weight_iter != pts_weights.end(); ++coords_iter, ++weight_iter) {
		mean_coord += (*coords_iter) * (*weight_iter);
		sum_weight += *weight_iter;
	}
	return mean_coord/ sum_weight;
};

inline auto median = [](std::vector<double>& values)->double {
	if (values.empty())
		return 0.0;

	if (values.size() % 2 == 0) {
		auto median_it1 = values.begin() + values.size() / 2 - 1;
		auto median_it2 = values.begin() + values.size() / 2;

		std::nth_element(values.begin(), median_it1, values.end());
		auto e1 = *median_it1;

		std::nth_element(values.begin(), median_it2, values.end());
		auto e2 = *median_it2;

		return (e1 + e2) / 2;

	}
	else {
		const auto median_it = values.begin() + values.size() / 2;
		std::nth_element(values.begin(), median_it, values.end());
		return *median_it;
	}
};

inline std::function<SpatialCoords(const std::list<SpatialCoords>&)> spatial_coords_median_aggregator = [](const std::list<SpatialCoords>& pts_coords) {

	std::vector<double> xcs, ycs; xcs.reserve(pts_coords.size()); ycs.reserve(pts_coords.size());

	for(const auto& sc: pts_coords) {
		if (sc.xc!=0) xcs.push_back(sc.xc);
		if (sc.yc != 0) ycs.push_back(sc.yc);
	}

	SpatialCoords median_coord = { median(xcs), median(ycs) };
	return median_coord;
};
