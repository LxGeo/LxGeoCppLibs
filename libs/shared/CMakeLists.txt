set(LIBRARY_OUTPUT_PATH ../../out/build/bin/)

project(LxGeoFactoryShared LANGUAGES CXX)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

find_package(Boost REQUIRED COMPONENTS filesystem )
if(Boost_FOUND)
	set(Boost_USE_STATIC_LIBS OFF)
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
include_directories(include)
add_definitions(-D_CONSOLE -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DLX_POLYGON_FACTORY_SHARED_EXPORTS)


file(GLOB_RECURSE 
	LX_GEO_FACTORY_SHARED_SRC_FILES 
	src/* 
	include/*)

add_library(LxGeoFactoryShared 
	SHARED
	${LX_GEO_FACTORY_SHARED_SRC_FILES})
	
target_link_libraries(LxGeoFactoryShared  ${GDAL_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${OpenCV_LIBS})
target_compile_options(LxGeoFactoryShared PRIVATE ${OpenMP_CXX_FLAGS})