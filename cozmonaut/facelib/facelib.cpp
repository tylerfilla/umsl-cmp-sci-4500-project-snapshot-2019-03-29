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

using OnFaceShowCb = std::function<void(int fid, int x, int y, int width, int height)>;
using OnFaceHideCb = std::function<void(int fid, int x, int y, int width, int height)>;
using OnFaceMoveCb = std::function<void(int fid, int x, int y, int width, int height)>;

class FaceRecognizer {
  std::vector<OnFaceShowCb> m_cbs_face_show;
  std::vector<OnFaceHideCb> m_cbs_face_hide;
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
};

void FaceRecognizer::on_face_show(OnFaceShowCb cb) {
  m_cbs_face_show.push_back(cb);
}

void FaceRecognizer::on_face_hide(OnFaceHideCb cb) {
  m_cbs_face_hide.push_back(cb);
}

void FaceRecognizer::on_face_move(OnFaceMoveCb cb) {
  m_cbs_face_move.push_back(cb);
}

void FaceRecognizer::poll() {
  for (auto&& cb : m_cbs_face_show) {
    cb(1, 2, 3, 4, 5);
  }
  for (auto&& cb : m_cbs_face_hide) {
    cb(6, 7, 8, 9, 0);
  }
}

PYBIND11_MODULE(facelib, m) {
  py::class_<FaceRecognizer>(m, "FaceRecognizer")
    .def(py::init<>())
    .def("on_face_show", &FaceRecognizer::on_face_show)
    .def("on_face_hide", &FaceRecognizer::on_face_hide)
    .def("on_face_move", &FaceRecognizer::on_face_move)
    .def("poll", &FaceRecognizer::poll);
}
