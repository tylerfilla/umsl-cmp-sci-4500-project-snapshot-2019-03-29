/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#ifndef FACES_SOURCES_CV2_SOURCE_H
#define FACES_SOURCES_CV2_SOURCE_H

#include <memory>
#include <pybind11/pybind11.h>
#include <faces/source.h>

namespace faces {
namespace sources {

struct PILSourceImpl;

/**
 * An OpenCV source.
 */
class PILSource : public Source {
  /** PImpl. */
  std::unique_ptr<PILSourceImpl> impl;

public:
  PILSource();

  PILSource(const PILSource& rhs) = delete;

  PILSource(PILSource&& rhs) = delete;

  ~PILSource();

  PILSource& operator=(const PILSource& rhs) = delete;

  PILSource& operator=(PILSource&& rhs) = delete;

  void update(const pybind11::object& img) final;

  std::optional<Image> wait(unsigned long millis) final;
};

namespace pil_source {

template<class Module>
void bind(Module&& m) {
  namespace py = pybind11;

  py::class_<PILSource, Source>(m, "PILSource")
      .def(py::init<>());
}

} // namespace pil_source
} // namespace sources
} // namespace faces

#endif // #ifndef FACES_SOURCES_CV2_SOURCE_H
