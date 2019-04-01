#
# InsertProjectName
# Copyright (c) 2019 The InsertProjectName Contributors
# InsertLicenseText
#

cmake_minimum_required(VERSION 3.4)
project(cozmonaut)

find_package(PythonInterp 3.7 REQUIRED)
find_package(PythonLibs 3.7 REQUIRED)

add_subdirectory(third_party/pybind11)

#
# facelib C++ library
#

set(facelib_SRC_FILES
        cozmonaut/facelib/facelib.cpp
        )

add_library(facelib SHARED ${facelib_SRC_FILES})
set_target_properties(facelib PROPERTIES CXX_STANDARD 17)
target_include_directories(facelib PRIVATE ${PYTHON_INCLUDE_DIRS})
target_link_libraries(facelib PRIVATE ${PYTHON_LIBRARIES} pybind11::pybind11)

# Name the library file in such a way that setuptools can detect it
# TODO: We should compute this instead of hardcoding it
set_target_properties(facelib PROPERTIES PREFIX "" OUTPUT_NAME "facelib.cpython-37m-x86_64-linux-gnu")

# On Windows, rename the DLL to use the .pyd file extension
# This is so Python can detect it and pick it up automatically I think? TODO Need to fact check that
if(WIN32)
    set_target_properties(facelib PROPERTIES SUFFIX ".pyd")
endif()
