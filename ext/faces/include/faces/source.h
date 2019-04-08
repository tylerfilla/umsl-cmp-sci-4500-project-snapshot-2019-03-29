/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#ifndef FACES_SOURCE_H
#define FACES_SOURCE_H

#include <optional>
#include <pybind11/pybind11.h>

namespace faces {

/** The common image type. */
struct Image {
  /** The image width. */
  int width;

  /** The image height. */
  int height;

  /** The image data. */
  std::vector<char> data;
};

/** An abstract video source. */
struct Source {
  /**
   * Update the source with the given image.
   *
   * @param img The image
   */
  virtual void update(const pybind11::object& img) = 0;

  /**
   * Wait for the next video frame.
   *
   * @param millis The maximum number of milliseconds to wait
   * @return The next frame
   */
  virtual std::optional<Image> wait(unsigned long millis) = 0;
};

namespace source {

template<class Module>
void bind(Module&& m) {
  namespace py = pybind11;

  py::class_<Source>(m, "Source")
      .def("update", [](Source& self, const py::object& img) {
        return self.update(img);
      });
}

} // namespace source
} // namespace faces

#endif // #ifndef FACES_SOURCE_H
