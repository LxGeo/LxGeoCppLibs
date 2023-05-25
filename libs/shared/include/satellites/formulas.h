#pragma once
#include "defs.h"

namespace LxGeo
{
    namespace GeometryFactoryShared
    {

        inline double compute_rotation_angle(const double& satAz1_r, const double& satEl1_r, const double& satAz2_r, const double& satEl2_r) {
            auto cot = [](const double& val) {return 1.0 / std::tan(val); };
            double num, denum;
            num = cot(satEl1_r) * std::cos(satAz1_r) - std::cos(satAz2_r) * cot(satEl2_r);
            denum = cot(satEl1_r) * std::sin(satAz1_r) - std::sin(satAz2_r) * cot(satEl2_r);
            return num / denum;
        }

        inline std::pair<double, double> compute_roof2roof_constants(const double& satAz1_r, const double& satEl1_r, const double& satAz2_r, const double& satEl2_r) {
            double dX = std::sin(satAz1_r) / std::tan(satEl1_r) - std::sin(satAz2_r) / std::tan(satEl2_r);
            double dY = std::cos(satAz1_r) / std::tan(satEl1_r) - std::cos(satAz2_r) / std::tan(satEl2_r);
            return std::make_pair(dX, dY);
        }
    }
}

