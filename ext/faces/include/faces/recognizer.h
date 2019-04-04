/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#ifndef FACES_RECOGNIZER_H
#define FACES_RECOGNIZER_H

#include <memory>
#include <pybind11/pybind11.h>

namespace faces {

struct RecognizerImpl;

/**
 * A custom-built continuous facial recognition device that uses the dlib
 * library and a background worker thread. Its design is tailored to handling
 * video frames from Anki's Cozmo SDK, so it is probably not the best for normal
 * video streams. The background thread is intended to dodge CPython's GIL, and
 * it would be inappropriate for normal video.
 */
class Recognizer {
  /** PImpl. */
  std::unique_ptr<RecognizerImpl> impl;

public:
  Recognizer();

  ~Recognizer();

  /** Start continuous recognition. */
  void start();

  /** Stop continuous recognition. */
  void stop();
};

namespace recognizer {

template<class Module>
void bind(Module&& m) {
  namespace py = pybind11;

  py::class_<Recognizer>(m, "Recognizer")
      .def("start", &Recognizer::start)
      .def("stop", &Recognizer::stop);
}

} // namespace recognizer
} // namespace faces

#endif // #ifndef FACES_RECOGNIZER_H
