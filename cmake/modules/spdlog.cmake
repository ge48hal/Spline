# spdlog.cmake

find_package(spdlog QUIET CONFIG)
if (NOT spdlog_FOUND)
  find_package(spdlog QUIET)
endif()

# 2) If still not found, fetch spdlog
if (TARGET spdlog::spdlog_header_only)
  message(STATUS "Using system spdlog header-only")
  set(SPDLOG_TARGET spdlog::spdlog_header_only)
else()
  message(STATUS "Fetching spdlog: v1.14.1 (header-only)")
  include(FetchContent)
  set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
  )
  FetchContent_MakeAvailable(spdlog)

  set(SPDLOG_TARGET spdlog::spdlog_header_only)
endif()
