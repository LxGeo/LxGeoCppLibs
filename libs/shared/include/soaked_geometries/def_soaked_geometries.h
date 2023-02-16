#pragma once
#include "defs_boost.h"
#include "defs_opencv.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		// Structure used to capture image matrix within total bounds of a ring
		template <typename cv_mat_type>
		struct ring_boundary_soak {			
			cv_mat_type img;
			cv_mat_type mask;
			double geotransform[6];
		};

		template <typename cv_mat_type>
		class Soacked_Pixels_Boost_Polygon_2 : public Boost_Polygon_2 {
		public:
			ring_boundary_soak<cv_mat_type> outer_ring_soak;
			std::vector<ring_boundary_soak<cv_mat_type>> inner_rings_soak;
		};


	}
}

namespace boost {
	namespace geometry {
		namespace traits
		{

			using namespace LxGeo::GeometryFactoryShared;

			template <typename cv_mat_type> struct tag<Soacked_Pixels_Boost_Polygon_2<cv_mat_type>> { using type = polygon_tag; };
			template <typename cv_mat_type> struct ring_mutable_type<Soacked_Pixels_Boost_Polygon_2<cv_mat_type>> { using type = Boost_Ring_2&; };
			template <typename cv_mat_type> struct ring_const_type<Soacked_Pixels_Boost_Polygon_2<cv_mat_type>> { using type = const Boost_Ring_2&; };
			template <typename cv_mat_type> struct interior_mutable_type<Soacked_Pixels_Boost_Polygon_2<cv_mat_type>> { using type = std::vector<Boost_Ring_2>; };
			template <typename cv_mat_type> struct interior_const_type<Soacked_Pixels_Boost_Polygon_2<cv_mat_type>> { using type = const std::vector<Boost_Ring_2>; };

			template <typename cv_mat_type> struct exterior_ring<Soacked_Pixels_Boost_Polygon_2<cv_mat_type>> {
				static auto& get(Soacked_Pixels_Boost_Polygon_2<cv_mat_type>& poly) { return poly.outer(); }
				static auto& get(const Soacked_Pixels_Boost_Polygon_2<cv_mat_type>& poly) { return poly.outer(); }
			};

			template <typename cv_mat_type> struct interior_rings<Soacked_Pixels_Boost_Polygon_2<cv_mat_type>> {
				static auto& get(Soacked_Pixels_Boost_Polygon_2<cv_mat_type>& poly) { return poly.inners(); }
				static auto& get(const Soacked_Pixels_Boost_Polygon_2<cv_mat_type>& poly) { return poly.inners(); }
			};

		}
	}
}