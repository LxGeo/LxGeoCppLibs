#pragma once
#include "defs.h"
#include "export_io_data.h"
#include "GDAL_OPENCV_IO.h"
#include "coords.h"

namespace LxGeo
{

	namespace IO_DATA
	{

		extern KGDAL2CV* kgdal2cv;

		template <typename cv_mat_type>
		struct GeoImage {
			cv_mat_type image;
			double geotransform[6];

			void set_image(cv::Mat& in_image) {				
				if constexpr (std::is_same_v<cv_mat_type, cv::Mat>) {
					// Behavior for cpu_matrix type
					image = in_image;
				}
				else if constexpr (std::is_same_v<cv_mat_type, cv::cuda::GpuMat>) {
					// Behavior for gpu_matrix type
					image.upload(in_image);
				}
			}
		};

		template <typename cv_mat_type>
		OGREnvelope bounds(const GeoImage<cv_mat_type>& gimg) {
			double xs = geoTransform[0];
			double ys = geoTransform[3];
			double xe = xs + geoTransform[1] * gimg.image.cols;
			double ye = ys + geoTransform[5] * data.rows;

			OGREnvelope bounds;
			bounds.MinX = std::min<double>(xs, xe); bounds.MaxX = std::max<double>(xs, xe);
			bounds.MinY = std::min<double>(ys, ye); bounds.MaxY = std::max<double>(ys, ye);
			return bounds;
		}


	}
}