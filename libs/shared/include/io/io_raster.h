#pragma once
#include "defs.h"
#include "export_io_data.h"
#include "GDAL_OPENCV_IO.h"
#include "coords.h"
#include "lightweight/geoimage.h"

namespace LxGeo
{

	namespace IO_DATA
	{
		namespace ns_RasterCompFlags {
			enum RasterCompFlags
			{
				m_pixelSize = 1 << 0,
				raster_size = 1 << 1,
				band_count = 1 << 2,
				spatial_refrence = 1 << 3,
				raster_data_type = 1 << 4,
				raster_X_size = 1 << 5,
				raster_Y_size = 1 << 6,
				geotransform = 1 << 7,
				ALL = m_pixelSize | raster_size | band_count | spatial_refrence | raster_data_type | raster_X_size | raster_Y_size | geotransform
			};

			inline RasterCompFlags operator|(RasterCompFlags a, RasterCompFlags b)
			{
				return static_cast<RasterCompFlags>(static_cast<int>(a) | static_cast<int>(b));
			}
		}
		
		using namespace ns_RasterCompFlags;

		class RasterIO
		{

		public:

			IO_DATA_API RasterIO() {
				bounding_box = Boost_Box_2();
				m_pixelSize = 0;
				raster_size = 0;
				band_count = 0;
				spatial_refrence = NULL;
				//geotransform = { 0,0,0,0,0,0 };
				raster_data_type = GDT_Unknown;
				raster_data = matrix();
				raster_dataset = NULL;
			};

			IO_DATA_API RasterIO(std::string _raster_path, GDALAccess read_mode = GA_ReadOnly, bool lazy_load = true): RasterIO() {
				load_raster(_raster_path, read_mode = read_mode, lazy_load = lazy_load);
			};

			IO_DATA_API RasterIO(RasterIO& copy_raster) {
				bounding_box = Boost_Box_2(copy_raster.bounding_box);
				m_pixelSize = copy_raster.m_pixelSize;
				raster_X_size = copy_raster.raster_X_size;
				raster_Y_size = copy_raster.raster_Y_size;
				raster_size = copy_raster.raster_size;
				band_count = copy_raster.band_count;
				if (copy_raster.spatial_refrence)
					spatial_refrence = copy_raster.spatial_refrence->Clone();
				else
					spatial_refrence = nullptr;
				for (size_t cpy_idx = 0; cpy_idx < 6; ++cpy_idx) geotransform[cpy_idx] = copy_raster.geotransform[cpy_idx];
				raster_data_type = copy_raster.raster_data_type;
				raster_data = matrix(copy_raster.raster_data);
				raster_dataset = NULL;
			}

			IO_DATA_API RasterIO(RasterIO& copy_raster, matrix& copy_raster_data): RasterIO(copy_raster){
				if (copy_raster_data.cols != raster_X_size || copy_raster_data.rows != raster_Y_size)
					throw std::runtime_error("RasterIO creation failed! matrix and raster size are different!");
				raster_data = copy_raster_data;
				raster_data_type = kgdal2cv.opencv2gdal(copy_raster_data.type());
			}

			IO_DATA_API RasterIO(matrix& copy_raster_data):RasterIO(){
				raster_data_type = kgdal2cv.opencv2gdal(copy_raster_data.type());
				band_count = copy_raster_data.channels();
				raster_data = copy_raster_data;
				raster_X_size = copy_raster_data.cols;
				raster_Y_size = copy_raster_data.rows;
				double new_geotransform[6] = { 0,1,0,0,0,1 };
				std::copy(new_geotransform, new_geotransform + 6, geotransform);
			}

			IO_DATA_API bool compare(RasterIO& target_raster, int flags = RasterCompFlags::ALL, int except_flags =0 ) {
				
				flags = flags & ~except_flags;
				// iterate through members || flags == 0 => all members
				if (flags | RasterCompFlags::m_pixelSize) {
					if (target_raster.m_pixelSize != m_pixelSize) {
						std::cout << "Rasters pxel sizes doesn't match" << std::endl;
						return false;
					}
				}

				if ( flags | RasterCompFlags::raster_size) {
					if (target_raster.raster_size != raster_size) {
						std::cout << "Rasters sizes doesn't match" << std::endl;
						return false;
					}
				}

				if (flags | RasterCompFlags::band_count) {
					if (target_raster.band_count != band_count) {
						std::cout << "Rasters band count doesn't match" << std::endl;
						return false;
					}
				}

				if ( flags | RasterCompFlags::spatial_refrence) {
					if (target_raster.spatial_refrence->IsSame(spatial_refrence)) {
						std::cout << "Rasters spatial refrence doesn't match" << std::endl;
						return false;
					}
				}

				if ( flags | RasterCompFlags::raster_data_type) {
					if (target_raster.raster_data_type != raster_data_type) {
						std::cout << "Rasters data type doesn't match" << std::endl;
						return false;
					}
				}

				if (flags | RasterCompFlags::raster_X_size) {
					if (target_raster.raster_X_size != raster_X_size) {
						std::cout << "Rasters X size doesn't match" << std::endl;
						return false;
					}
				}

				if ( flags | RasterCompFlags::raster_Y_size) {
					if (target_raster.raster_Y_size != raster_Y_size) {
						std::cout << "Rasters Y size doesn't match" << std::endl;
						return false;
					}
				}

				if (flags | RasterCompFlags::geotransform) {
					if (std::equal(std::begin(target_raster.geotransform), std::end(target_raster.geotransform), std::begin(geotransform))){
						std::cout << "Rasters geotransform doesn't match" << std::endl;
						return false;
					}
				}
				return true;
			}

			IO_DATA_API void close() {
				if (raster_dataset)
					GDALClose((GDALDatasetH)raster_dataset);
			}

			IO_DATA_API ~RasterIO() {
				//close();
			};

			IO_DATA_API double get_pixel_width() {
				return std::abs(geotransform[1]);
			}

			IO_DATA_API double get_pixel_height() {
				return std::abs(geotransform[5]);
			}

			IO_DATA_API Boost_Point_2 get_origin_point() {
				return Boost_Point_2(geotransform[0], geotransform[3]);
			}

			IO_DATA_API auto get_matrix_transformer(double x_shift=0.0, double y_shift=0.0) {
				return bg::strategy::transform::matrix_transformer<double, 2, 2>( geotransform[1], geotransform[2], geotransform[0]+ x_shift,
					geotransform[4], geotransform[5], geotransform[3]+ y_shift,
					0.0, 0.0, 1.0);
			}

			IO_DATA_API auto get_inv_matrix_transformer(double x_shift = 0.0, double y_shift = 0.0) {
				return bg::strategy::transform::inverse_transformer<double, 2, 2>(get_matrix_transformer());
			}

			IO_DATA_API bool load_raster(std::string _raster_path, GDALAccess read_mode= GA_ReadOnly, bool lazy_load = true);

			IO_DATA_API void write_raster(std::string raster_path, bool force_write);

			IO_DATA_API GDALDataset* create_copy_dataset(std::string raster_path, GDALDataType _out_data_type= GDT_Unknown, size_t _band_count=NULL);

			template <typename coord_type>
			void _calc_pixel_coords(const double& sc_x, const double& sc_y, coord_type& px_col, coord_type& px_row) {
				double& px = geotransform[0];
				double& py = geotransform[3];
				double& rx = geotransform[1];
				double& ry = geotransform[5];
				px_col = static_cast<coord_type>((sc_x - px) / rx);
				px_row = static_cast<coord_type>((sc_y - py) / ry);
			}

			IO_DATA_API PixelCoords get_pixel_coords(const SpatialCoords& sp) {				
				PixelCoords result_coords;
				_calc_pixel_coords(sp.xc, sp.yc, result_coords.col, result_coords.row);
				return result_coords;
			}

			IO_DATA_API PixelCoords get_pixel_coords(const Boost_Point_2& sp) {

				PixelCoords result_coords;
				_calc_pixel_coords(sp.get<0>(), sp.get<1>(), result_coords.col, result_coords.row);
				return result_coords;
			}

			IO_DATA_API void get_pixel_coords(const Boost_Point_2& sp, cv::Point& cvp) {
				_calc_pixel_coords(sp.get<0>(), sp.get<1>(), cvp.x, cvp.y);
			}

			IO_DATA_API cv::Point get_cv_point(const SpatialCoords& sp) {

				cv::Point result_coords;
				_calc_pixel_coords(sp.xc, sp.yc, result_coords.x, result_coords.y);
				return result_coords;
			}

			IO_DATA_API cv::Point get_cv_point(const Boost_Point_2& sp) {

				cv::Point result_coords;
				_calc_pixel_coords(sp.get<0>(), sp.get<1>(), result_coords.x, result_coords.y);
				return result_coords;
			}

			
			void _calc_spatial_coords(const int& px_col, const int& px_row, double& sc_x, double& sc_y) {
				double& px = geotransform[0];
				double& py = geotransform[3];
				double& rx = geotransform[1];
				double& ry = geotransform[5];
				sc_x = px_col * rx + px;
				sc_y = px_row * ry + py;
			}

			IO_DATA_API SpatialCoords get_spatial_coords(const PixelCoords& pc) {
				SpatialCoords result_coords;
				_calc_spatial_coords(pc.col, pc.row, result_coords.xc, result_coords.yc);				
				return result_coords;
			}
			
			IO_DATA_API SpatialCoords get_spatial_coords(const int& col, const int& row) {
				SpatialCoords result_coords;
				_calc_spatial_coords(col, row, result_coords.xc, result_coords.yc);
				return result_coords;
			}

			IO_DATA_API static GDALDataset* create_dataset(std::string& dataset_path, GDALDriver* gdal_driver, GDALDataType gdal_datatype, size_t raster_X_size, size_t raster_Y_size,
				double geotransform[6], size_t band_count, const OGRSpatialReference* srs, char** papzoptions);

			IO_DATA_API static GDALDataset* create_dataset(std::string& dataset_path, GDALDriver* gdal_driver, GDALDataType gdal_datatype, 
				OGREnvelope raster_extents, double pixel_x_size, double pixel_y_size, size_t band_count, const OGRSpatialReference* srs, char** papzoptions);

			template <typename cv_mat_type>
			GeoImage<cv_mat_type> get_view_pixel(const int& xStart, const int& yStart, const int& xSize, const int& ySize) {
				int left_pad = -std::min<int>(xStart, 0);
				int right_pad = std::max<int>(xStart+xSize, raster_X_size)-raster_X_size;
				int top_pad = -std::min<int>(yStart, 0);
				int down_pad = std::max<int>(yStart + ySize, raster_Y_size) - raster_Y_size;

				matrix non_padded_data;
				if (!raster_data.empty()) {
					non_padded_data = raster_data(cv::Rect(xStart + left_pad, yStart + top_pad, xSize - right_pad, ySize - down_pad));
				}
				else {
					// May be corrected due wrong values defintion below (check geoimage read padded)
					non_padded_data = kgdal2cv.ImgReadByGDAL(raster_path, xStart + left_pad, yStart + top_pad, xSize - right_pad, ySize - down_pad);
				}
				matrix padded_data;
				if (left_pad || right_pad || top_pad || down_pad)
					copyMakeBorder(non_padded_data, padded_data, top_pad, down_pad, left_pad, right_pad, cv::BORDER_CONSTANT, cv::Scalar(0));
				else
					padded_data = non_padded_data;

				GeoImage<cv_mat_type> out_geoimage;
				out_geoimage.set_image(padded_data);
				auto new_origin = get_spatial_coords( xStart, yStart );
				out_geoimage.geotransform[0] = new_origin.xc;
				out_geoimage.geotransform[1] = geotransform[1];
				out_geoimage.geotransform[2] = geotransform[2];
				out_geoimage.geotransform[3] = new_origin.yc;
				out_geoimage.geotransform[4] = geotransform[4];
				out_geoimage.geotransform[5] = geotransform[5];
				return out_geoimage;
			}
			template <typename cv_mat_type>
			GeoImage<cv_mat_type> get_view_spatial(const double& xmin, const double& ymin, const double& xmax, const double& ymax){
				double col_start_subpixel, col_end_subpixel, row_start_subpixel, row_end_subpixel;
				_calc_pixel_coords(xmin, ymax, col_start_subpixel, row_start_subpixel);
				_calc_pixel_coords(xmax, ymin, col_end_subpixel, row_end_subpixel);
				int col_start = (int)std::round(col_start_subpixel), col_end = (int)std::round(col_end_subpixel);
				int row_start = (int)std::round(row_start_subpixel), row_end = (int)std::round(row_end_subpixel);
				return get_view_pixel<cv_mat_type>(col_start, row_start, col_end - col_start, row_end - row_start);
			}

		public:
			KGDAL2CV kgdal2cv;
			std::string raster_path;
			Boost_Box_2 bounding_box;
			double m_pixelSize;
			size_t raster_X_size;
			size_t raster_Y_size;
			size_t raster_size;
			size_t band_count;
			GDALDataset* raster_dataset;
			OGRSpatialReference* spatial_refrence;
			double geotransform[6]; // xoff, a, b, yoff, d, e = geotransform
			GDALDataType raster_data_type;
			matrix raster_data;
			cv::Rect pixel_extent;
		};
		
	}
}