#pragma once
#include "defs_boost.h"
#include <boost/range/adaptor/indirected.hpp>


namespace LxGeo
{
	namespace GeometryFactoryShared
	{


		template<typename pixel_values_type, typename CoordinateType, std::size_t DimensionCount, typename CoordinateSystem>
		class Elementary_Pinned_Pixels_Boost_Point : public bg::model::point<CoordinateType, DimensionCount, CoordinateSystem> {
		public:
			using Boost_Point_2::Boost_Point_2;
			std::list<pixel_values_type> pinned_pixel;
		};
		
		template <typename pixel_values_type>
		using Elementary_Pinned_Pixels_Boost_Point_2 = Elementary_Pinned_Pixels_Boost_Point<pixel_values_type, double, 2, bg::cs::cartesian>;
		

		// Extension of point class with a list of pixel values
		/*template <typename pixel_values_type>
		class Elementary_Pinned_Pixels_Boost_Point_2 : public bg::model::point<double, 2, bg::cs::cartesian> {
		public:
			using Boost_Point_2::Boost_Point_2;
		public:
			std::list<pixel_values_type> pinned_pixel;
		};*/
		

		template <typename pixel_values_type>
		using Elementary_Pinned_Pixels_Boost_MultiPoint_2 = bg::model::multi_point<Elementary_Pinned_Pixels_Boost_Point_2<pixel_values_type>>;
		template <typename pixel_values_type>
		using Elementary_Pinned_Pixels_Boost_Box_2 = bg::model::box<Elementary_Pinned_Pixels_Boost_Point_2<pixel_values_type>>;
		template <typename pixel_values_type>
		using Elementary_Pinned_Pixels_Boost_Segment_2 = bg::model::segment<Elementary_Pinned_Pixels_Boost_Point_2<pixel_values_type>>;
		template <typename pixel_values_type>
		using Elementary_Pinned_Pixels_Boost_LineString_2 = bg::model::linestring<Elementary_Pinned_Pixels_Boost_Point_2<pixel_values_type>>;
		template <typename pixel_values_type>
		using Elementary_Pinned_Pixels_Boost_Ring_2 = bg::model::ring<Elementary_Pinned_Pixels_Boost_Point_2<pixel_values_type>>;
		template <typename pixel_values_type>
		using Elementary_Pinned_Pixels_Boost_Polygon_2 = bg::model::polygon<Elementary_Pinned_Pixels_Boost_Point_2<pixel_values_type>>;

		/*Extension of a set of geometries with a list of pixel values for each structural elements (ex: rings for a polygon)*/
		template <typename pixel_values_type>
		class Structural_Pinned_Pixels_Boost_Segment_2 : public Boost_Segment_2 {
			std::list<pixel_values_type> pinned_pixel;
		};
		template <typename pixel_values_type>
		class Structural_Pinned_Pixels_Boost_Box_2 : public Boost_Box_2 {
			std::list<pixel_values_type> pinned_pixel;
		};
		template <typename pixel_values_type>
		class Structural_Pinned_Pixels_Boost_LineString_2 : public Boost_LineString_2 {
			std::list<pixel_values_type> pinned_pixel;
		};
		template <typename pixel_values_type>
		class Structural_Pinned_Pixels_Boost_Ring_2 : public Boost_Ring_2 {
			std::list<pixel_values_type> pinned_pixel;
		};
		template <typename pixel_values_type>
		class Structural_Pinned_Pixels_Boost_Polygon_2 : public Boost_Polygon_2 {
		public:
			std::list<pixel_values_type> outer_pinned_pixel;
			std::vector<std::list<pixel_values_type>> inners_pinned_pixels;
		};

	}
}


namespace boost {
	namespace geometry {
		namespace traits
		{
			using namespace LxGeo::GeometryFactoryShared;

			template <typename pixel_values_type, typename C, std::size_t D, typename S>
			struct tag< Elementary_Pinned_Pixels_Boost_Point<pixel_values_type, C, D, S> >
			{
				typedef point_tag type;
			};
			template <typename pixel_values_type, typename C, std::size_t D, typename S>
			struct coordinate_type< Elementary_Pinned_Pixels_Boost_Point<pixel_values_type, C, D, S> >
			{
				typedef C type;
			};
			template <typename pixel_values_type, typename C, std::size_t D, typename S>
			struct coordinate_system< Elementary_Pinned_Pixels_Boost_Point<pixel_values_type, C, D, S> >
			{
				typedef S type;
			};
			template <typename pixel_values_type, typename C, std::size_t D, typename S>
			struct dimension< Elementary_Pinned_Pixels_Boost_Point<pixel_values_type, C, D, S> >
			{
				static const std::size_t value = D;
			};
			template <typename pixel_values_type, typename C, std::size_t D, typename S, std::size_t I>
			struct access<Elementary_Pinned_Pixels_Boost_Point<pixel_values_type, C, D, S>, I>
			{
				static inline C get(Elementary_Pinned_Pixels_Boost_Point<pixel_values_type, C, D, S> const& p)
				{
					return p.template get<I>();
				}

				static inline void set(Elementary_Pinned_Pixels_Boost_Point<pixel_values_type, C, D, S>& p, C const& v)
				{
					p.template set<I>(v);
				}
			};

		}
	}
}

namespace boost {
	namespace geometry {
		namespace traits
		{

			template <typename pixel_values_type> struct tag<Structural_Pinned_Pixels_Boost_Polygon_2<pixel_values_type>> { using type = polygon_tag; };
			template <typename pixel_values_type> struct ring_mutable_type<Structural_Pinned_Pixels_Boost_Polygon_2<pixel_values_type>> { using type = Boost_Ring_2&; };
			template <typename pixel_values_type> struct ring_const_type<Structural_Pinned_Pixels_Boost_Polygon_2<pixel_values_type>> { using type = const Boost_Ring_2&; };
			template <typename pixel_values_type> struct interior_mutable_type<Structural_Pinned_Pixels_Boost_Polygon_2<pixel_values_type>> { using type = std::vector<Boost_Ring_2>; };
			template <typename pixel_values_type> struct interior_const_type<Structural_Pinned_Pixels_Boost_Polygon_2<pixel_values_type>> { using type = const std::vector<Boost_Ring_2>; };

			template<typename pixel_values_type> struct exterior_ring<Structural_Pinned_Pixels_Boost_Polygon_2<pixel_values_type>> {
				static auto& get(Structural_Pinned_Pixels_Boost_Polygon_2<pixel_values_type>& poly) { return poly.outer(); }
				static auto& get(const Structural_Pinned_Pixels_Boost_Polygon_2<pixel_values_type>& poly) { return poly.outer(); }
			};

			template<typename pixel_values_type> struct interior_rings<Structural_Pinned_Pixels_Boost_Polygon_2<pixel_values_type>> {
				static auto& get(Structural_Pinned_Pixels_Boost_Polygon_2<pixel_values_type>& poly) { return poly.inners(); }
				static auto& get(const Structural_Pinned_Pixels_Boost_Polygon_2<pixel_values_type>& poly) { return poly.inners(); }
			};

		}
	}
}