cmake_minimum_required(VERSION 3.10)
project(tif)

set(CMAKE_CXX_STANDARD 17)

set(OpenCV_DIR "C:/Marko/programs/opencv/build/install")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
#target_link_libraries(tif ${OpenCV_LIBS})

#find_package(OpenCV REQUIRED)

#include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(tif main.cpp)

target_link_libraries(tif ${OpenCV_LIBS})