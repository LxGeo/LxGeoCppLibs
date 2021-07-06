#pragma once
#include <gdal_priv.h>
#include <gdal.h>
#include "export_io_data.h"

#include <opencv2/core/core.hpp>

class KGDAL2CV
{
public:
	IO_DATA_API KGDAL2CV();
	IO_DATA_API ~KGDAL2CV();
	IO_DATA_API bool ImgWriteByGDAL(GDALDataset*, const cv::Mat, int = 0, int = 0);
	IO_DATA_API bool ImgWriteByGDAL(GDALRasterBand*, const cv::Mat, int = 0, int = 0);
	cv::Mat ImgReadByGDAL(cv::String, bool = true);
	cv::Mat ImgReadByGDAL(cv::String, int, int, int, int, bool = true);
	cv::Mat ImgReadByGDAL(GDALRasterBand*, int, int, int, int);
	cv::Mat ImgReadByGDAL(GDALRasterBand*);
	IO_DATA_API int gdal2opencv(const GDALDataType&, const int& channels);
	IO_DATA_API GDALDataType KGDAL2CV::opencv2gdal(const int cvType);
	IO_DATA_API void Close();
private:
	GDALDataset* m_dataset;
	cv::String m_filename;
	GDALDriver* m_driver;
	bool hasColorTable;

	int m_width;
	int m_height;

	int m_type;
	int m_nBand;

	bool readHeader();
	bool readData(cv::Mat img);
	int gdalPaletteInterpretation2OpenCV(GDALPaletteInterp const&, GDALDataType const&);
	void write_ctable_pixel(const double&, const GDALDataType&, GDALColorTable const*, cv::Mat&, const int&, const int&, const int&);
	void write_pixel(const double&, const GDALDataType&, const int&, cv::Mat&, const int&, const int&, const int&);
	double range_cast(const GDALDataType&, const int&, const double&);
	double range_cast_inv(const GDALDataType&, const int&, const double&);
	bool CheckDataType(const GDALDataType&, cv::Mat);
	KGDAL2CV(const KGDAL2CV&);
};


IO_DATA_API extern KGDAL2CV* kgdal2cv;