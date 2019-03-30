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

using FaceCallback = std::function<void()>;

class FaceRecognizer {
  std::vector<FaceCallback> m_callbacks;

public:
  FaceRecognizer() = default;

  FaceRecognizer(const FaceRecognizer& rhs) = delete;

  FaceRecognizer(FaceRecognizer&& rhs) noexcept = delete;

  ~FaceRecognizer() = default;

  void listen(FaceCallback cb);

  void poll();
};

void FaceRecognizer::listen(FaceCallback cb) {
  m_callbacks.push_back(cb);
}

void FaceRecognizer::poll() {
  // FIXME
  for (auto&& cb : m_callbacks) {
    cb();
  }
}

PYBIND11_MODULE(facelib, m) {
  py::class_<FaceRecognizer>(m, "FaceRecognizer")
    .def(py::init<>())
    .def("listen", &FaceRecognizer::listen)
    .def("poll", &FaceRecognizer::poll);
}

/*
struct facelib {
    facelib(const std::string &name) : name(name) { }
    void setName(const std::string &name_) { name = name_; }
    const std::string &getName() const { return name; }

    std::string name;
};

PYBIND11_MODULE(facelib, m) {
    py::class_<facelib>(m, "facelib")
        .def(py::init<const std::string &>())
        .def("setName", &facelib::setName)
        .def("getName", &facelib::getName);
}*/
