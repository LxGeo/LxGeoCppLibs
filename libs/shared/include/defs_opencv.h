#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include "opencv2/cudawarping.hpp"
#include "export_shared.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		typedef cv::Mat matrix;

        template<int _depth> class TypeDepth
        {
        public:
            enum { depth = CV_USRTYPE1 };
            typedef void value_type;
        };

        template<> class TypeDepth<CV_8U>
        {
        public:
            enum { depth = CV_8U };
            typedef uchar value_type;
        };

        template<> class TypeDepth<CV_8S>
        {
        public:
            enum { depth = CV_8S };
            typedef schar value_type;
        };

        template<> class TypeDepth<CV_16U>
        {
        public:
            enum { depth = CV_16U };
            typedef ushort value_type;
        };

        template<> class TypeDepth<CV_16S>
        {
        public:
            enum { depth = CV_16S };
            typedef short value_type;
        };

        template<> class TypeDepth<CV_32S>
        {
        public:
            enum { depth = CV_32S };
            typedef int value_type;
        };

        template<> class TypeDepth<CV_32F>
        {
        public:
            enum { depth = CV_32F };
            typedef float value_type;
        };

        template<> class TypeDepth<CV_64F>
        {
        public:
            enum { depth = CV_64F };
            typedef double value_type;
        };

        LX_GEO_FACTORY_SHARED_API cv::Mat multiply_affine_matrices(const cv::Mat& m1, const cv::Mat& m2);
	}
}