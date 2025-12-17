cmake_minimum_required(VERSION 3.10)

# define project name, version
project (Spline VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)  # Ensure compiler-specific features work

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(OMP "Enable Parallel (OpenMp)" ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

include(doxygen)
include(gtest)
include(openmp)
include(spdlog)
include(GoogleTest)

# -------------------------------------------------------
# SHARED LIBRARY spline_c++
# -------------------------------------------------------

set(SPLINE_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/src/Spline.cpp")
set(PYBIND_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/python/pybinding.cpp")

file(GLOB_RECURSE MY_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    # header don't need to be included but this might be necessary for some IDEs
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

list(REMOVE_ITEM MY_SRC "${SPLINE_MAIN}" "${PYBIND_SRC}")

add_library(spline_c++ SHARED ${MY_SRC})   


set_target_properties(spline_c++ PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ON
        ENABLE_EXPORTS ON
        POSITION_INDEPENDENT_CODE ON
)


# Include directories
target_include_directories(spline_c++ PUBLIC
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libxsd
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)



# Define build types : Debug and Release
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")


# Set target-specific compiler/linker options (using modern target_... functions instead of global CMAKE_..._FLAGS)
target_compile_options(spline_c++ PUBLIC
    # Release mode options (Optimization and native architecture)
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-march=native>
    -mavx2
)


target_link_libraries(spline_c++ PUBLIC
            spdlog::spdlog
)  

if(OMP)
    target_link_libraries(spline_c++ PUBLIC  OpenMP::OpenMP_CXX )
endif()

# -------------------------------------------------------
# Spline executable settings (Spline main file)
# -------------------------------------------------------

add_executable(Spline ${SPLINE_MAIN})

target_link_libraries(Spline PRIVATE
    spline_c++
    
)

target_include_directories(Spline
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(Spline PRIVATE
    $<$<CONFIG:Debug>:-g>
)

set_target_properties(Spline PROPERTIES
    ENABLE_EXPORTS ON
)

# Optionally add separate Debug and Release settings to avoid overwriting them
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Building in Release mode ")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building in Debug mode ")
endif()


# -------------------------------------------------------
# spline tests executable settings (Spline tests)
# -------------------------------------------------------


#testfiles 
file(GLOB_RECURSE MY_TESTS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
)

add_executable(tests ${MY_TESTS}) # make tests file executable

target_include_directories(tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(tests PRIVATE
    GTest::gtest
    spline_c++
)

target_compile_options(tests PRIVATE
    -march=native
)

set_property(TARGET tests PROPERTY ENVIRONMENT "GTEST_COLOR=1")

enable_testing()

gtest_discover_tests(tests)


# -------------------------------------------------------
# pybind11 Python module settings
# -------------------------------------------------------

find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(splinepy
    ${CMAKE_CURRENT_SOURCE_DIR}/src/python/pybinding.cpp
)

target_link_libraries(splinepy PRIVATE
    spline_c++
)

target_include_directories(splinepy PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
