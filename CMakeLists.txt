cmake_minimum_required(VERSION 3.13.0 FATAL_ERROR)

if(APPLE)
	set(MACOSX_DEPLOYMENT_TARGET "10.14")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "")
endif()

project(pbr2gltf2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

add_library(pbr2gltf2 src/pbr2gltf2.cpp include/pbr2gltf2.hpp)
target_include_directories(pbr2gltf2 
    PUBLIC "include"
    PRIVATE "thirdparty/json/include"
    PRIVATE "thirdparty/stb"
    PRIVATE "thirdparty/filesystem/include"
)

if(APPLE)
    set_target_properties(pbr2gltf2 PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION C
        MACOSX_FRAMEWORK_IDENTIFIER io.ux3d.pbr2gltf2
        # "current version" in semantic format in Mach-O binary file
        VERSION 1.0.0
        # "compatibility version" in semantic format in Mach-O binary file
        SOVERSION 1.0.0
        PUBLIC_HEADER "include/pbr2gltf2.hpp"
        XCODE_ATTRIBUTE_SKIP_INSTALL YES # Frameworks should generally skip the install
    )
else()
    set_target_properties(pbr2gltf2
    PROPERTIES
    PUBLIC_HEADER "include/pbr2gltf2.hpp"
    )
endif()

add_executable(p2g2 src/main.cpp)
target_link_libraries(p2g2 pbr2gltf2)

install(TARGETS p2g2 pbr2gltf2)