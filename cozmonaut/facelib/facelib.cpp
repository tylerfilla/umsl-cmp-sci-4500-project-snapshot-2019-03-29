/*
 * InsertProjectName
 * Copyright (c) 2019 The InsertProjectName Contributors
 * InsertLicenseText
 */

#include <functional>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

/** The callback type for face show events. */
using OnFaceShowCb = std::function<void(int fid, int x, int y, int width, int height)>;

/** The callback type for face hide events. */
using OnFaceHideCb = std::function<void(int fid, int x, int y, int width, int height)>;

/** The callback type for face move events. */
using OnFaceMoveCb = std::function<void(int fid, int x, int y, int width, int height)>;

class FaceRecognizer {
  /** Registered face show callbacks. */
  std::vector<OnFaceShowCb> m_cbs_face_show;

  /** Registered face hide callbacks. */
  std::vector<OnFaceHideCb> m_cbs_face_hide;

  /** Registered face move callbacks. */
  std::vector<OnFaceMoveCb> m_cbs_face_move;

public:
  FaceRecognizer() = default;

  FaceRecognizer(const FaceRecognizer& rhs) = delete;

  FaceRecognizer(FaceRecognizer&& rhs) noexcept = delete;

  ~FaceRecognizer() = default;

  void on_face_show(OnFaceShowCb cb);

  void on_face_hide(OnFaceHideCb cb);

  void on_face_move(OnFaceMoveCb cb);

  void poll();

  void submit_frame(const char* bytes, int width, int height);
};

void FaceRecognizer::on_face_show(OnFaceShowCb cb) {
  // Register callback for face show event
  m_cbs_face_show.push_back(cb);
}

void FaceRecognizer::on_face_hide(OnFaceHideCb cb) {
  // Register callback for face hide event
  m_cbs_face_hide.push_back(cb);
}

void FaceRecognizer::on_face_move(OnFaceMoveCb cb) {
  // Register callback for face move event
  m_cbs_face_move.push_back(cb);
}

void FaceRecognizer::poll() {
}

#include <iostream>

void FaceRecognizer::submit_frame(const char* bytes, int width, int height) {
  std::cout << "frame " << width << "x" << height << ": " << (void*) bytes << "\n";
}

PYBIND11_MODULE(facelib, m) {
  py::class_<FaceRecognizer>(m, "FaceRecognizer")
    .def(py::init<>())
    .def("on_face_show", &FaceRecognizer::on_face_show)
    .def("on_face_hide", &FaceRecognizer::on_face_hide)
    .def("on_face_move", &FaceRecognizer::on_face_move)
    .def("poll", &FaceRecognizer::poll)
    .def("submit_frame", &FaceRecognizer::submit_frame);
}
