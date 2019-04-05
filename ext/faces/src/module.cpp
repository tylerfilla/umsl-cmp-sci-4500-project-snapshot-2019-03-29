/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <pybind11/pybind11.h>

#include <faces/cache.h>
#include <faces/recognizer.h>

PYBIND11_MODULE(faces, m) {
  faces::cache::bind(m);
  faces::recognizer::bind(m);
}
