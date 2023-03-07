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

			void set_geotransform(const double _geotransform[6]) {
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
			void _calc_pixel_coords(const double& sc_x, const double& sc_y, coord_type& px_col, coord_type& px_row) const  {
				const double& px = geotransform[0];
				const double& py = geotransform[3];
				const double& rx = geotransform[1];
				const double& ry = geotransform[5];
				const double& nx = geotransform[2];
				const double& ny = geotransform[4];
				//px_col = static_cast<coord_type>((sc_x - px) / rx);
				//px_row = static_cast<coord_type>((sc_y - py) / ry);

				px_col = (ny) ? static_cast<coord_type>((sc_x - px) / rx + (sc_y - py) / ny) : static_cast<coord_type>((sc_x - px) / rx);
				px_row = (nx) ? static_cast<coord_type>((sc_y - py) / ry + (sc_x - px) / nx) : static_cast<coord_type>((sc_y - py) / ry);
			}

			void _calc_spatial_coords(const int& px_col, const int& px_row, double& sc_x, double& sc_y) const {
				const double& px = geotransform[0];
				const double& py = geotransform[3];
				const double& rx = geotransform[1];
				const double& ry = geotransform[5];
				const double& nx = geotransform[2];
				const double& ny = geotransform[4];
				sc_x = px_col * rx + px + px_row*nx;
				sc_y = px_row * ry + py + px_col*ny;
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

			void to_file(const std::string& out_file, OGRSpatialReference* spatial_refrence = nullptr, GDALDataType* ptr_out_data_type = nullptr) {
				KGDAL2CV kgdal2cv;

				// create gdal dataset
				GDALDriver* tiff_driver = GetGDALDriverManager()->GetDriverByName("GTiff");
				GDALDataType out_data_type;
				if (ptr_out_data_type) out_data_type = *ptr_out_data_type;
				else out_data_type = kgdal2cv.opencv2gdal(image.type());
				
				GDALDataset* new_dataset = tiff_driver->Create(out_file.c_str(), image.cols, image.rows, image.channels(), out_data_type, NULL);
				if (new_dataset == NULL) { throw std::exception(fmt::format("Cannot create dataset at {}", out_file).c_str()); }
				if (spatial_refrence) new_dataset->SetSpatialRef(spatial_refrence);
				
				new_dataset->SetGeoTransform(geotransform);
				cv::Mat to_save_image;
				if constexpr (std::is_same_v<cv_mat_type, cv::Mat>)
					to_save_image = image;
				else
					image.download(to_save_image);

				to_save_image.convertTo(to_save_image, kgdal2cv.gdal2opencv(out_data_type, image.channels()));
				kgdal2cv.ImgWriteByGDAL(new_dataset, to_save_image, 0, 0);
				GDALClose(new_dataset);
			}

			static GeoImage<cv_mat_type> from_file(const std::string& in_file, const OGREnvelope& spatial_envelope) {
				GeoImage<cv_mat_type> loaded_gimg;
				GDALDataset* raster_dataset = (GDALDataset*)GDALOpen(in_file.c_str(), GA_ReadOnly);
				raster_dataset->GetGeoTransform(loaded_gimg.geotransform);
				GDALClose((GDALDatasetH)raster_dataset);
				int col_start_pixel, col_end_pixel, row_start_pixel, row_end_pixel;
				loaded_gimg._calc_pixel_coords(spatial_envelope.MinX, spatial_envelope.MaxY, col_start_pixel, row_start_pixel);
				loaded_gimg._calc_pixel_coords(spatial_envelope.MaxX, spatial_envelope.MinY, col_end_pixel, row_end_pixel);
				KGDAL2CV kgdal2cv;
				cv::Mat loaded_image = kgdal2cv.PaddedImgReadByGDAL(in_file, col_start_pixel, row_start_pixel, col_end_pixel-col_start_pixel, row_end_pixel-row_start_pixel);
				loaded_gimg.set_image(loaded_image);
				// fix geotransform because of padding
				loaded_gimg.geotransform[0] = spatial_envelope.MinX; loaded_gimg.geotransform[3] = spatial_envelope.MaxY;
				return loaded_gimg;
			}

			static GeoImage<cv_mat_type> from_file(const std::string& in_file) {
				GeoImage<cv_mat_type> loaded_gimg;
				GDALDataset* raster_dataset = (GDALDataset*)GDALOpen(in_file.c_str(), GA_ReadOnly);
				raster_dataset->GetGeoTransform(loaded_gimg.geotransform);
				GDALClose((GDALDatasetH)raster_dataset);
				KGDAL2CV kgdal2cv;
				cv::Mat loaded_image = kgdal2cv.ImgReadByGDAL(in_file);
				loaded_gimg.set_image(loaded_image);
				return loaded_gimg;
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


		template <typename cv_mat_type>
		GeoImage<cv_mat_type> rotate(const GeoImage<cv_mat_type>& gimg, const double& rotation_angle) {
			// get rotation matrix for rotating the image around its center
			cv::Point2f rotation_center(gimg.image.cols / 2.0, gimg.image.rows / 2.0);
			cv::Mat rot = cv::getRotationMatrix2D(rotation_center, rotation_angle, 1.0);
			// determine bounding rectangle
			cv::Rect bbox = cv::RotatedRect(rotation_center, gimg.image.size(), rotation_angle).boundingRect();
			// adjust transformation matrix
			rot.at<double>(0, 2) += bbox.width / 2.0 - rotation_center.x;
			rot.at<double>(1, 2) += bbox.height / 2.0 - rotation_center.y;

			GeoImage<cv_mat_type> rotated_gimg;
			if constexpr (std::is_same_v<cv_mat_type, cv::Mat>) {
				// Behavior for cpu_matrix type
				cv::warpAffine(gimg.image, rotated_gimg.image, rot, bbox.size(), cv::INTER_CUBIC);
			}
			else if constexpr (std::is_same_v<cv_mat_type, cv::cuda::GpuMat>) {
				// Behavior for gpu_matrix type
				cv::cuda::warpAffine(gimg.image, rotated_gimg.image, rot, bbox.size(), cv::INTER_CUBIC);
			}

			// assign updated geotransform
			cv::Mat rot_inv; cv::invertAffineTransform(rot, rot_inv);
			cv::Mat input_geotransform_as_matrix; transform_G2CV_affine(gimg.geotransform, input_geotransform_as_matrix);
			cv::Mat product_mat = multiply_affine_matrices(input_geotransform_as_matrix, rot_inv);
			transform_CV2G_affine(product_mat, rotated_gimg.geotransform);

			return rotated_gimg;
		}


	}
}