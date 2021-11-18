cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

if(APPLE)
	set(MACOSX_DEPLOYMENT_TARGET "10.14")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "")
endif()

project(pbr2gltf2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_library(Pbr2Gltf2 src/pbr2gltf2.cpp include/pbr2gltf2.hpp)
target_include_directories(Pbr2Gltf2 
    PUBLIC "include"
    PRIVATE "thirdparty/json/include"
    PRIVATE "thirdparty/stb"
    PRIVATE "thirdparty/filesystem/include"
)

add_executable(pbr2gltf2 src/main.cpp)
target_link_libraries(pbr2gltf2 Pbr2Gltf2)

install(TARGETS Pbr2Gltf2 pbr2gltf2)