cmake_minimum_required(VERSION 3.10)
project(ocv)

set(CMAKE_CXX_STANDARD 17)

#set(OpenCV_DIR "C:/OpenCV/opencv/build")

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(ocv main.cpp)

target_link_libraries(ocv ${OpenCV_LIBS})