#include "gdal_algs_wrap/gdal_proximity.h"

#include <gdal_priv.h>
#include "gdal_utils.h"
#include "gdal_alg.h"

namespace LxGeo
{
    namespace GeometryFactoryShared
    {

        LX_GEO_FACTORY_SHARED_API GeoImage<cv::Mat> proximity_raster(const GeoImage<cv::Mat>& input_image, size_t source_band_idx, const std::list<double>& target_pixels,
            GDALDataType output_type, std::string distunits, std::optional<double>maxdist,
            std::optional<double> nodata, std::optional<double> fixed_buf_val, const std::list<std::string>& extra_options)
        {
            // Pre-check parameters
            if (distunits != "GEO" && distunits != "PIXEL")
                throw std::runtime_error("Wrong distunits argument!");            

            GDALRasterBandH in_band, out_band;
            // Save geoimage to file
            const std::string temp_categorized_raster_path = random_string(5, "/vsimem/", ".tif");
            
            std::shared_ptr<GDALDataset> categorized_raster_dataset = input_image.to_file(temp_categorized_raster_path);
            if (source_band_idx < 1 || source_band_idx > categorized_raster_dataset->GetRasterCount())
                throw std::runtime_error("Wrong source band index!");
            in_band = categorized_raster_dataset->GetRasterBand(source_band_idx);
            
            // create output
            const std::string temp_proximity_raster_path = random_string(5, "/vsimem/", ".tif");
            //const std::string temp_proximity_raster_path = random_string(5, "C:/Users/geoimage/Pictures/", ".tif");
            RProfile ref_profile = RProfile::from_geoimage<cv::Mat>(input_image);
            ref_profile.dtype = output_type; ref_profile.count = 1;
            auto proximity_raster_dataset = ref_profile.to_gdal_dataset(temp_proximity_raster_path);
            out_band = proximity_raster_dataset->GetRasterBand(1);             

            // Rasterize options.
            char** argv = NULL;
            argv = CSLAddString(argv, ("DISTUNITS="+ distunits).c_str());
            if (!target_pixels.empty()) {
                std::string comma_sep_values;
                for(const auto& c_value : target_pixels)
                    comma_sep_values += std::to_string(c_value);
                argv = CSLAddString(argv, ("VALUES="+ comma_sep_values).c_str());
            }
            if (maxdist.has_value()) {
                argv = CSLAddString(argv, ("MAXDIST="+ std::to_string(maxdist.value())).c_str() );
            }
            if (nodata.has_value()) {
                argv = CSLAddString(argv, ("NODATA="+ std::to_string(nodata.value())).c_str());
            }
            if (fixed_buf_val.has_value()) {
                argv = CSLAddString(argv, ("FIXED_BUF_VAL="+ std::to_string(fixed_buf_val.value())).c_str());
            }

            for (auto& c_option : extra_options) {
                argv = CSLAddString(argv, c_option.c_str());
            }

            // Perform algorithm.
            int          usageError;
            CPLErr err = GDALComputeProximity(in_band, out_band, argv, NULL, nullptr );   
            proximity_raster_dataset->FlushCache();
            //------------------------------
            // Cleanup.
            CSLDestroy(argv);

            GeoImage<cv::Mat> proximity_gimg = GeoImage<cv::Mat>::from_dataset(proximity_raster_dataset);
            return proximity_gimg;
        
        }        
    }
}