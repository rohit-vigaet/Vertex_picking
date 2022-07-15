# CMakeList.txt : Top-level CMake project file, do global configuration
cmake_minimum_required (VERSION 3.21) # C++ 20

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (NO_VCPKG_TOOLCHAIN_FILE TRUE)
foreach (THE_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_FILE})
    if (${THE_TOOLCHAIN_FILE} MATCHES "vcpkg.cmake")
        set (NO_VCPKG_TOOLCHAIN_FILE FALSE)
    endif ()
    if (NOT NO_VCPKG_TOOLCHAIN_FILE)
        break () # there is a vcpkg toolchain file
    endif ()
endforeach ()

if (DEFINED ENV{VCPKG_DIR} AND NO_VCPKG_TOOLCHAIN_FILE)
    list (APPEND CMAKE_TOOLCHAIN_FILE  "$ENV{VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")
    set (NO_VCPKG_TOOLCHAIN_FILE FALSE)
endif ()

if (NO_VCPKG_TOOLCHAIN_FILE)
    message (FATAL_ERROR "check if env:VCPKG_DIR if defined or define CMAKE_TOOLCHAIN_FILE=VCPKG_TOOLCHAIN_FILE")
endif ()


if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/MP /Zc:__cplusplus")
endif()

project ("ThisProject")

add_compile_definitions (PROJECT_ROOT_LOCATION="${CMAKE_SOURCE_DIR}/")
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif ()

message ("|||| THIS IS ${CMAKE_CXX_COMPILER_ID} \tCOMPILER |||||")

add_subdirectory (ThisProject)
add_subdirectory (QOpenGL-example)