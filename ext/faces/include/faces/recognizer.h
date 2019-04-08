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

struct Cache;
struct Source;

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

  Recognizer(const Recognizer& rhs) = delete;

  Recognizer(Recognizer&& rhs) = delete;

  ~Recognizer();

  Recognizer& operator=(const Recognizer& rhs) = delete;

  Recognizer& operator=(Recognizer&& rhs) = delete;

  /**
   * @return The face cache
   */
  Cache* get_cache() const;

  /**
   * @param p_cache The face cache
   */
  void set_cache(Cache* p_cache);

  /**
   * @return The video source
   */
  Source* get_source() const;

  /**
   * @param p_source The video source
   */
  void set_source(Source* p_source);

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
      .def(py::init<>())
      .def_property("cache", &Recognizer::get_cache, &Recognizer::set_cache)
      .def_property("source", &Recognizer::get_source, &Recognizer::set_source)
      .def("start", &Recognizer::start)
      .def("stop", &Recognizer::stop);
}

} // namespace recognizer
} // namespace faces

#endif // #ifndef FACES_RECOGNIZER_H
