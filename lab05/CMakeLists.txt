cmake_minimum_required(VERSION 3.10)
project(lab05)

set(CMAKE_CXX_STANDARD 17)

#set(OpenCV_DIR "C:/OpenCV/opencv/build")

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(lab05 main.cpp)

target_link_libraries(lab05 ${OpenCV_LIBS})