#pragma once
#include <opencv2/opencv.hpp>
#include "opencv2/ximgproc.hpp"

namespace LxGeo
{
	namespace LxGeoCommon
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
	}
}