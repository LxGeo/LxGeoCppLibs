#pragma once
#ifdef LXGEO_FACTORY_STATIC
#define LX_GEO_FACTORY_SHARED_API  
#else

#ifdef LXGEO_FACTORY_SHARED_EXPORTS
#define LX_GEO_FACTORY_SHARED_API __declspec(dllexport)
#else
#define LX_GEO_FACTORY_SHARED_API __declspec(dllimport)
#endif

#endif