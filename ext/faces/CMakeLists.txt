#
# Cozmonaut
# Copyright (c) 2019 The Cozmonaut Contributors
#
# InsertLicenseText
#

cmake_minimum_required(VERSION 3.8)
project(cozmonaut)

find_package(PythonInterp 3.7 REQUIRED)
find_package(PythonLibs 3.7 REQUIRED)

set(BUILD_SHARED_LIBS ON)

add_subdirectory(third_party/dlib)
add_subdirectory(third_party/pybind11)

set(faces_SRC_FILES
        src/cache.cpp
        src/encoding.cpp
        src/module.cpp
        src/recognizer.cpp
        )

add_library(faces SHARED ${faces_SRC_FILES})
set_target_properties(faces PROPERTIES CXX_STANDARD 17)
target_include_directories(faces PRIVATE include ${PYTHON_INCLUDE_DIRS})
target_link_libraries(faces PRIVATE ${PYTHON_LIBRARIES} dlib::dlib pybind11::pybind11)
