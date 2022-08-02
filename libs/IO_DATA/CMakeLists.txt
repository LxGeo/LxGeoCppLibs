set(LIBRARY_OUTPUT_PATH ../out/${CMAKE_BUILD_TYPE})
project(IO_DATA LANGUAGES CXX)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

if(MSVC)

	if(DEFINED FORCE_STATIC )
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
		add_definitions(-DIO_DATA_STATIC -DLXGEO_FACTORY_STATIC)
	else()
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
	endif()

endif()

find_package(Boost REQUIRED COMPONENTS filesystem log)
if(Boost_FOUND)
	set(Boost_USE_STATIC_LIBS ON)
	set(Boost_USE_MULTITHREAD OFF)
	include_directories(${Boost_INCLUDE_DIRS})
	link_directories(${Boost_LIBRARY_DIRS})
else()
  message(STATUS "NOTICE: This exe requires Boost and will not be compiled.")
endif()

find_package(CGAL REQUIRED)
if(CGAL_FOUND)
  include(${CGAL_USE_FILE})
else()
  message(STATUS "NOTICE: This exe requires CGAL and will not be compiled.")
endif()
 
find_package(GDAL REQUIRED)
if (GDAL_FOUND)
  include_directories(${GDAL_INCLUDE_DIRS})
else()
  message(STATUS "NOTICE: This exe requires GDAL and will not be compiled.")
endif()

find_package(OpenCV REQUIRED)
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
include_directories(../shared/include)
add_definitions(-D_CONSOLE -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)

file(GLOB_RECURSE 
	IO_DATA_SRC_FILES 
	src/* 
	include/*)

if(DEFINED FORCE_STATIC )
    add_library(IO_DATA 
	STATIC
	${IO_DATA_SRC_FILES})
else()
    add_library(IO_DATA 
	SHARED
	${IO_DATA_SRC_FILES})
endif()


add_dependencies(${PROJECT_NAME} LxGeoFactoryShared)

target_link_libraries(IO_DATA ${GDAL_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_LOG_LIBRARY} ${OpenCV_LIBS} fmt::fmt fmt::fmt-header-only)

target_link_libraries(${PROJECT_NAME} debug ../out/Debug/LxGeoFactoryShared)
target_link_libraries(${PROJECT_NAME} optimized ../out/Release/LxGeoFactoryShared)

target_compile_options(IO_DATA PRIVATE ${OpenMP_CXX_FLAGS})