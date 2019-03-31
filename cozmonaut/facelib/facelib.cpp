/*
 * InsertProjectName
 * Copyright (c) 2019 The InsertProjectName Contributors
 * InsertLicenseText
 */

#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// TODO: Remove these headers
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

#include <spdyface.h>
#include <spdyface_dlib.h>

namespace py = pybind11;

namespace facelib {

/** An image frame. */
struct Image {
  /** The frame width. */
  int width;

  /** The frame height. */
  int height;

  /** The raw image bytes. */
  std::string bytes;
};

/** A registry of faces. */
class Registry {
  /** The map backing store. */
  std::map<int, Image> m_store;

public:
  Registry() = default;

  Registry(const Registry& rhs) = delete;

  Registry(Registry&& rhs) noexcept = delete;

  ~Registry() = default;

  /** Register a face image with a face ID. */
  void add_face(int fid, Image image);

  /** Remove a face by its face ID. */
  void remove_face(int fid);
};

void Registry::add_face(int fid, Image image) {
  // TODO: Also precompute stats on the face
  m_store[fid] = image;
}

void Registry::remove_face(int fid) {
  m_store.erase(fid);
}

/** An event involving a face. */
struct Event {
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
using OnFaceShowCb = std::function<void(Event)>;

/** The callback type for face hide events. */
using OnFaceHideCb = std::function<void(Event)>;

/** The callback type for face move events. */
using OnFaceMoveCb = std::function<void(Event)>;

/** The face recognizer device. */
class Recognizer {
  /** The processor thread. */
  std::thread m_thd_proc;

  /** Pending incoming image frame. */
  Image m_pend_frame;

  /** Whether the pending incoming image frame is present. */
  bool m_pend_frame_present;

  /** Condition variable signalling presence of the pending incoming frame. */
  std::condition_variable m_pend_frame_cv;

  /** Mutex for pending incoming image frame. */
  std::mutex m_pend_frame_mutex;

  /** Pending outgoing face show events. */
  std::vector<Event> m_pend_face_show;

  /** Mutex for pending outgoing face show events. */
  std::mutex m_pend_face_show_mutex;

  /** Registered face show callbacks. */
  std::vector<OnFaceShowCb> m_cbs_face_show;

  /** Pending outgoing face hide events. */
  std::vector<Event> m_pend_face_hide;

  /** Mutex for pending outgoing face hide events. */
  std::mutex m_pend_face_hide_mutex;

  /** Registered face hide callbacks. */
  std::vector<OnFaceHideCb> m_cbs_face_hide;

  /** Pending outgoing face move events. */
  std::vector<Event> m_pend_face_move;

  /** Mutex for pending outgoing face move events. */
  std::mutex m_pend_face_move_mutex;

  /** Registered face move callbacks. */
  std::vector<OnFaceMoveCb> m_cbs_face_move;

public:
  /** The face registry. */
  Registry* registry;

  Recognizer() = default;

  Recognizer(const Recognizer& rhs) = delete;

  Recognizer(Recognizer&& rhs) noexcept = delete;

  ~Recognizer() = default;

private:
  /** Process a single video frame. */
  void process_frame(const Image& frame);

  /** The processor function. */
  void processor_loop();

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
  void submit_frame(Image frame);

  /** Start the processor. */
  void start_processor();

  /** Stop the processor. */
  void stop_processor();
};

static dlib::image_window window;

void Recognizer::process_frame(const Image& frame) {
  // Create an image
  dlib::array2d<dlib::rgb_pixel> frame_dlib(frame.height, frame.width);

  // Quickly scan through the frame
  // Python Image Library (PIL) uses a different format than dlib C++ library
  for (int row = 0; row < frame.height; ++row) {
    for (int col = 0; col < frame.width; ++col) {
      // Compute pixel offset
      std::size_t pixloc = 3 * (row * frame.width + col);

      // Extract color components for this pixel
      auto r = (unsigned char) frame.bytes[pixloc + 0];
      auto g = (unsigned char) frame.bytes[pixloc + 1];
      auto b = (unsigned char) frame.bytes[pixloc + 2];

      // Load up a dlib pixel
      dlib::rgb_pixel pix(r, g, b);

      // Store away the pixel in the dlib frame
      frame_dlib[row][col] = pix;
    }
  }

  window.set_image(frame_dlib);
}

void Recognizer::processor_loop() {
  do {
    // The frame to process
    Image frame;

    {
      // Acquire pending frame lock
      // This will unlock automatically when it goes out of scope
      // It can also be transferred somewhere else (like a condition variable)
      std::unique_lock<std::mutex> lock(m_pend_frame_mutex);

      // Wait while frame is not present
      // Note that the CV steals the lock and periodically toggles it
      // The lock above does NOT stay locked while we wait
      // When waiting is over, it puts the lock back in the variable above
      m_pend_frame_cv.wait(lock, [&]() {
        return m_pend_frame_present;
      });

      // Move the pending frame out for processing on our time
      frame = std::move(m_pend_frame);

      // Mark pending frame not present
      m_pend_frame_present = false;
    }

    // FINALLY, we get to process the frame!
    // And we can take our time, too!!
    process_frame(frame);
  } while (true); // TODO: Add a stop condition
}

void Recognizer::on_face_show(OnFaceShowCb cb) {
  // Register callback for face show event
  m_cbs_face_show.push_back(cb);
}

void Recognizer::on_face_hide(OnFaceHideCb cb) {
  // Register callback for face hide event
  m_cbs_face_hide.push_back(cb);
}

void Recognizer::on_face_move(OnFaceMoveCb cb) {
  // Register callback for face move event
  m_cbs_face_move.push_back(cb);
}

void Recognizer::poll() {
  // Generic event dispatcher algorithm
  auto&& dispatch = [](auto&& evts, auto&& cbs) {
    for (auto&& evt : evts) {
      for (auto&& cb : cbs) {
        cb(evt);
      }
    }
    evts.clear();
  };

  {
    // Lock and dispatch face show events
    std::lock_guard<std::mutex> lock(m_pend_face_show_mutex);
    dispatch(m_pend_face_show, m_cbs_face_show);
  }

  {
    // Lock and dispatch face hide events
    std::lock_guard<std::mutex> lock(m_pend_face_hide_mutex);
    dispatch(m_pend_face_hide, m_cbs_face_hide);
  }

  {
    // Lock and dispatch face move events
    std::lock_guard<std::mutex> lock(m_pend_face_move_mutex);
    dispatch(m_pend_face_move, m_cbs_face_move);
  }
}

void Recognizer::submit_frame(Image frame) {
  // Acquire pending frame lock
  std::lock_guard<std::mutex> lock(m_pend_frame_mutex);

  // Replace the pending frame
  m_pend_frame = std::move(frame);

  // If a pending frame is present, we have to drop it
  // This ensures we keep the pipeline going without introducing latency in the video stream
  // This wouldn't be a problem if Cozmo would just deliver a normal video stream!!
  if (m_pend_frame_present) {
    std::cout << "WARNING: DROPPING FRAME!\n";
  }

  // You've got mail!
  m_pend_frame_present = true;
  m_pend_frame_cv.notify_one();
}

void Recognizer::start_processor() {
  // Kick off the processor thread
  // TODO: Guard against double-start
  m_thd_proc = std::thread(&Recognizer::processor_loop, this);
}

void Recognizer::stop_processor() {
  // TODO: Add a stop condition
}

} // namespace facelib

PYBIND11_MODULE(facelib, m) {
  py::class_<facelib::Image>(m, "Image")
    .def(py::init<>())
    .def_readwrite("width", &facelib::Image::width)
    .def_readwrite("height", &facelib::Image::height)
    .def_readwrite("bytes", &facelib::Image::bytes);

  py::class_<facelib::Event>(m, "Event")
    .def(py::init<>())
    .def_readwrite("fid", &facelib::Event::fid)
    .def_readwrite("x", &facelib::Event::x)
    .def_readwrite("y", &facelib::Event::y)
    .def_readwrite("width", &facelib::Event::width)
    .def_readwrite("height", &facelib::Event::height);

  py::class_<facelib::Recognizer>(m, "Recognizer")
    .def(py::init<>())
    .def("on_face_show", &facelib::Recognizer::on_face_show)
    .def("on_face_hide", &facelib::Recognizer::on_face_hide)
    .def("on_face_move", &facelib::Recognizer::on_face_move)
    .def("poll", &facelib::Recognizer::poll)
    .def("submit_frame", &facelib::Recognizer::submit_frame)
    .def("start_processor", &facelib::Recognizer::start_processor)
    .def("stop_processor", &facelib::Recognizer::stop_processor)
    .def_readwrite("registry", &facelib::Recognizer::registry);

  py::class_<facelib::Registry>(m, "Registry")
    .def(py::init<>())
    .def("add_face", &facelib::Registry::add_face)
    .def("remove_face", &facelib::Registry::remove_face);
}
