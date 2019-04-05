/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <map>

#include <faces/encoding.h>
#include <faces/caches/basic_cache.h>

namespace faces {
namespace caches {

struct BasicCacheImpl {
  /** The face backing store. */
  std::map<int, Encoding> m_faces;
};

BasicCache::BasicCache() : impl() {
  impl = std::make_unique<BasicCacheImpl>();
}

BasicCache::~BasicCache() = default;

void BasicCache::insert(int id, const Encoding& face) {
  // Validate new face ID
  validate_user_id(id);

  // If this ID is not not already in use
  // In other words, if this ID is already in use
  if (impl->m_faces.find(id) != impl->m_faces.end()) {
    throw std::runtime_error("duplicate face id");
  }

  // Copy in the new face encoding
  impl->m_faces[id] = face;
}

void BasicCache::remove(int id) {
  // Look up the doomed face by its ID
  auto where = impl->m_faces.find(id);

  // If face was not found
  if (where == impl->m_faces.end()) {
    throw std::runtime_error("unknown face id");
  }

  // Delete the encoding
  impl->m_faces.erase(where);
}

void BasicCache::rename(int id_old, int id_new) {
  // Validate new face ID
  validate_user_id(id_new);

  // Look up the old face by its ID
  auto where = impl->m_faces.find(id_old);

  // If old face was not found
  if (where == impl->m_faces.end()) {
    throw std::runtime_error("unknown old face id");
  }

  // Copy the face encoding out of the map
  auto face = where->second;

  // Erase the old mapping
  impl->m_faces.erase(where);

  // Copy the face mapping into the new location
  impl->m_faces[id_new] = face;
}

void BasicCache::query() {
  // TODO: Use a simple linear scan
}

} // namespace caches
} // namespace faces
