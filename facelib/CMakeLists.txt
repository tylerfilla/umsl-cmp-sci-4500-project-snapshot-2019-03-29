#
# InsertProjectName
# Copyright (c) 2019 The InsertProjectName Contributors
# InsertLicenseText
#

cmake_minimum_required(VERSION 3.8)
project(cozmonaut)

find_package(PythonInterp 3.7 REQUIRED)
find_package(PythonLibs 3.7 REQUIRED)

set(BUILD_SHARED_LIBS ON)

add_subdirectory(third_party/dlib)
add_subdirectory(third_party/pybind11)

set(facelib_SRC_FILES
        src/main.cpp
        )

add_library(facelib SHARED ${facelib_SRC_FILES})
set_target_properties(facelib PROPERTIES CXX_STANDARD 17)
target_include_directories(facelib PRIVATE ${PYTHON_INCLUDE_DIRS})
target_link_libraries(facelib PRIVATE ${PYTHON_LIBRARIES} dlib::dlib pybind11::pybind11)
