/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <pybind11/pybind11.h>

#include <faces/cache.h>
#include <faces/encoding.h>
#include <faces/recognizer.h>
#include <faces/source.h>
#include <faces/caches/basic_cache.h>
#include <faces/sources/pil_source.h>

PYBIND11_MODULE(faces, m) {
  // faces
  faces::cache::bind(m);
  faces::encoding::bind(m);
  faces::recognizer::bind(m);
  faces::source::bind(m);

  // faces.caches
  auto m_caches = m.def_submodule("caches");
  faces::caches::basic_cache::bind(m_caches);

  // faces.sources
  auto m_sources = m.def_submodule("sources");
  faces::sources::pil_source::bind(m_sources);
}
