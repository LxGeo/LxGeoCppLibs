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

			GeoImage() {};
			template <typename cv_mat_type__>
			GeoImage(cv_mat_type__& _image, double _geotransform[6]) {
				set_image(_image);
				set_geotransform(_geotransform);
			}

			void set_geotransform(double _geotransform[6]) {
				for (size_t g_idx = 0; g_idx < 6; g_idx++)
					geotransform[g_idx] = _geotransform[g_idx];
			}
			
			template <typename cv_mat_type__>
			void set_image(const cv_mat_type__& in_image) {

				if constexpr (std::is_same_v<cv_mat_type, cv_mat_type__>) {
					// behaviour if input image type correspond to class image type
					image = in_image;
				}
				else {
					if constexpr (std::is_same_v<cv_mat_type, cv::Mat>) {
						// Behavior for cpu_matrix type
						in_image.download(image);
					}
					else if constexpr (std::is_same_v<cv_mat_type, cv::cuda::GpuMat>) {
						// Behavior for gpu_matrix type
						image.upload(in_image);
					}
				}
			}
			//void set_image(cv_mat_type& in_image) {
			//	image = in_image;
			//}

			template <typename coord_type>
			void _calc_pixel_coords(const double& sc_x, const double& sc_y, coord_type& px_col, coord_type& px_row) {
				double& px = geotransform[0];
				double& py = geotransform[3];
				double& rx = geotransform[1];
				double& ry = geotransform[5];
				px_col = static_cast<coord_type>((sc_x - px) / rx);
				px_row = static_cast<coord_type>((sc_y - py) / ry);
			}

			void _calc_spatial_coords(const int& px_col, const int& px_row, double& sc_x, double& sc_y) {
				double& px = geotransform[0];
				double& py = geotransform[3];
				double& rx = geotransform[1];
				double& ry = geotransform[5];
				sc_x = px_col * rx + px;
				sc_y = px_row * ry + py;
			}

			template <typename cv_mat_type>
			GeoImage<cv_mat_type> get_view_pixel(const int& xStart, const int& yStart, const int& xSize, const int& ySize) {
				int left_pad = -std::min<int>(xStart, 0);
				int right_pad = std::max<int>(xStart + xSize, image.cols) - image.cols;
				int top_pad = -std::min<int>(yStart, 0);
				int down_pad = std::max<int>(yStart + ySize, image.rows) - image.rows;

				
				cv::Rect view_rect(xStart + left_pad, yStart + top_pad, xSize - right_pad, ySize - down_pad);
				cv_mat_type non_padded_data(image, view_rect);

				cv_mat_type padded_data;
				if (left_pad || right_pad || top_pad || down_pad)
					cv::cuda::copyMakeBorder(non_padded_data, padded_data, top_pad, down_pad, left_pad, right_pad, cv::BORDER_CONSTANT, cv::Scalar(0));
				else
					padded_data = non_padded_data;

				double new_origin_x, new_origin_y;
				_calc_spatial_coords(xStart, yStart, new_origin_x, new_origin_y);
				double _geotransform[6] = { 
					new_origin_x , geotransform[1], geotransform[2],
					new_origin_y, geotransform[4], geotransform[5] };
				GeoImage<cv_mat_type> out_geoimage;
				out_geoimage.set_image(padded_data);
				out_geoimage.set_geotransform(_geotransform);
				return out_geoimage;
			};

			template <typename cv_mat_type>
			GeoImage<cv_mat_type> get_view_spatial(const double& xmin, const double& ymin, const double& xmax, const double& ymax) {
				double col_start_subpixel, col_end_subpixel, row_start_subpixel, row_end_subpixel;
				_calc_pixel_coords(xmin, ymax, col_start_subpixel, row_start_subpixel);
				_calc_pixel_coords(xmax, ymin, col_end_subpixel, row_end_subpixel);
				const int col_start = (int)std::round(col_start_subpixel), col_end = (int)std::round(col_end_subpixel);
				const int row_start = (int)std::round(row_start_subpixel), row_end = (int)std::round(row_end_subpixel);
				return get_view_pixel<cv_mat_type>(col_start, row_start, col_end - col_start, row_end - row_start);
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