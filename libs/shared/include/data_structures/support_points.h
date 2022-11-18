#pragma once
#include "defs.h"
#include "relationships/composition_struct.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		
		enum SupportPointsStrategy
		{
			none = 0,
			vertex_only = 1 << 0,
			vertex_and_mid_point = 1 << 1,
			constant_walker = 1 << 2
		};

		struct SupportPointsGenOptions {

			SupportPointsGenOptions(SupportPointsStrategy _strategy, double _STEP_LENGTH=0): strategy(_strategy), STEP_LENGTH(_STEP_LENGTH) {
				if ((strategy == SupportPointsStrategy::constant_walker) & (STEP_LENGTH == 0)) {
					throw std::runtime_error("Cannot initiate support point options with a 'constant_walker' strategy and 'step_length' == 0!");
				}
			};

			SupportPointsStrategy strategy;
			double STEP_LENGTH;
		};

		struct SupportPoints : public compositionStrucure<Boost_Point_2> {

			SupportPoints(const SupportPointsStrategy& _strategy) : strategy(_strategy) {};

			SupportPointsStrategy strategy = SupportPointsStrategy::none;
		};

		LX_GEO_FACTORY_SHARED_API SupportPoints decompose_polygons(std::vector<Boost_Polygon_2>& input_polygons,
			const SupportPointsGenOptions& supp_p_options);


	}
}