cmake_minimum_required(VERSION 2.8)
project( WelshColorisation )

add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

# gperftool

# opencv
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")

# openmp
find_package(OpenMP)
if (OPENMP_FOUND)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# temporary test
add_compile_options(-Wall -g)
add_executable( WelshColorisation ${SOURCES} )
target_link_libraries( WelshColorisation -lprofiler )
target_link_libraries( WelshColorisation ${OpenCV_LIBS} )

# compiling the shared lib
add_library( wcolorisation SHARED src/WelshColorisation.cpp )