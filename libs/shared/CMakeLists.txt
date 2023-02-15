set(LIBRARY_OUTPUT_PATH ../out/${CMAKE_BUILD_TYPE})
project(LxGeoFactoryShared)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

if(MSVC)

	if(DEFINED FORCE_STATIC )
		message(STATUS "NOTICE: **** Static build for LxGeoFactoryShared *****")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
		message(STATUS "NOTICE: **** Adding static flags for LxGeoFactoryShared *****")
		add_definitions(-DLXGEO_FACTORY_STATIC)
	else()
		message(STATUS "NOTICE: **** Dynamic build for LxGeoFactoryShared *****")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
	endif()

endif()

find_package(Boost REQUIRED COMPONENTS filesystem log )
if(Boost_FOUND)
	set(Boost_USE_STATIC_LIBS ON)
	set(Boost_USE_MULTITHREAD OFF)
	include_directories(${Boost_INCLUDE_DIRS})
	link_directories(${Boost_LIBRARY_DIRS})
else()
  message(STATUS "NOTICE: This demo requires Boost and will not be compiled.")
endif()

find_package(CGAL REQUIRED)
if(CGAL_FOUND)
  include(${CGAL_USE_FILE})
else()
  message(STATUS "NOTICE: This demo requires CGAL and will not be compiled.")
endif()

find_package(GDAL REQUIRED)
if (GDAL_FOUND)
  include_directories(${GDAL_INCLUDE_DIRS})
else()
  message(STATUS "NOTICE: This exe requires GDAL and will not be compiled.")
endif()

find_package(OpenCV REQUIRED )
if (OpenCV_FOUND)
  include_directories(${OpenCV_INCLUDE_DIRS})
else()
  message(STATUS "NOTICE: This exe requires OpenCV and will not be compiled.")
endif()

find_package(OpenMP)
if (OpenMP_FOUND)
else()
  message(STATUS "NOTICE: This exe uses OpenMP for better performances (not found).")
endif()

find_package(fmt CONFIG REQUIRED)
if (fmt_FOUND)
else()
  message(STATUS "NOTICE: This exe requires fmt and will not be compiled.")
endif()

include_directories(include)
include_directories(../IO_DATA/include)
add_definitions(-D_CONSOLE -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)


file(GLOB_RECURSE 
	LX_GEO_FACTORY_SHARED_SRC_FILES 
	src/* 
	include/*)

if(DEFINED FORCE_STATIC )
    add_library(LxGeoFactoryShared 
	STATIC
	${LX_GEO_FACTORY_SHARED_SRC_FILES})
else()
    add_library(LxGeoFactoryShared 
	SHARED
	${LX_GEO_FACTORY_SHARED_SRC_FILES})
endif()

	
target_link_libraries(LxGeoFactoryShared  ${GDAL_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_LOG_LIBRARY} ${OpenCV_LIBS} fmt::fmt fmt::fmt-header-only)
target_compile_options(LxGeoFactoryShared PRIVATE ${OpenMP_CXX_FLAGS})