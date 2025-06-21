cmake_minimum_required(VERSION 3.14)

project(smart-pointers)

set(CMAKE_MODULE_PATH              "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS  ON)

add_subdirectory(allocations_checker)

include(cmake/BuildFlags.cmake)
include(cmake/AddCatch.cmake)
include(cmake/AddTests.cmake)
