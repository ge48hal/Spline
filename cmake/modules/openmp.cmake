cmake_minimum_required(VERSION 3.10)

option(OPENMP "Enable OpenMP support" ON)

if(OPENMP)
    find_package(OpenMP QUIET)

    if(OpenMP_CXX_FOUND)
        message(STATUS "Enabling OpenMP.")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(OMP ON)
    else()
        message(WARNING "OpenMP not found. Compiling without OpenMP.")
        set (OMP OFF)
    endif()
else()
    message(STATUS "OpenMP disabled. Compiling without OpenMP.")
    set (OMP OFF)

endif()