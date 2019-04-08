#
# Cozmonaut
# Copyright (c) 2019 The Cozmonaut Contributors
#
# InsertLicenseText
#

cmake_minimum_required(VERSION 3.8)
project(cozmonaut)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PythonInterp 3.7 REQUIRED)
find_package(PythonLibs 3.7 REQUIRED)

add_subdirectory(third_party/pybind11)
add_subdirectory(third_party/spdyface)

set(faces_SRC_FILES
        src/caches/basic_cache.cpp
        src/sources/pil_source.cpp
        src/cache.cpp
        src/common_image.cpp
        src/encoding.cpp
        src/module.cpp
        src/recognizer.cpp
        )

add_library(faces SHARED ${faces_SRC_FILES})
set_target_properties(faces PROPERTIES CXX_STANDARD 17)
target_include_directories(faces PRIVATE include ${PYTHON_INCLUDE_DIRS})
target_link_libraries(faces PRIVATE ${PYTHON_LIBRARIES} pybind11::pybind11 spdyface)
