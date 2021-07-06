#pragma once

#ifdef LxGeoFactoryShared_EXPORTS
#define LX_GEO_FACTORY_SHARED_API __declspec(dllexport)
#else
#define LX_GEO_FACTORY_SHARED_API __declspec(dllimport)
#endif