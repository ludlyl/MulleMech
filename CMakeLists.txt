# The MIT License (MIT)
#
# Copyright (c) 2017-2018 Alexander Kurbatov

cmake_minimum_required(VERSION 3.1)

project(MulleMech)

# Specify output directories.
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")

# Build with c++17 support (14 is required by sc2api).
set(CMAKE_CXX_STANDARD 17)

# Disable building of examples in the sc2api submodule.
set(BUILD_API_EXAMPLES OFF CACHE INTERNAL "" FORCE)

# Disable building of tests in the sc2api submodule.
set(BUILD_API_TESTS OFF CACHE INTERNAL "" FORCE)

add_subdirectory("s2client-api")
add_subdirectory("src")
