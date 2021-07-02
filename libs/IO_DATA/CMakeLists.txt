set(LIBRARY_OUTPUT_PATH ../../out/build/bin/)

project(Io_Data LANGUAGES CXX)
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

include_directories(include)
include_directories(../shared/include)
add_definitions(-D_CONSOLE -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DLX_KINETIC2_EXPORTS)

file(GLOB_RECURSE 
	IO_DATA_SRC_FILES 
	src/* 
	include/*)

file(GLOB_RECURSE 
	SHARED 
	../shared/include/*
	../shared/src/*)

add_library(IO_DATA 
	SHARED
	${IO_DATA_SRC_FILES})

include_directories("../shared/include/")


target_link_libraries(IO_DATA ${GDAL_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${OpenCV_LIBS})
target_compile_options(IO_DATA PRIVATE ${OpenMP_CXX_FLAGS})