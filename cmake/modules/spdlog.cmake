# spdlog.cmake

#check if spdlog is already included
if (NOT TARGET spdlog)

    include(FetchContent)
    set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.14.1 #Latest version spdlog at 08.11.2024
    )
    FetchContent_MakeAvailable(spdlog)

    #need to put more options if needed.
endif()