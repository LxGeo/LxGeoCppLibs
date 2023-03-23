#pragma once
#include <ogrsf_frmts.h>
#include "defs_boost.h"
#include "defs_cgal.h"
#include "defs_opencv.h"
#include "export_shared.h"

namespace LxGeo
{

	namespace GeometryFactoryShared
	{

		LX_GEO_FACTORY_SHARED_API Inexact_Point_2 transform_B2C_Point(const Boost_Point_2& boost_point);

		LX_GEO_FACTORY_SHARED_API Boost_Point_2 transform_C2B_Point(const Inexact_Point_2& cgal_point);

		LX_GEO_FACTORY_SHARED_API void container_transform_B2C_Points(std::vector<Boost_Point_2>& input_container, std::vector<Inexact_Point_2>& output_container);

		LX_GEO_FACTORY_SHARED_API void container_transform_OGRRING2vector_Points(OGRLinearRing* input_ring, std::vector<Inexact_Point_2>& output_vector);

		LX_GEO_FACTORY_SHARED_API std::vector<Inexact_Point_2> container_transform_OGRRING2vector_Points(OGRLinearRing* input_ring);

		LX_GEO_FACTORY_SHARED_API OGRLinearRing container_transform_vector_Points2OGRRING(const std::vector<Inexact_Point_2>& input_vector);

		LX_GEO_FACTORY_SHARED_API OGRPoint transform_B2OGR_Point(const Boost_Point_2& in_point);

		LX_GEO_FACTORY_SHARED_API OGRLineString transform_B2OGR_LineString(const Boost_Ring_2& in_ring);

		LX_GEO_FACTORY_SHARED_API OGRLinearRing transform_B2OGR_Ring(const Boost_Ring_2& in_ring);

		LX_GEO_FACTORY_SHARED_API Boost_Polygon_2 transform_OGR2B_Polygon(OGRPolygon* ogr_polygon);

		LX_GEO_FACTORY_SHARED_API Boost_LineString_2 transform_OGR2B_Linestring(OGRLineString* ogr_linestring);

		LX_GEO_FACTORY_SHARED_API Boost_Point_2 transform_OGR2B_Point(OGRPoint* ogr_point);

		LX_GEO_FACTORY_SHARED_API OGRPolygon transform_B2OGR_Polygon(const Boost_Polygon_2& in_polygon);

		LX_GEO_FACTORY_SHARED_API OGRLineString transform_B2OGR_LineString(const Boost_LineString_2& in_linestring);

		LX_GEO_FACTORY_SHARED_API OGREnvelope transform_B2OGR_Envelope(const Boost_Box_2& in_envelope);

		LX_GEO_FACTORY_SHARED_API Boost_Box_2 transform_OGR2B_Envelope(const OGREnvelope& in_envelope);

		LX_GEO_FACTORY_SHARED_API void transform_G2CV_affine(const double in_geotransform[6], cv::Mat& out_affine_matrix);

		LX_GEO_FACTORY_SHARED_API void transform_CV2G_affine(const cv::Mat& affine_matrix, double outtransform[6]);

		template <typename ogr_geom_type, typename boost_geom_type>
		boost_geom_type transform_OGR2B_geometry(ogr_geom_type* in_geometry) {			

			if constexpr (std::is_same_v<boost_geom_type, Boost_Point_2>) {
				return transform_OGR2B_Point(in_geometry);
			}
			else if constexpr (std::is_same_v<boost_geom_type, Boost_LineString_2>) {
				return transform_OGR2B_Linestring(in_geometry);
			}
			else if constexpr (std::is_same_v<boost_geom_type, Boost_Polygon_2>) {
				return transform_OGR2B_Polygon(in_geometry);
			}
			else
				throw std::exception("Only points, linestrings, polygons are transformable!");
		}

		template <typename boost_geom_type, typename ogr_geom_type>
		ogr_geom_type transform_B2OGR_geometry(boost_geom_type in_geometry) {

			if constexpr (std::is_same_v<boost_geom_type, Boost_Point_2>) {
				return transform_B2OGR_Point(in_geometry);
			}
			else if constexpr (std::is_same_v<boost_geom_type, Boost_LineString_2>) {
				return transform_B2OGR_LineString(in_geometry);
			}
			else if constexpr (std::is_same_v<boost_geom_type, Boost_Polygon_2>) {
				return transform_B2OGR_Polygon(in_geometry);
			}
			else
				throw std::exception("Only points, linestrings, polygons are transformable!");
		}

		template <typename envelope_type>
		envelope_type create_envelope(const double& minx, const double& miny, const double& maxx, const double& maxy) {
			if constexpr (std::is_same_v<envelope_type, Boost_Box_2>)
				return Boost_Box_2({ minx, miny }, { maxx, maxy });
			else if constexpr (std::is_same_v<envelope_type, OGREnvelope>) {
				OGREnvelope out_env; out_env.MinX = minx; out_env.MinY = miny; out_env.MaxX= maxx; out_env.MaxY= maxy;
				return out_env;
			}
		}

	}
}