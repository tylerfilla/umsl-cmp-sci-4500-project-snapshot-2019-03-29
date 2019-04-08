/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#ifndef FACES_RECOGNIZER_H
#define FACES_RECOGNIZER_H

#include <functional>
#include <memory>
#include <tuple>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace faces {

struct Cache;
struct Encoding;
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
public:
  /**
   * A callback for face appearances.
   *
   * @param id The face ID
   * @param rect The face bounding rectangle
   * @param enc The face encoding
   */
  using CbFaceAppear = std::function<void(Recognizer& rec, int id, std::tuple<int, int, int, int> rect, Encoding& enc)>;

  /**
   * A callback for face disappearances.
   *
   * @param id The face ID
   * @param rect The face bounding rectangle
   */
  using CbFaceDisappear = std::function<void(Recognizer& rec, int id)>;

  /**
   * A callback for face movements.
   *
   * @param id The face ID
   * @param rect The face bounding rectangle
   */
  using CbFaceMove = std::function<void(Recognizer& rec, int id, std::tuple<int, int, int, int> rect)>;

private:
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

  /**
   * Register a callback for face appearances.
   *
   * @param cb The callback
   */
  void register_face_appear(CbFaceAppear cb);

  /**
   * Register a callback for face disappearances.
   *
   * @param cb The callback
   */
  void register_face_disappear(CbFaceDisappear cb);

  /**
   * Register a callback for face movements.
   *
   * @param cb The callback
   */
  void register_face_move(CbFaceMove cb);

  /** Start continuous recognition. */
  void start();

  /** Stop continuous recognition. */
  void stop();

  /** Poll for event callbacks. */
  void poll();
};

namespace recognizer {

template<class Module>
void bind(Module&& m) {
  namespace py = pybind11;

  py::class_<Recognizer>(m, "Recognizer")
      .def(py::init<>())
      .def_property("cache", &Recognizer::get_cache, &Recognizer::set_cache)
      .def_property("source", &Recognizer::get_source, &Recognizer::set_source)
      .def("register_face_appear", &Recognizer::register_face_appear)
      .def("register_face_disappear", &Recognizer::register_face_disappear)
      .def("register_face_move", &Recognizer::register_face_move)
      .def("start", &Recognizer::start)
      .def("stop", &Recognizer::stop)
      .def("poll", &Recognizer::poll);
}

} // namespace recognizer
} // namespace faces

#endif // #ifndef FACES_RECOGNIZER_H
