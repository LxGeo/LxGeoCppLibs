#pragma once
#include <ogrsf_frmts.h>
#include "export_shared.h"


namespace LxGeo
{
	namespace GeometryFactoryShared
	{

		LX_GEO_FACTORY_SHARED_API OGRPolygon envelopeToPolygon(const OGREnvelope& in_envelope);

		LX_GEO_FACTORY_SHARED_API OGREnvelope box_buffer(OGREnvelope& in_box, double buff);
		
	}
}