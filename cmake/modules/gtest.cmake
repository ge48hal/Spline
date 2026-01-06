set(GTEST_VERSION 1.15.2)

# 1) Prefer an installed package first
find_package(GTest ${GTEST_VERSION} QUIET CONFIG)

# Some distros provide only MODULE mode (no CONFIG package),
# so try module mode too if CONFIG wasn't found.
if (NOT GTest_FOUND)
  find_package(GTest ${GTEST_VERSION} QUIET)  # MODULE mode
endif()

# 2) If still not found, fetch
if (NOT GTest_FOUND)
  message(STATUS "Fetching GTest: v${GTEST_VERSION}")
  include(FetchContent)
  set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v${GTEST_VERSION}
  )

  # Optional: avoid installing gtest with your project
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  # Optional: gtest uses CRT settings on MSVC
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(googletest)
endif()

