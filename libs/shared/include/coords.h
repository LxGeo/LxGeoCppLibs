#pragma once

struct SpatialCoords {
	double xc;
	double yc;

	SpatialCoords& operator+=(const SpatialCoords& rhs) { xc += rhs.xc; yc += rhs.yc; return *this; }
	SpatialCoords& operator/=(const double& N) { xc /= N; yc /= N; return *this; }
	SpatialCoords operator/(const double& N) { return { xc / N, yc / N }; }
};

struct PixelCoords {
	size_t col;
	size_t row;
};