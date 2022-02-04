#pragma once

#ifdef IO_DATA_STATIC
#define IO_DATA_API  
#else

#ifdef IO_DATA_EXPORTS
#define IO_DATA_API __declspec(dllexport)
#else
#define IO_DATA_API __declspec(dllimport) 
#endif

#endif