/*
 * InsertProjectName
 * Copyright (c) 2019 The InsertProjectName Contributors
 * InsertLicenseText
 */

#include <functional>
#include <thread>
#include <vector>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

/** An event involving a face. */
struct FaceEvent {
  /** The face ID. */
  int fid;

  /** The screen x-coordinate of the top-left of the face bounding box. */
  int x;

  /** The screen y-coordinate of the top-left of the face bounding box. */
  int y;

  /** The width of the face bounding box. */
  int width;

  /** The height of the face bounding box. */
  int height;
};

/** The callback type for face show events. */
using OnFaceShowCb = std::function<void(FaceEvent)>;

/** The callback type for face hide events. */
using OnFaceHideCb = std::function<void(FaceEvent)>;

/** The callback type for face move events. */
using OnFaceMoveCb = std::function<void(FaceEvent)>;

class FaceRecognizer {
  /** The processor thread. */
  std::thread m_thd_proc;

  /** Pending face show events. */
  std::vector<FaceEvent> m_pend_face_show;

  /** Registered face show callbacks. */
  std::vector<OnFaceShowCb> m_cbs_face_show;

  /** Pending face hide events. */
  std::vector<FaceEvent> m_pend_face_hide;

  /** Registered face hide callbacks. */
  std::vector<OnFaceHideCb> m_cbs_face_hide;

  /** Registered face moved events. */
  std::vector<FaceEvent> m_pend_face_move;

  /** Registered face move callbacks. */
  std::vector<OnFaceMoveCb> m_cbs_face_move;

public:
  FaceRecognizer() = default;

  FaceRecognizer(const FaceRecognizer& rhs) = delete;

  FaceRecognizer(FaceRecognizer&& rhs) noexcept = delete;

  ~FaceRecognizer() = default;

private:
  /** The processor function. */
  void processor();

public:
  /** Register a face show callback. */
  void on_face_show(OnFaceShowCb cb);

  /** Register a face hide callback. */
  void on_face_hide(OnFaceHideCb cb);

  /** Register a face move callback. */
  void on_face_move(OnFaceMoveCb cb);

  /** Poll for callbacks. */
  void poll();

  /** Submit a new frame. */
  void submit_frame(const char* bytes, int width, int height);

  /** Start the processor. */
  void start_processor();

  /** Stop the processor. */
  void stop_processor();
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
  // Generic event dispatcher algorithm
  auto&& dispatch = [](auto&& evts, auto&& cbs) {
    for (auto&& evt : evts) {
      for (auto&& cb : cbs) {
        cb(evt);
      }
    }
    evts.clear();
  };

  // Dispatch for all event types
  dispatch(m_pend_face_show, m_cbs_face_show);
  dispatch(m_pend_face_hide, m_cbs_face_hide);
  dispatch(m_pend_face_move, m_cbs_face_move);
}

#include <iostream>

void FaceRecognizer::submit_frame(const char* bytes, int width, int height) {
  std::cout << "frame " << width << "x" << height << ": " << (void*) bytes << "\n";

  // TODO
}

void FaceRecognizer::start_processor() {
  // TODO
}

void FaceRecognizer::stop_processor() {
  // TODO
}

PYBIND11_MODULE(facelib, m) {
  py::class_<FaceEvent>(m, "FaceEvent")
    .def_readwrite("fid", &FaceEvent::fid)
    .def_readwrite("x", &FaceEvent::x)
    .def_readwrite("y", &FaceEvent::y)
    .def_readwrite("width", &FaceEvent::width)
    .def_readwrite("height", &FaceEvent::height);

  py::class_<FaceRecognizer>(m, "FaceRecognizer")
    .def(py::init<>())
    .def("on_face_show", &FaceRecognizer::on_face_show)
    .def("on_face_hide", &FaceRecognizer::on_face_hide)
    .def("on_face_move", &FaceRecognizer::on_face_move)
    .def("poll", &FaceRecognizer::poll)
    .def("submit_frame", &FaceRecognizer::submit_frame)
    .def("start_processor", &FaceRecognizer::start_processor)
    .def("stop_processor", &FaceRecognizer::stop_processor);
}
