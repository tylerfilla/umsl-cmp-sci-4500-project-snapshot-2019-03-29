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

  /** The next face ID for unknown faces. */
  int m_unknown_id;

  BasicCacheImpl();
};

BasicCacheImpl::BasicCacheImpl()
    : m_faces()
    , m_unknown_id(-1) {
}

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

int BasicCache::insert_unknown(const faces::Encoding& face) {
  // Generate a new ID for unknown faces
  // We shouldn't need to worry about collisions (the user will probably rename, anyway)
  int id = impl->m_unknown_id--;

  // Copy in the new face encoding
  impl->m_faces[id] = face;

  return id;
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

const Encoding& BasicCache::retrieve(int id) const {
  // Look up the face by its ID
  auto where = impl->m_faces.find(id);

  // If face was not found
  if (where == impl->m_faces.end()) {
    throw std::runtime_error("unknown face id");
  }

  // Copy out the encoding
  return where->second;
}

int BasicCache::query(const Encoding& face, double tol) const {
  // Square the tolerance
  // By comparing squares, we can avoid costly sqrt(3) calls
  auto tol_sq = tol * tol;

  // The matched face ID
  int matched_id = 0;

  // Find the first matching face
  // If no faces match, then we leave matched_id at zero
  for (auto&&[id, known] : impl->m_faces) {
    if (known.compare(face) < tol_sq) {
      matched_id = id;
      break;
    }
  }

  return matched_id;
}

} // namespace caches
} // namespace faces
