cmake_minimum_required(VERSION 3.10)

# define project name, version
project (Spline VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)  # Ensure compiler-specific features work


# Define build types : Debug and Release
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")

#find_package(gtest Required)
#find_package(spdlog REQUIRED)

file(GLOB_RECURSE MY_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    # header don't need to be included but this might be necessary for some IDEs
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)
add_executable(Spline ${MY_SRC})

# Include directories
target_include_directories(Spline PUBLIC
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/libxsd
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Spline
        # stuff that is used in headers and source files
        PUBLIC
            spdlog::spdlog
)   

# set c++ standard to c++20
set_target_properties(Spline PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ON
        ENABLE_EXPORTS ON
)

# Set target-specific compiler/linker options (using modern target_... functions instead of global CMAKE_..._FLAGS)
target_compile_options(Spline PRIVATE
    # Release mode options (Optimization and native architecture)
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-march=native>
)
target_link_options(Spline PRIVATE
    # Linker flags for Debug build
    $<$<CONFIG:Debug>:-g>
)

# Optionally add separate Debug and Release settings to avoid overwriting them
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Building in Release mode ")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building in Debug mode ")
endif()


list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

include(doxygen)
include(gtest)
include(openmp)
include(spdlog)
include(GoogleTest)

#testfiles 
file(GLOB_RECURSE MY_TESTS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

list(FILTER MY_TESTS EXCLUDE REGEX "Spline.cpp") # exclude main file from tests
add_executable(tests ${MY_TESTS}) # make tests file executable

target_include_directories(tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(tests PRIVATE
    GTest::gtest    
    spdlog::spdlog
)
set_property(TARGET tests PROPERTY ENVIRONMENT "GTEST_COLOR=1")

gtest_discover_tests(tests)

enable_testing()
