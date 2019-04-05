/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#ifndef FACES_CACHE_H
#define FACES_CACHE_H

#include <pybind11/pybind11.h>

namespace faces {

/** An abstract face cache. */
struct Cache {
  /**
   * Map a face into the cache.
   *
   * @param id The face ID
   * @param face
   */
  virtual void insert(int id, const Encoding& face) = 0;

  virtual void remove(int id) = 0;

protected:
  /**
   * Validate a user-given face ID.
   *
   * @param id The face ID in question
   */
  static void validate_user_id(int id);
};

namespace cache {

template<class Module>
void bind(Module&& m) {
  namespace py = pybind11;

  py::class_<Cache>(m, "Cache");
}

} // namespace cache
} // namespace faces

#endif // #ifndef FACES_CACHE_H
