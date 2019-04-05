/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <faces/caches/basic_cache.h>

namespace faces {
namespace caches {

struct BasicCacheImpl {
};

BasicCache::BasicCache() : impl() {
  impl = std::make_unique<BasicCacheImpl>();
}

BasicCache::~BasicCache() = default;

void BasicCache::insert(int id, const Encoding& rhs) {
}

void BasicCache::remove(int id) {
}

void BasicCache::rename(int id_old, int id_new) {
}

void BasicCache::query() {
}

} // namespace caches
} // namespace faces
