#pragma once
#include "defs_boost.h"
#include "stitchable_geometries/def_stitched_geoms.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		enum ElementaryStitchStrategy
		{
			unique = 0, // read a single value for a each point
			pixel_buffered = 1 << 0, // applicable for all
			spatial_buffered = 1 << 1, // applicable for all
			spatial_corner_area = 1 << 2 /* applicable for 3 + points connected geometry(ex: linestring, ring, polygon) as for the options spa_left_up_pt will be the maximum range 
										 defined by spa_buff_t::left and spa_buff_t::up*/
		};
		
		struct pix_buff_t { size_t up = 0; size_t down = 0; size_t left = 0; size_t right = 0; };
		struct spa_buff_t { double up = 0.0; double down = 0.0; double left = 0.0; double right = 0.0;};

		struct ElementaryStitchOptions{

			ElementaryStitchOptions(ElementaryStitchStrategy _strategy, pix_buff_t _PIX_BUFF, spa_buff_t _SPA_BUFF) 
				: strategy(_strategy){
				assert(_SPA_BUFF.up >= 0.0 && _SPA_BUFF.down >= 0.0 && _SPA_BUFF.left >= 0.0 && _SPA_BUFF.right >= 0.0 && "spatial buffers struct elements should be all positive!");
				assert(_SPA_BUFF.up > 0.0 || _SPA_BUFF.down > 0.0 || _SPA_BUFF.left > 0.0 || _SPA_BUFF.right > 0.0 && "At least one buffer value should be higher than zero!");

				// pixel buffer points setting
				pix_left_up_pt.set<0>(_PIX_BUFF.left);  pix_left_up_pt.set<1>(_PIX_BUFF.up);
				pix_right_down_pt.set<0>(_PIX_BUFF.right);  pix_right_down_pt.set<1>(_PIX_BUFF.down);
				
				// spatial buffer points setting
				spa_left_up_pt.set<0>(_SPA_BUFF.left);  spa_left_up_pt.set<1>(_SPA_BUFF.up);
				spa_right_down_pt.set<0>(_SPA_BUFF.right);  spa_right_down_pt.set<1>(_SPA_BUFF.down);
			};
		
			ElementaryStitchStrategy strategy;
			Boost_Discrete_Point_2 pix_left_up_pt, pix_right_down_pt;
			Boost_Point_2 spa_left_up_pt, spa_right_down_pt;

		};

	}
}