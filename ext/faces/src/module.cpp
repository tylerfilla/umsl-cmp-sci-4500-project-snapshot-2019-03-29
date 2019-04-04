/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <pybind11/pybind11.h>
#include <faces/recognizer.h>

PYBIND11_MODULE(faces, m) {
  faces::recognizer::bind(m);
}
