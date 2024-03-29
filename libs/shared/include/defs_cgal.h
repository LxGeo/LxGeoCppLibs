#pragma once
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
//#include <CGAL/Exact_predicates_exact_constructions_kernel_with_sqrt.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include<CGAL/Polygon_2.h>
#include<CGAL/create_straight_skeleton_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_with_holes_2.h>
#include <CGAL/CORE_Expr.h>
#include <CGAL/number_utils.h>
#include <CGAL/Cartesian_converter.h>
//#include <CGAL/Quadtree.h>
#include <boost/multiprecision/gmp.hpp>
#include <CGAL/IO/WKT.h>
#include <CGAL/number_utils.h>
namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		typedef CGAL::Exact_predicates_exact_constructions_kernel EK;
		typedef CGAL::Lazy_exact_nt<boost::multiprecision::mpq_rational> FT;
		typedef CGAL::Lazy_exact_nt<CORE::Expr> CORE_Expr;

		typedef EK::Point_2 Point_2;
		typedef EK::Segment_2 Segment_2;
		typedef EK::Line_2 Line_2;
		typedef EK::Vector_2 Vector_2;
		typedef EK::Direction_2 Direction_2;
		typedef EK::Triangle_2 Triangle_2;
		typedef CGAL::Polygon_2<EK> Polygon_2;
		typedef CGAL::Polygon_with_holes_2<EK> Polygon_with_holes_2;

		typedef CGAL::Exact_predicates_inexact_constructions_kernel IK;
		typedef IK::Point_2 Inexact_Point_2;
		typedef IK::Segment_2 Inexact_Segment_2;
		typedef IK::Line_2 Inexact_Line_2;
		typedef IK::Vector_2 Inexact_Vector_2;
		typedef IK::Direction_2 Inexact_Direction_2;
		typedef IK::Triangle_2 Inexact_Triangle_2;
		typedef IK::Iso_rectangle_2 Inexact_Iso_rectangle_2;
		typedef CGAL::Polygon_2<IK> Inexact_Polygon_2;
		typedef CGAL::Polygon_with_holes_2<IK> Inexact_Polygon_with_holes_2;
		typedef CGAL::Straight_skeleton_2<IK>  Inexact_Straight_Skeleton_2;


		typedef CGAL::Bbox_2 Bbox_2;

		//typedef CGAL::Quadtree<IK, std::vector<Inexact_Point_2>> Quadtree;

		typedef CGAL::Cartesian_converter<IK, EK> IK_to_EK;
		typedef CGAL::Cartesian_converter<EK, IK> EK_to_IK;

		
		/*
		typedef CGAL::Simple_cartesian<int>::Point_2 Integer_Point;
		typedef CGAL::Simple_cartesian<int>::Vector_2 Integer_Vector;
		typedef CGAL::Simple_cartesian<int>::Direction_2 Integer_Direction;
		typedef CGAL::Simple_cartesian<int>::Line_2 Integer_Line;
		*/
	}
}
