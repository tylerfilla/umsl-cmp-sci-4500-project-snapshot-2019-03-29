/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <cstddef>
#include <faces/encoding.h>

namespace faces {

Encoding::Encoding() = default;

Encoding::Encoding(const Encoding& rhs) = default;

Encoding::Encoding(Encoding&& rhs) noexcept = default;

Encoding::~Encoding() = default;

Encoding& Encoding::operator=(const Encoding& rhs) = default;

Encoding& Encoding::operator=(Encoding&& rhs) noexcept = default;

double Encoding::compare(const Encoding& rhs) const {
  // The running sum
  double sum = 0;

  // Go over face vector element-by-element
  for (std::size_t i = 0; i < std::tuple_size_v<vector_type>; ++i) {
    // Element-wise difference
    auto diff = rhs.m_vector[i] - m_vector[i];

    // Add squared difference
    sum += diff * diff;
  }

  return sum;
}

} // namespace faces
