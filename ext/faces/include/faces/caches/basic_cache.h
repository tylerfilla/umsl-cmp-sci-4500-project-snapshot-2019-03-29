/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#ifndef FACES_CACHES_BASIC_CACHE_H
#define FACES_CACHES_BASIC_CACHE_H

#include <memory>
#include <pybind11/pybind11.h>
#include <faces/cache.h>

namespace faces {
namespace caches {

struct BasicCacheImpl;

/**
 * A basic face cache.
 */
class BasicCache : public Cache {
  /** PImpl. */
  std::unique_ptr<BasicCacheImpl> impl;

public:
  BasicCache();

  BasicCache(const BasicCache& rhs) = delete;

  BasicCache(BasicCache&& rhs) = delete;

  ~BasicCache();

  BasicCache& operator=(const BasicCache& rhs) = delete;

  BasicCache& operator=(BasicCache&& rhs) = delete;

  void insert(int id, const Encoding& rhs) final;

  void remove(int id) final;

  void rename(int id_old, int id_new) final;

  void query() final;
};

namespace basic_cache {

template<class Module>
void bind(Module&& m) {
  namespace py = pybind11;

  py::class_<BasicCache, Cache>(m, "BasicCache")
      .def(py::init<>());
}

} // namespace basic_cache
} // namespace caches
} // namespace faces

#endif // #ifndef FACES_CACHES_BASIC_CACHE_H
