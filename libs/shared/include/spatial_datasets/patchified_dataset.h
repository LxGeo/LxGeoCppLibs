#pragma once
#include "defs.h"
#include "geometry_constructor/grid.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		struct PatchifiedDatasetParameters {
			double spatial_patch_size, spatial_patch_overlap;
			double spatial_pad_size = 0.0;
			Boost_Polygon_2 boundary_geometry;
		};

		class ContinuousPatchifiedDataset {

		public:

			ContinuousPatchifiedDataset(){}

			ContinuousPatchifiedDataset(PatchifiedDatasetParameters _patchified_dst_parameters): patchified_dst_parameters(_patchified_dst_parameters){
				setup_spatial(
					patchified_dst_parameters.spatial_patch_size,
					patchified_dst_parameters.spatial_patch_overlap,
					patchified_dst_parameters.boundary_geometry,
					patchified_dst_parameters.spatial_pad_size
				);
			}

			void setup_spatial(double _spatial_patch_size, double _spatial_patch_overlap, const Boost_Polygon_2& _boundary_geometry, double spatial_pad_size = 0.0)
			{
				patchified_dst_parameters.spatial_patch_size = _spatial_patch_size;
				patchified_dst_parameters.spatial_patch_overlap = _spatial_patch_overlap;
				patchified_dst_parameters.boundary_geometry = _boundary_geometry;
				// Construct a square buffered geometry
				Boost_Box_2 boundary_geometry_envelope;
				bg::envelope(_boundary_geometry, boundary_geometry_envelope);
				boundary_geometry_envelope = Boost_Box_2(
					Boost_Point_2(boundary_geometry_envelope.min_corner().get<0>() - spatial_pad_size, boundary_geometry_envelope.min_corner().get<1>() - spatial_pad_size),
					Boost_Point_2(boundary_geometry_envelope.max_corner().get<0>() + spatial_pad_size, boundary_geometry_envelope.max_corner().get<1>() + spatial_pad_size)
				);

				int grid_step = _spatial_patch_size - _spatial_patch_overlap * 2;
				assert (grid_step > 0 && "Spatial patch overlap is high! Reduce patch overlap.");

				grid_boxes = create_rectangular_grid(
					boundary_geometry_envelope, grid_step, grid_step, _spatial_patch_size, _spatial_patch_size,
					[&_boundary_geometry](const Boost_Box_2& a)->bool {return bg::intersects(_boundary_geometry, a); });
			};

			size_t length() {
				return grid_boxes.size();
			}

			Boost_Box_2& operator[](int offset) {
				assert(abs(offset) < length() && "Dataset offset out of bounds!");
				offset = (offset + length()) % length();
				return grid_boxes[offset];
			}

		public:
			PatchifiedDatasetParameters patchified_dst_parameters;
			std::vector<Boost_Box_2> grid_boxes;
		};

	}
}