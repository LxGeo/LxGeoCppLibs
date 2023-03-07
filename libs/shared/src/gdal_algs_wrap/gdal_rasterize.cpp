#include "gdal_algs_wrap/gdal_rasterize.h"

#include <gdal_priv.h>
#include "gdal_utils.h"
#include "geometries_with_attributes/geometries_with_attributes.h"

namespace LxGeo
{
    namespace GeometryFactoryShared
    {

        void rasterize_shapefile(const std::string& out_raster_path, const std::string& input_shapefile_path,
            OGREnvelope* raster_extents, double raster_px_size, double raster_py_size, const std::list<std::string>& extra_options,
            double* nodata_value, const std::string& output_type )
        {
            unsigned int openFlags = GDAL_OF_VECTOR | GDAL_OF_READONLY;
            GDALDataset* pSrcDataset = static_cast<GDALDataset*>(GDALOpenEx(input_shapefile_path.c_str(), openFlags, NULL, NULL, NULL));
            OGRLayer* pSrcLayer = pSrcDataset->GetLayer(0);

            // Rasterize options.
            char** argv = NULL;
            argv = CSLAddString(argv, "-l");
            argv = CSLAddString(argv, pSrcLayer->GetName());
            if (nodata_value != nullptr) {
                argv = CSLAddString(argv, "-a_nodata");
                argv = CSLAddString(argv, +std::to_string(*nodata_value).c_str());
            }
            argv = CSLAddString(argv, "-tr");
            argv = CSLAddString(argv, std::to_string(raster_px_size).c_str());
            argv = CSLAddString(argv, std::to_string(raster_py_size).c_str());

            assert(gdalConstants::gdal_types_set.find(output_type) != gdalConstants::gdal_types_set.end());
            argv = CSLAddString(argv, "-ot");
            argv = CSLAddString(argv, output_type.c_str());

            if (raster_extents) {
                argv = CSLAddString(argv, "-te");
                argv = CSLAddString(argv, std::to_string(raster_extents->MinX - 1).c_str());
                argv = CSLAddString(argv, std::to_string(raster_extents->MinY - 1).c_str());
                argv = CSLAddString(argv, std::to_string(raster_extents->MaxX + 1).c_str());
                argv = CSLAddString(argv, std::to_string(raster_extents->MaxY + 1).c_str());
            }
            else {
                OGREnvelope out_extents; pSrcLayer->GetExtent(&out_extents);
                argv = CSLAddString(argv, "-te");
                argv = CSLAddString(argv, std::to_string(out_extents.MinX).c_str());
                argv = CSLAddString(argv, std::to_string(out_extents.MinY).c_str());
                argv = CSLAddString(argv, std::to_string(out_extents.MaxX).c_str());
                argv = CSLAddString(argv, std::to_string(out_extents.MaxY).c_str());
            }

            for (auto& c_option : extra_options) {
                argv = CSLAddString(argv, c_option.c_str());
            }

            GDALRasterizeOptions* pOptions = GDALRasterizeOptionsNew(argv, NULL);

            // Perform rasterization.
            int          usageError;
            GDALDataset* pDstDataset =
                static_cast<GDALDataset*>(GDALRasterize(out_raster_path.c_str(), NULL, pSrcDataset, pOptions, &usageError));

            //------------------------------
            // Cleanup.
            GDALRasterizeOptionsFree(pOptions);
            CSLDestroy(argv);
            GDALClose(pSrcDataset);
            GDALClose(pDstDataset);
        }


    }
}