# boost.cmake
# Header-only Boost configuration

set(BOOST_VERSION "1.85.0")

# 1) Try to find a system-installed Boost first 
find_package(Boost QUIET CONFIG)
if (NOT Boost_FOUND)
  find_package(Boost QUIET)
endif()

if (Boost_FOUND)
  message(STATUS "Using system Boost (header-only)")

  # If the Boost::boost target exists, use it directly
  if (TARGET Boost::boost)
    set(BOOST_TARGET Boost::boost)
  else()
    # Older FindBoost.cmake only provides include directories
    # Create an INTERFACE target to expose Boost headers
    add_library(boost_headers INTERFACE)
    target_include_directories(boost_headers INTERFACE ${Boost_INCLUDE_DIRS})
    add_library(Boost::boost ALIAS boost_headers)
    set(BOOST_TARGET Boost::boost)
  endif()

else()
  # 2) If Boost is not found, fetch headers only via FetchContent
  message(STATUS "Fetching Boost headers only: ${BOOST_VERSION}")

  include(FetchContent)
  set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

  FetchContent_Declare(
    boost_headers
    URL https://github.com/boostorg/boost/releases/download/boost-${BOOST_VERSION}/boost-${BOOST_VERSION}.tar.gz
  )

  FetchContent_MakeAvailable(boost_headers)

  # Create an INTERFACE library exposing only Boost headers
  add_library(boost_headers INTERFACE)
  target_include_directories(
    boost_headers
    INTERFACE ${boost_headers_SOURCE_DIR}
  )

  # Provide the standard Boost::boost alias target
  add_library(Boost::boost ALIAS boost_headers)
  set(BOOST_TARGET Boost::boost)
endif()
