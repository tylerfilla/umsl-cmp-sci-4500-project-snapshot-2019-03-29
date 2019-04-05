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

class Encoding;

/** An abstract face cache. */
struct Cache {
  /**
   * Map a new face into the cache.
   *
   * @param id The face ID
   * @param face The face encoding
   */
  virtual void insert(int id, const Encoding& face) = 0;

  /**
   * Remove a face from the cache.
   *
   * @param id The face ID
   */
  virtual void remove(int id) = 0;

  /**
   * Rename a face in the cache.
   *
   * @param id_old The old face ID
   * @param id_new The new face ID
   */
  virtual void rename(int id_old, int id_new) = 0;

  /**
   * Query a face in the cache.
   *
   * TODO
   */
  virtual void query() = 0;

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

  py::class_<Cache>(m, "Cache")
      .def("insert", [](Cache& self, int id, const Encoding& face) {
        self.insert(id, face);
      })
      .def("remove", [](Cache& self, int id) {
        self.remove(id);
      })
      .def("rename", [](Cache& self, int id_old, int id_new) {
        self.rename(id_old, id_new);
      })
      .def("query", [](Cache& self) {
        self.query();
      });
}

} // namespace cache
} // namespace faces

#endif // #ifndef FACES_CACHE_H
