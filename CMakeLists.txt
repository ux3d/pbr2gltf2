cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

if(APPLE)
	set(MACOSX_DEPLOYMENT_TARGET "10.14")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "")
endif()

project(pbr2gltf2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories("thirdparty/json/include")
include_directories("thirdparty/stb")
include_directories("thirdparty/filesystem/include")

add_executable(pbr2gltf2 src/main.cpp)
