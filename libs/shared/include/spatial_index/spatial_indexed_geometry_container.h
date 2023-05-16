#pragma once
#include "defs.h"
#include "geometries_with_attributes/geometries_with_attributes.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		template <typename geom_type>
		struct SpatialIndexedGeometryContainer {

			//typedef std::conditional<std::is_same_v<geom_type, Boost_Point_2>, std::pair<Boost_Point_2, size_t>, std::pair<Boost_Box_2, size_t>>::type Boost_Value;
			using Boost_Value = typename std::conditional<std::is_same_v<geom_type, Boost_Point_2>, std::pair<Boost_Point_2, size_t>, std::pair<Boost_Box_2, size_t>>::type;
			typedef bgi::rtree<Boost_Value, bgi::quadratic<16> > Boost_RTree;

		public:
			Boost_RTree rtree;

			SpatialIndexedGeometryContainer() {}
			SpatialIndexedGeometryContainer(const SpatialIndexedGeometryContainer& other): rtree(other.rtree) {};
			SpatialIndexedGeometryContainer(const Boost_RTree& ref_rtree) :rtree(ref_rtree) {};
			~SpatialIndexedGeometryContainer() {};

			SpatialIndexedGeometryContainer(SpatialIndexedGeometryContainer&& other) noexcept {
				rtree = std::move(other.rtree);				
			}
			
			SpatialIndexedGeometryContainer& operator=(SpatialIndexedGeometryContainer&& other) noexcept {
				if (this != &other) {
					rtree = std::move(other.rtree);
				}
				return *this;
			}

			SpatialIndexedGeometryContainer& operator=(SpatialIndexedGeometryContainer const&) = default;

			virtual size_t length() const = 0;
			virtual geom_type& operator[](int offset) = 0;
			virtual const geom_type& operator[](int offset) const = 0;

			void init_rtree() {
				rtree.clear();
				if constexpr (std::is_same_v<geom_type, Boost_Point_2>)
					init_rtree_1d();
				else
					init_rtree_2d();
			}

		private:
			void init_rtree_1d() {
				for (size_t idx = 0; idx < this->length(); idx++) {
					const geom_type& c_boost_geom = this->operator[](idx);
					rtree.insert(std::make_pair(c_boost_geom, idx));
				}
			}

			void init_rtree_2d() {
				for (size_t idx = 0; idx < this->length(); idx++) {
					const geom_type& c_boost_geom = this->operator[](idx);
					Boost_Box_2 envelope; bg::envelope(c_boost_geom, envelope);
					rtree.insert(std::make_pair(envelope, idx));
				}
			}

		};

	}
}