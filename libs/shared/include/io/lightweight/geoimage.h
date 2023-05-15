#pragma once
#include "defs_ogr.h"
#include "defs_opencv.h"
#include "lightweight/raster_profile.h"
#include "export_io_data.h"
#include "GDAL_OPENCV_IO.h"
#include "coords.h"
#include "spatial_coord_transformer.h"

namespace LxGeo
{

	namespace IO_DATA
	{

		extern KGDAL2CV* kgdal2cv;

		template <typename cv_mat_type>
		struct GeoImage {
			cv_mat_type image;
			double geotransform[6];
			std::optional<double> no_data;
			std::string crs_wkt;

			GeoImage() {};
			template <typename cv_mat_type__>
			GeoImage(const cv_mat_type__& _image, const double _geotransform[6]) {
				set_image(_image);
				set_geotransform(_geotransform);
			}

			GeoImage(const GeoImage& ref_gimg) { // check this for copy leaks
				image = ref_gimg.image.clone();
				set_geotransform(ref_gimg.geotransform);
				no_data = ref_gimg.no_data;
				crs_wkt = ref_gimg.crs_wkt;
			};

			GeoImage(GeoImage&& other) noexcept {				
				image = std::move(other.image);
				no_data = std::move(other.no_data);
				crs_wkt = std::move(other.crs_wkt);
				set_geotransform(other.geotransform);
			}

			GeoImage& operator=(GeoImage&& other) noexcept {
				if (this != &other) {
					image = std::move(other.image);
					no_data = std::move(other.no_data);
					crs_wkt = std::move(other.crs_wkt);
					set_geotransform(other.geotransform);
				}
				return *this;
			}

			void set_geotransform(const double _geotransform[6]) {
				memcpy(geotransform, _geotransform, sizeof(double) * 6);
			}

			template <typename cv_mat_type__>
			void set_image(const cv_mat_type__& in_image) {

				if constexpr (std::is_same_v<cv_mat_type, cv_mat_type__>) {
					// behaviour if input image type correspond to class image type
					image = std::move(in_image); // was simple assignment && changed to move
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

			cv::Mat get_image() const {
				if constexpr (std::is_same_v < cv_mat_type, cv::Mat>)
					return image;
				else if constexpr (std::is_same_v < cv_mat_type, cv::cuda::GpuMat>)
				{
					cv::Mat cpu_image;
					image.download(cpu_image);
					return cpu_image;
				}
			}

			template <typename coord_type>
			void _calc_pixel_coords(const double& sc_x, const double& sc_y, coord_type& px_col, coord_type& px_row) const {
				AffineTransformerBase trans_obj(geotransform);
				trans_obj._calc_pixel_coords(sc_x, sc_y, px_col, px_row);
			}

			void _calc_spatial_coords(const int& px_col, const int& px_row, double& sc_x, double& sc_y) const {
				AffineTransformerBase trans_obj(geotransform);
				trans_obj._calc_spatial_coords(px_col, px_row, sc_x, sc_y);
			}

			template <typename cv_mat_type>
			GeoImage<cv_mat_type> get_view_pixel(const int& xStart, const int& yStart, const int& xSize, const int& ySize) const {
				int left_pad = -std::min<int>(xStart, 0);
				int right_pad = std::max<int>(xStart + xSize, image.cols) - image.cols;
				int top_pad = -std::min<int>(yStart, 0);
				int down_pad = std::max<int>(yStart + ySize, image.rows) - image.rows;


				cv::Rect view_rect(xStart + left_pad, yStart + top_pad, xSize - left_pad - right_pad, ySize - top_pad - down_pad);
				cv_mat_type non_padded_data(image, view_rect);

				cv_mat_type padded_data;
				if (left_pad || right_pad || top_pad || down_pad) {
					if constexpr (std::is_same_v < cv_mat_type, cv::cuda::GpuMat>)
						cv::cuda::copyMakeBorder(non_padded_data, padded_data, top_pad, down_pad, left_pad, right_pad, cv::BORDER_CONSTANT, cv::Scalar(0));
					else
						cv::copyMakeBorder(non_padded_data, padded_data, top_pad, down_pad, left_pad, right_pad, cv::BORDER_CONSTANT, cv::Scalar(0));
				}
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
				out_geoimage.no_data = no_data;
				out_geoimage.crs_wkt = crs_wkt;
				return out_geoimage;
			};

			template <typename envelope_type>
			GeoImage<cv_mat_type> get_view_spatial(const envelope_type& spatial_envelope) const {
				double MinX, MinY, MaxX, MaxY;
				if constexpr (std::is_same_v<envelope_type, OGREnvelope>) {
					MinX = spatial_envelope.MinX; MinY = spatial_envelope.MinY; MaxX = spatial_envelope.MaxX; MaxY = spatial_envelope.MaxY;
				}
				else if constexpr (std::is_same_v<envelope_type, Boost_Box_2>) {
					MinX = spatial_envelope.min_corner().get<0>();
					MinY = spatial_envelope.min_corner().get<1>();
					MaxX = spatial_envelope.max_corner().get<0>();
					MaxY = spatial_envelope.max_corner().get<1>();
				}
				return get_view_spatial<cv_mat_type>(MinX, MinY, MaxX, MaxY);
			}

			template <typename cv_mat_type>
			GeoImage<cv_mat_type> get_view_spatial(const double& xmin, const double& ymin, const double& xmax, const double& ymax) const{
				double col_start_subpixel, col_end_subpixel, row_start_subpixel, row_end_subpixel;
				_calc_pixel_coords(
					(sign(geotransform[1]) == 1) ? xmin : xmax,
					(sign(geotransform[5]) == 1) ? ymin : ymax,
					col_start_subpixel, row_start_subpixel);
				_calc_pixel_coords(
					(sign(geotransform[1]) == 1) ? xmax : xmin,
					(sign(geotransform[5]) == 1) ? ymax : ymin,
					col_end_subpixel, row_end_subpixel);
				const int col_start = (int)std::round(col_start_subpixel), col_end = (int)std::round(col_end_subpixel);
				const int row_start = (int)std::round(row_start_subpixel), row_end = (int)std::round(row_end_subpixel);
				return get_view_pixel<cv_mat_type>(col_start, row_start, col_end - col_start, row_end - row_start);
			}

			/*void to_file(const std::string& out_file, OGRSpatialReference* spatial_refrence = nullptr, GDALDataType* ptr_out_data_type = nullptr) const {
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
			}*/

			/**
			* Writes geoimage to dataset in the right position
			*
			* @param gdal_dataset a unique pointer to the sink gdal dataset.
			* @param no_cropping_allowed a boolean specifying if cropping is allowed else an exception is thrown.
			* @return void.
			*/
			void to_dataset(std::shared_ptr<GDALDataset> gdal_dataset, bool no_cropping_allowed = false) const {

				double sink_geotransform[6];
				gdal_dataset->GetGeoTransform(sink_geotransform);
				int col_start, row_start, col_end, row_end;
				AffineTransformerBase(sink_geotransform)._calc_pixel_coords(geotransform[0], geotransform[3], col_start, row_start);
				col_end = col_start + image.cols;
				row_end = row_start + image.rows;
				if (
					col_start >= gdal_dataset->GetRasterXSize() ||
					row_start >= gdal_dataset->GetRasterYSize() ||
					col_end < 0 ||
					row_end < 0
					)
					throw std::exception("GeoImage is fully outside the sink dataset!");

				int left_crop = -std::min(col_start, 0);
				int right_crop = std::max(col_end - gdal_dataset->GetRasterXSize(), 0);
				int top_crop = -std::min(row_start, 0);
				int bottom_crop = std::max(row_end - gdal_dataset->GetRasterYSize(), 0);

				bool crop_needed = (left_crop | right_crop | top_crop | bottom_crop);
				if (crop_needed && no_cropping_allowed)
					throw std::exception("GeoImage is not fully included within sink dataset! Try unsetting crop_allowed parameter!");

				cv::Mat cropped_matrix = get_image();
				cropped_matrix = cropped_matrix.colRange(left_crop, image.cols - right_crop).rowRange(top_crop, image.rows - bottom_crop);
				int sink_xstart = col_start + left_crop, sink_ystart = row_start + top_crop;

				KGDAL2CV kgdal2cv;
				kgdal2cv.ImgWriteByGDAL(gdal_dataset.get(), cropped_matrix, sink_xstart, sink_ystart);
			}

			std::shared_ptr<GDALDataset> to_file(const std::string& filepath) const {
				RProfile c_profile = RProfile::from_geoimage(*this);
				auto dataset_ptr = c_profile.to_gdal_dataset(filepath);
				this->to_dataset(dataset_ptr);
				return dataset_ptr;
			}

			template <typename envelope_type>
			static GeoImage<cv_mat_type> from_file(const std::string& in_file, const envelope_type& spatial_envelope) {
				GeoImage<cv_mat_type> loaded_gimg;
				GDALDataset* raster_dataset = (GDALDataset*)GDALOpen(in_file.c_str(), GA_ReadOnly);
				if (raster_dataset->GetGeoTransform(loaded_gimg.geotransform) != CE_None) {
					// temporary fix for the north facing rasters
					loaded_gimg.geotransform[5] = -1.0;
				}
				int raster_has_nodata;
				double nodata_value = raster_dataset->GetRasterBand(1)->GetNoDataValue(&raster_has_nodata);
				std::optional<double> nodata; if (raster_has_nodata) nodata= nodata_value;
				loaded_gimg.no_data = nodata;
				GDALClose((GDALDatasetH)raster_dataset);
				int col_start_pixel, col_end_pixel, row_start_pixel, row_end_pixel;

				/**/
				double MinX, MaxX, MinY, MaxY;
				if constexpr (std::is_same_v<envelope_type, OGREnvelope>) {
					MinX = spatial_envelope.MinX; MaxY = spatial_envelope.MaxY; MaxX = spatial_envelope.MaxX; MinY = spatial_envelope.MinY;
				}
				else if constexpr (std::is_same_v<envelope_type, Boost_Box_2>) {
					MinX = spatial_envelope.min_corner().get<0>(); MaxX = spatial_envelope.max_corner().get<0>();
					MinY = spatial_envelope.min_corner().get<1>(); MaxY = spatial_envelope.max_corner().get<1>();
				}
				int x_dircetion_sign = sign(loaded_gimg.geotransform[1]); // x sign
				int y_direction_sign = sign(loaded_gimg.geotransform[5]); // y sign
				loaded_gimg._calc_pixel_coords(
					(x_dircetion_sign==1) ? MinX : MaxX,
					(y_direction_sign==1) ? MinY: MaxY,
					col_start_pixel, row_start_pixel);
				loaded_gimg._calc_pixel_coords(
					(x_dircetion_sign == 1) ? MaxX : MinX,
					(y_direction_sign == 1) ? MaxY : MinY,
					col_end_pixel, row_end_pixel);

				loaded_gimg.geotransform[0] = (x_dircetion_sign == 1) ? MinX : MaxX;
				loaded_gimg.geotransform[3] = (y_direction_sign == 1) ? MinY : MaxY;
				
				KGDAL2CV kgdal2cv;
				cv::Mat loaded_image = kgdal2cv.PaddedImgReadByGDAL(in_file, col_start_pixel, row_start_pixel, col_end_pixel - col_start_pixel, row_end_pixel - row_start_pixel);
				loaded_gimg.set_image(loaded_image);
				// fix geotransform because of padding
				
				return loaded_gimg;
			}

			static GeoImage<cv_mat_type> from_file(const std::string& in_file) {
				GeoImage<cv_mat_type> loaded_gimg;
				GDALDataset* raster_dataset = (GDALDataset*)GDALOpen(in_file.c_str(), GA_ReadOnly);
				if (raster_dataset->GetGeoTransform(loaded_gimg.geotransform) != CE_None) {
					// temporary fix for the north facing rasters
					loaded_gimg.geotransform[5] = -1.0;
				}
				int raster_has_nodata;
				double nodata_value = raster_dataset->GetRasterBand(1)->GetNoDataValue(&raster_has_nodata);
				std::optional<double> nodata; if (raster_has_nodata) nodata= nodata_value;
				loaded_gimg.no_data = nodata;
				GDALClose((GDALDatasetH)raster_dataset);
				KGDAL2CV kgdal2cv;
				cv::Mat loaded_image = kgdal2cv.ImgReadByGDAL(in_file);
				loaded_gimg.set_image(loaded_image);
				return loaded_gimg;
			}

			static GeoImage<cv_mat_type> from_dataset(std::shared_ptr<GDALDataset> raster_dataset) {
				GeoImage<cv_mat_type> loaded_gimg;
				if (raster_dataset->GetGeoTransform(loaded_gimg.geotransform) != CE_None) {
					// temporary fix for the north facing rasters
					loaded_gimg.geotransform[5] = -1.0;
				}
				int raster_has_nodata;
				double nodata_value = raster_dataset->GetRasterBand(1)->GetNoDataValue(&raster_has_nodata);
				std::optional<double> nodata; if (raster_has_nodata) nodata= nodata_value;
				loaded_gimg.no_data = nodata;
				KGDAL2CV kgdal2cv;
				// TODO: this should be changed by adding a method in kgdal to load from dataset
				std::string file_name(raster_dataset->GetFileList()[0]);
				cv::Mat loaded_image = kgdal2cv.ImgReadByGDAL(file_name);
				loaded_gimg.set_image(loaded_image);
				return loaded_gimg;
			}

		};

		template <typename cv_mat_type>
		OGREnvelope bounds(const GeoImage<cv_mat_type>& gimg) {
			double xs = gimg.geotransform[0];
			double ys = gimg.geotransform[3];
			double xe = xs + gimg.geotransform[1] * gimg.image.cols;
			double ye = ys + gimg.geotransform[5] * gimg.image.rows;

			OGREnvelope bounds;
			bounds.MinX = std::min<double>(xs, xe); bounds.MaxX = std::max<double>(xs, xe);
			bounds.MinY = std::min<double>(ys, ye); bounds.MaxY = std::max<double>(ys, ye);
			return bounds;
		}


		template <typename cv_mat_type>
		GeoImage<cv_mat_type> rotate(const GeoImage<cv_mat_type>& gimg, const double& rotation_angle, int warp_mode = cv::INTER_CUBIC) {
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
				cv::warpAffine(gimg.image, rotated_gimg.image, rot, bbox.size(), warp_mode);
			}
			else if constexpr (std::is_same_v<cv_mat_type, cv::cuda::GpuMat>) {
				// Behavior for gpu_matrix type
				cv::cuda::warpAffine(gimg.image, rotated_gimg.image, rot, bbox.size(), warp_mode);
			}

			// assign updated geotransform
			cv::Mat rot_inv; cv::invertAffineTransform(rot, rot_inv);
			cv::Mat input_geotransform_as_matrix; transform_G2CV_affine(gimg.geotransform, input_geotransform_as_matrix);
			cv::Mat product_mat = multiply_affine_matrices(input_geotransform_as_matrix, rot_inv);
			transform_CV2G_affine(product_mat, rotated_gimg.geotransform);

			return rotated_gimg;
		}

		template <typename cv_mat_type>
		struct VirtualGeoImage : GeoImage<cv_mat_type> {
			/*
			* Constructor using a geoimage & cv::mat transformer & geotransform alteration
			*/
			VirtualGeoImage(const GeoImage<cv_mat_type>& _ref_gimg,
				const std::function<cv_mat_type(const cv_mat_type&)>& image_transform_functor,
				const std::function<void(double[6])>& geotransform_alteration_functor = [](double[6]){}
			) : GeoImage<cv_mat_type>(image_transform_functor(_ref_gimg.image), _ref_gimg.geotransform) {
				geotransform_alteration_functor(this->geotransform);
			};

		};


	}
}