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

		template <typename point_type>
		std::vector<std::vector<cv::Point>> transform_B2CV_Polygon(const bg::model::polygon<point_type>& in_poly) {
			std::vector<std::vector<cv::Point>> contours;
			for (auto it = bg::interior_rings(in_poly).begin(); it != bg::interior_rings(in_poly).end(); ++it) {
				std::vector<cv::Point> contour;
				for (auto pt_it = it->begin(); pt_it != it->end(); ++pt_it) {
					contour.push_back(cv::Point((*pt_it).get<0>(), (*pt_it).get<1>()));
				}
				contours.push_back(contour);
			}
			std::vector<cv::Point> exterior_contour;
			for (auto pt_it = bg::exterior_ring(in_poly).begin(); pt_it != bg::exterior_ring(in_poly).end(); ++pt_it) {
				exterior_contour.push_back(cv::Point((*pt_it).get<0>(), (*pt_it).get<1>()));
			}
			contours.insert(contours.begin(), exterior_contour);
			return contours;
		}

		// Define a helper struct for coordinate access
		template <typename PointT>
		struct PointTraits {
			static double getX(const PointT& point) { return point.x(); }
			static double getY(const PointT& point) { return point.y(); }
		};

		// Specialization for CGAL Point
		template <>
		struct PointTraits<Inexact_Point_2> {
			static double getX(const Inexact_Point_2& point) { return point.x(); }
			static double getY(const Inexact_Point_2& point) { return point.y(); }
		};

		// Specialization for CGAL Point
		template <>
		struct PointTraits<Point_2> {
			static double getX(const Point_2& point) { return CGAL::to_double(point.x()); }
			static double getY(const Point_2& point) { return CGAL::to_double(point.y()); }
		};

		// Specialization for OGRPoint
		template <>
		struct PointTraits<OGRPoint> {
			static double getX(const OGRPoint& point) { return point.getX(); }
			static double getY(const OGRPoint& point) { return point.getY(); }
		};

		template <>
		struct PointTraits<Boost_Point_2> {
			static double getX(const Boost_Point_2& point) {
				return boost::geometry::get<0>(point);
			}

			static double getY(const Boost_Point_2& point) {
				return boost::geometry::get<1>(point);
			}
		};

		template <typename PolygonTypeT>
		struct PolygonTrait {
			using PolygonType = PolygonTypeT;
			using PointType = typename PolygonType::PointType;

			static std::vector<PointType> getOuterRing(const PolygonType& polygon) {};


			static std::vector<PointType> getInnerRing(const PolygonType& polygon, size_t idx) {};
			

			PolygonType create(const std::vector<PointType>& exterior_ring, const std::vector<std::vector<PointType>>& interior_rings = {}) {};
			static size_t interiorCount(const PolygonType& polygon) {};

		};

		template<typename K>
		struct PolygonTrait<CGAL::Polygon_2<K>> {
			using PolygonType = CGAL::Polygon_2<K>;
			using PointType = typename K::Point_2;

			template <template <typename...> class Container>
			static Container<PointType> getOuterRing(const PolygonType& polygon) {
				Container<PointType> points;
				for (const PointType& c_point : polygon) {
					points.push_back(c_point);
				}
				return points;
			}

			template <template <typename...> class Container, template <typename...> class OuterContainer = std::list>
			static PolygonType create(const Container<PointType>& exterior_ring, const OuterContainer<Container<PointType>>& interior_rings = {}) {
				if (interior_rings.size() > 0)
					throw std::runtime_error("Cannot use CGAL::Polygon_2 for polygons with holes!");
				return PolygonType(exterior_ring.begin(), exterior_ring.end());
			}

			static size_t interiorCount(const PolygonType& polygon) { return 0; };

		};

		template<typename K>
		struct PolygonTrait<CGAL::Polygon_with_holes_2<K>> {
			using PointType = typename K::Point_2;
			using PolygonType = CGAL::Polygon_with_holes_2<K>;
			using ExteriorPolygonType = CGAL::Polygon_2<K>;

			template <template <typename...> class Container>
			static Container<PointType> getOuterRing(const PolygonType& polygon) {
				return PolygonTrait<ExteriorPolygonType>::getOuterRing(polygon.outer_boundary());
			}

			template <template <typename...> class Container>
			static Container<PointType> getInnerRing(const PolygonType& polygon, size_t idx) {

				if (idx<0 || idx>polygon.holes().size())
					throw std::runtime_error("Interior ring index out of bounds!");

				Container<PointType> points;

				auto respective_ring = polygon.holes().begin();
				for (size_t c_idx = 0; c_idx < idx; c_idx++)
					respective_ring++;

				for (const auto& c_pt : respective_ring) {
					points.push_back(c_pt);
				}
				return points;

			}

			template <template <typename...> class Container, template <typename...> class OuterContainer = std::list>
			static PolygonType create(const Container<PointType>& exterior_ring, const OuterContainer<Container<PointType>>& interior_rings = {}) {
				return PolygonType(exterior_ring, interior_rings.begin(), interior_rings.end());
			}

			static size_t interiorCount(const PolygonType& polygon) { return polygon.holes().size(); };

		};


		template<typename PointTypeT>
		struct PolygonTrait< bg::model::polygon<PointTypeT> > {

			using PointType = typename PointTypeT;
			using PolygonType = bg::model::polygon<PointType>;

			static std::vector<PointType> getOuterRing(const PolygonType& polygon) {
				std::vector<PointType> points; points.reserve(polygon.outer().size());
				for (const auto& c_pt : polygon.outer()) {
					points.push_back(c_pt);
				}
				return points;
			}

			static std::vector<PointType> getInnerRing(const PolygonType& polygon, size_t idx) {
				if (idx<0 || idx>polygon.inners().size())
					throw std::runtime_error("Interior ring index out of bounds!");

				std::vector<PointType> points;

				auto respective_ring = polygon.inners().begin();
				for (size_t c_idx = 0; c_idx < idx; c_idx++)
					respective_ring++;

				points.reserve(respective_ring->size());
				for (const auto& c_pt : *respective_ring) {
					points.push_back(c_pt);
				}
				return points;
			}

			
			static PolygonType create(const std::vector<PointType>& exterior_ring, const std::vector<std::vector<PointType>>& interior_rings = {}) {
				bg::model::polygon<PointType> p;
				for (const auto& c_exterior_point : exterior_ring)
					bg::append(p.outer(), c_exterior_point);

				size_t num_interors = interior_rings.size();
				p.inners().resize(num_interors);

				size_t c_ring_idx = 0;
				for (auto c_ring_it = interior_rings.begin(); c_ring_it != interior_rings.end(); c_ring_it++, c_ring_idx++) {
					for (const auto& c_interior_point : *c_ring_it)
						bg::append(p.inners()[c_ring_idx], c_interior_point);
				}

				return p;
			}

			static size_t interiorCount(const PolygonType& polygon) { return polygon.inners().size(); };

		};
		
		template<>
		struct PolygonTrait<OGRPolygon> {

			using PointType = typename OGRPoint;
			using PolygonType = typename OGRPolygon;

			template <template <typename...> class Container>
			static Container<PointType> getOuterRing(const PolygonType& polygon) {
				Container<OGRPoint> points;
				const OGRLinearRing* outerRing = polygon.getExteriorRing();
				for (int i = 0; i < outerRing->getNumPoints(); ++i) {
					OGRPoint c_point;
					outerRing->getPoint(i, &c_point);
					points.push_back(c_point);
				}
				return points;
			}

			template <template <typename...> class Container>
			static Container<PointType> getInnerRing(const PolygonType& polygon, size_t idx) {
				Container<OGRPoint> points;
				const OGRLinearRing* inner_ring = polygon.getInteriorRing(idx);
				for (int i = 0; i < inner_ring->getNumPoints(); ++i) {
					OGRPoint c_point;
					inner_ring->getPoint(i, &c_point);
					points.push_back(c_point);
				}
				return points;
			}

			template <template <typename...> class Container, template <typename...> class OuterContainer = std::list>
			static PolygonType create(const Container<PointType>& exterior_ring, const OuterContainer<Container<PointType>>& interior_rings = {}) {

				OGRPolygon polygon;
				OGRLinearRing exteriorRing;

				for (const auto& point : exterior_ring) {
					exteriorRing.addPoint(&point);
				}

				polygon.addRing(&exteriorRing);

				for (auto ring = interior_rings.begin(); ring != interior_rings.end(); ring++) {
					OGRLinearRing interiorRing;
					for (const auto& point : *ring) {
						interiorRing.addPoint(&point);
					}
					polygon.addRing(&interiorRing);
				}
				return polygon;

			}

			static size_t interiorCount(const PolygonType& polygon) { return polygon.getNumInteriorRings(); };

		};

		template <typename LineStringT>
		struct LineStringTrait {

			using PointType = typename LineStringT::PointType;
			using LineStringType = LineStringT;

			static PointType getPointAt(const LineStringType& linestring, int idx) {};

			static std::vector<PointType> getPoints(const LineStringType& linestring) {};

			static size_t length(const LineStringType& linestring) {};

			template <typename Iter>
			static LineStringType create(const Iter& begin, const Iter& end) {};

		};

		template<typename PointTypeT>
		struct LineStringTrait< bg::model::linestring<PointTypeT> > {

			using PointType = typename PointTypeT;
			using LineStringType = bg::model::linestring<PointType>;

			static PointType getPointAt(const LineStringType& linestring, int idx) {
				return linestring.at(idx);
			};

			static std::vector<PointType> getPoints(const LineStringType& linestring) {
				std::vector<PointType> points(linestring.begin(), linestring.end());
				return points;
			};

			static size_t length(const LineStringType& linestring) {
				return linestring.size();
			};

			template <typename Iter>
			static LineStringType create(const Iter& begin, const Iter& end) {
				return LineStringType(begin, end);
			};

		};



		template <typename env_type>
		struct envelopeGet {
			const env_type* env;
			envelopeGet(const env_type* _env) :env(_env) {};
			double getMinX() const {
				if constexpr (std::is_same_v<env_type, OGREnvelope>)
					return env->MinX;
				else
					return env->min_corner().get<0>();
			}
			double getMinY() const {
				if constexpr (std::is_same_v<env_type, OGREnvelope>)
					return env->MinY;
				else
					return env->min_corner().get<1>();
			}
			double getMaxX() const {
				if constexpr (std::is_same_v<env_type, OGREnvelope>)
					return env->MaxX;
				else
					return env->max_corner().get<0>();
			}
			double getMaxY() const {
				if constexpr (std::is_same_v<env_type, OGREnvelope>)
					return env->MaxY;
				else
					return env->max_corner().get<1>();
			}
		};

		template <typename env_type>
		struct envelopeGetSet : envelopeGet<env_type> {
			env_type* env;
			envelopeGetSet(env_type* _env) :envelopeGet<env_type>(_env), env(_env) {};

			void setMinX(const double& minx) {
				if constexpr (std::is_same_v<env_type, OGREnvelope>)
					env->MinX = minx;
				else
					env->min_corner().set<0>(minx);
			}
			void setMinY(const double& miny) {
				if constexpr (std::is_same_v<env_type, OGREnvelope>)
					env->MinY = miny;
				else
					env->min_corner().set<1>(miny);
			}
			void setMaxX(const double& maxx) {
				if constexpr (std::is_same_v<env_type, OGREnvelope>)
					env->MaxX = maxx;
				else
					env->max_corner().set<0>(maxx);
			}
			void setMaxY(const double& maxy) {
				if constexpr (std::is_same_v<env_type, OGREnvelope>)
					env->MaxY = maxy;
				else
					env->max_corner().set<1>(maxy);
			}
		};

		template <typename env_type_1, typename env_type_2>
		void merge_bounds(const env_type_1& env1, env_type_2& env2) {
			envelopeGetSet(&env2).setMinX(min(envelopeGetSet(&env2).getMinX(), envelopeGetSet(&env1).getMinX()));
			envelopeGetSet(&env2).setMinY(min(envelopeGetSet(&env2).getMinY(), envelopeGetSet(&env1).getMinY()));
			envelopeGetSet(&env2).setMaxX(min(envelopeGetSet(&env2).getMaxX(), envelopeGetSet(&env1).getMaxX()));
			envelopeGetSet(&env2).setMaxY(min(envelopeGetSet(&env2).getMaxY(), envelopeGetSet(&env1).getMaxY()));
		}

		template <typename env_type_0, typename env_type_1, typename env_type_2>
		env_type_0 retrun_merge_bounds(const env_type_1& env1, env_type_2& env2) {
			env_type_0 output_envelope;
			envelopeGetSet(&output_envelope).setMinX(min(envelopeGetSet(&env2).getMinX(), envelopeGetSet(&env1).getMinX()));
			envelopeGetSet(&output_envelope).setMinY(min(envelopeGetSet(&env2).getMinY(), envelopeGetSet(&env1).getMinY()));
			envelopeGetSet(&output_envelope).setMaxX(min(envelopeGetSet(&env2).getMaxX(), envelopeGetSet(&env1).getMaxX()));
			envelopeGetSet(&output_envelope).setMaxY(min(envelopeGetSet(&env2).getMaxY(), envelopeGetSet(&env1).getMaxY()));
			return output_envelope;
		}

		template <typename PointT>
		double inner_angle(const PointT& p0, const PointT& p1, const PointT& p2) {

			double angle = 0.0;

			double BAx = PointTraits<PointT>::getX(p0) - PointTraits<PointT>::getX(p1);
			double BAy = PointTraits<PointT>::getY(p0) - PointTraits<PointT>::getY(p1);
			double BCx = PointTraits<PointT>::getX(p2) - PointTraits<PointT>::getX(p1);
			double BCy = PointTraits<PointT>::getY(p2) - PointTraits<PointT>::getY(p1);

			double dotProduct = BAx * BCx + BAy * BCy;
			double magnitudeBA = std::sqrt(BAx * BAx + BAy * BAy);
			double magnitudeBC = std::sqrt(BCx * BCx + BCy * BCy);

			if (magnitudeBA > 0 && magnitudeBC > 0)
			{
				double cosTheta = min(1.0, dotProduct / (magnitudeBA * magnitudeBC));
				angle = std::acos(cosTheta);
			}

			return angle;
			
		}

	}
}