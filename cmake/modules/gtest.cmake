set(GTEST_VERSION 1.15.2)

#check if gtest is already included
if (NOT TARGET gtest)
    message("Fetching gtest : version 1.15.2 ")
    include(FetchContent)
    set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2 # v1.15.2 is the latest version at 07.11.2021
    )
    FetchContent_MakeAvailable(googletest)

endif()
