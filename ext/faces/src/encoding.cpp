/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <faces/encoding.h>

namespace faces {

Encoding::Encoding() = default;

Encoding::Encoding(const Encoding& rhs) = default;

Encoding::Encoding(Encoding&& rhs) noexcept = default;

Encoding::~Encoding() = default;

Encoding& Encoding::operator=(const Encoding& rhs) = default;

Encoding& Encoding::operator=(Encoding&& rhs) noexcept = default;

double Encoding::compare(const Encoding& rhs) const {
  // TODO: Compare the faces
  return 0;
}

} // namespace faces
