#include "defs.h"
#include "defs_cgal.h"
#include "export_shared.h"

namespace LxGeo
{
	namespace GeometryFactoryShared
	{
		LX_GEO_FACTORY_SHARED_API FT determinant(const Vector_2 & AB, const Vector_2 & ST)
		{
			return (AB.x() * ST.y() - AB.y() * ST.x());
		}
	}
}