/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <stdexcept>
#include <faces/cache.h>

namespace faces {

void Cache::validate_user_id(int id) {
  // User-given IDs cannot be negative
  // This range is reserved for temporary face IDs
  if (id < 0) {
    throw std::runtime_error("");
  }

  // User-given IDs cannot be zero
  // This ID is used to represent the lack of a face
  if (id == 0) {
    throw std::runtime_error("");
  }
}

} // namespace faces
