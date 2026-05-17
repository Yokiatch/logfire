cmake_minimum_required(VERSION 3.20)
project(logfire VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-O3 -march=native -Wall -Wextra -Wpedantic)

find_package(PkgConfig REQUIRED)
pkg_check_modules(RE2 REQUIRED re2)

add_library(logfire_core STATIC
  cpp/core/mmap_reader.cpp
  cpp/core/line_scanner.cpp
  cpp/core/query_filter.cpp
  cpp/core/serializer.cpp
)
set_property(TARGET logfire_core PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(logfire_core PUBLIC cpp/core ${RE2_INCLUDE_DIRS})
target_link_libraries(logfire_core PUBLIC ${RE2_LIBRARIES})

add_executable(logfire_cli cpp/cli/main.cpp)
target_link_libraries(logfire_cli PRIVATE logfire_core)

find_package(GTest QUIET)
if(GTest_FOUND)
  enable_testing()
  add_executable(logfire_tests tests/cpp/test_core.cpp)
  target_link_libraries(logfire_tests PRIVATE logfire_core GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(logfire_tests)
endif()

# ── pybind11 extension ────────────────────────────────────────────
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

include(FetchContent)
FetchContent_Declare(
  pybind11
  GIT_REPOSITORY https://github.com/pybind/pybind11.git
  GIT_TAG        v2.13.6
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(pybind11)

pybind11_add_module(_logfire cpp/bindings/py_bindings.cpp)
target_include_directories(_logfire PRIVATE cpp/core)
target_link_libraries(_logfire PRIVATE logfire_core)