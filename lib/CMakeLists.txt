cmake_minimum_required(VERSION 2.8)
project( WelshColorisation )

add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
add_compile_options(-Wall)

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable( WelshColorisation ${SOURCES} )
target_link_libraries( WelshColorisation ${OpenCV_LIBS} )