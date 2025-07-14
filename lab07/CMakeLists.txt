cmake_minimum_required(VERSION 3.10)
project(lab07)

set(CMAKE_CXX_STANDARD 17)

# Include OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Fetch libtiff
include(FetchContent)
FetchContent_Declare(
        libtiff
        GIT_REPOSITORY https://gitlab.com/libtiff/libtiff.git
        GIT_TAG v4.5.0 # Replace with the desired version
)
FetchContent_MakeAvailable(libtiff)

# Include libtiff directories
include_directories(${libtiff_SOURCE_DIR} ${libtiff_BINARY_DIR})

# Add executable
add_executable(lab07 main.cpp)

# Link libraries
target_link_libraries(lab07 ${OpenCV_LIBS} tiff)