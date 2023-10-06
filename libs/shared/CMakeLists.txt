project(LxGeoFactoryShared)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
set(CMAKE_CXX_STANDARD 20)
if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.12.25835)
    set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
    set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")
  endif()
if(DEFINED ALT_LIBRARY_OUTPUT_PATH)
  set(LIBRARY_OUTPUT_PATH ${ALT_LIBRARY_OUTPUT_PATH})
else()
  #set(LIBRARY_OUTPUT_PATH "../out/${CMAKE_BUILD_TYPE}")
endif()
if(MSVC)

	if(DEFINED FORCE_STATIC )
		message(STATUS "NOTICE: **** Static build for ${PROJECT_NAME} *****")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
		message(STATUS "NOTICE: **** Adding static flags for ${PROJECT_NAME} *****")
		add_definitions(-DLXGEO_FACTORY_STATIC)
	else()
		message(STATUS "NOTICE: **** Dynamic build for ${PROJECT_NAME} *****")
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

find_package(nlohmann_json CONFIG REQUIRED)

include_directories(include)
include_directories(include/io/)
add_definitions(-D_CONSOLE -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)

###############

# Find all header files in the include directory
file(GLOB_RECURSE HEADERS "include/*.h")

# Find all source files in the src directory
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Define filters for header and source files based on their directory structure
foreach(_headerFile IN ITEMS ${HEADERS})
    file(RELATIVE_PATH _headerRelativePath ${CMAKE_CURRENT_SOURCE_DIR}/include ${_headerFile})
    get_filename_component(_headerGroup ${_headerRelativePath} DIRECTORY)
    string(REPLACE "/" "\\" _headerGroup "${_headerGroup}")
    source_group("Header Files\\${_headerGroup}" FILES "${_headerFile}")
endforeach()

foreach(_sourceFile IN ITEMS ${SOURCES})
    file(RELATIVE_PATH _sourceRelativePath ${CMAKE_CURRENT_SOURCE_DIR}/src ${_sourceFile})
    get_filename_component(_sourceGroup ${_sourceRelativePath} DIRECTORY)
    string(REPLACE "/" "\\" _sourceGroup "${_sourceGroup}")
    source_group("Source Files\\${_sourceGroup}" FILES "${_sourceFile}")
endforeach()


###############

file(GLOB_RECURSE 
	LX_GEO_FACTORY_SHARED_SRC_FILES 
	src/* 
	include/*)

if(DEFINED FORCE_STATIC )
    add_library( ${PROJECT_NAME}  
	STATIC
	${LX_GEO_FACTORY_SHARED_SRC_FILES})
else()
    add_library( ${PROJECT_NAME}  
	SHARED
	${LX_GEO_FACTORY_SHARED_SRC_FILES})
endif()
	
target_link_libraries( ${PROJECT_NAME}   ${GDAL_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_LOG_LIBRARY} ${OpenCV_LIBS} fmt::fmt fmt::fmt-header-only nlohmann_json::nlohmann_json)
target_compile_options( ${PROJECT_NAME}  PRIVATE ${OpenMP_CXX_FLAGS})