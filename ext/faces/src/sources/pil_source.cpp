/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <iostream>

#include <faces/sources/pil_source.h>

namespace faces {
namespace sources {

namespace py = pybind11;

struct PILSourceImpl {
};

PILSource::PILSource() : impl() {
  impl = std::make_unique<PILSourceImpl>();
}

PILSource::~PILSource() = default;

void PILSource::update(const py::object& img) {
  // Get dimensions of image
  auto width = py::cast<int>(img.attr("width"));
  auto height = py::cast<int>(img.attr("height"));

  // Get raw bytes of the image
  // Strings are odd for working with raw bytes, but pybind11 has an implicit conversion available
  // Also, this performs the necessary memory management to keep pointers from dangling
  auto bytes = py::cast<std::string>(img.attr("tobytes")("raw"));

  std::cout << width << ", " << height << ", " << bytes.size() << "\n";
}

} // namespace sources
} // namespace faces
