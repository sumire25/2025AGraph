cmake_minimum_required(VERSION 3.24)
project(OpenGL_Full)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include .cpp files here
set(SOURCE_FILES OpenGL_Full/main.cpp OpenGL_Full/glad.c)

# Include .h files here
set(HEADER_FILES
        OpenGL_Full/MarchingCubes.h
        OpenGL_Full/GridFromTiff.h)

# Include the .cpp and .h files as executables
add_executable(OpenGL_Full ${SOURCE_FILES} ${HEADER_FILES})

# Include directories
include_directories(OpenGL_Full Linking/include)
include_directories(GLAD/include)

# Include subdirectory
add_subdirectory(dependencies)

# Fetch libtiff from its repository
include(FetchContent)
FetchContent_Declare(
        libtiff
        GIT_REPOSITORY https://gitlab.com/libtiff/libtiff.git
        GIT_TAG v4.5.0 # Replace with the desired version
)
FetchContent_MakeAvailable(libtiff)

# Link libraries
target_link_libraries(OpenGL_Full PRIVATE glu32 opengl32 glfw tiff)