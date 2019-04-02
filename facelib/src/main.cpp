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

#include <dlib/matrix.h>
#include <dlib/geometry/vector.h>
#include <dlib/dnn.h>
#include <dlib/image_transforms.h>
#include <dlib/clustering.h>

// TODO: Clean this entire file up (remove unneeded things, factor out duplications, remove headers...)
// TODO: Document document document...
// TODO: Remove all printfs and couts
// TODO: Let user specify, in Python, location of dlib datafiles and tolerance for face matcher

namespace py = pybind11;

namespace facelib {

/** An image in PIL raw format. */
struct Image {
  /** The frame width. */
  int width;

  /** The frame height. */
  int height;

  /** The raw image bytes. */
  std::string bytes;
};

dlib::array2d<dlib::rgb_pixel> image_to_dlib_array(const Image& image) {
  // Create a dlib array with the same size
  // The height == # rows and the width == # cols
  dlib::array2d<dlib::rgb_pixel> image_dlib(image.height, image.width);

  // Quickly scan through the image to convert its bytes
  // Python Image Library (PIL) uses a different format than dlib C++ library
  for (int row = 0; row < image.height; ++row) {
    for (int col = 0; col < image.width; ++col) {
      // Compute pixel offset into the image
      // This is the byte offset (into the PIL image) of the first pixel component
      auto pixel_offset = 3ul * (row * image.width + col);

      // Extract color components for this pixel
      // This looks into the pixel a bit further for color data
      auto r = (unsigned char) image.bytes[pixel_offset + 0];
      auto g = (unsigned char) image.bytes[pixel_offset + 1];
      auto b = (unsigned char) image.bytes[pixel_offset + 2];

      // Store away the pixel in the dlib array
      // This adopts dlib C++ format (with a custom pixel class)
      image_dlib[row][col] = dlib::rgb_pixel(r, g, b);
    }
  }

  return image_dlib;
}

/** A registry of faces. */
class Registry {
  friend class Recognizer;

  //
  // The following types are from:
  // https://github.com/davisking/dlib/blob/master/tools/python/src/face_recognition.cpp
  //

  template<template<int, template<class> class, int, class> class block, int N, template<class> class BN,
      typename SUBNET>
  using residual = dlib::add_prev1<block<N, BN, 1, dlib::tag1<SUBNET>>>;

  template<template<int, template<class> class, int, class> class block, int N, template<class> class BN,
      typename SUBNET>
  using residual_down = dlib::add_prev2<dlib::avg_pool<2, 2, 2, 2, dlib::skip1<dlib::tag2<block<N, BN, 2,
      dlib::tag1<SUBNET>>>>>>;

  template<int N, template<class> class BN, int stride, typename SUBNET>
  using block = BN<dlib::con<N, 3, 3, 1, 1, dlib::relu<BN<dlib::con<N, 3, 3, stride, stride, SUBNET>>>>>;

  template<int N, typename SUBNET>
  using ares = dlib::relu<residual<block, N, dlib::affine, SUBNET>>;

  template<int N, typename SUBNET>
  using ares_down = dlib::relu<residual_down<block, N, dlib::affine, SUBNET>>;

  template<class SUBNET>
  using alevel0 = ares_down<256, SUBNET>;

  template<class SUBNET>
  using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;

  template<class SUBNET>
  using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;

  template<class SUBNET>
  using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;

  template<class SUBNET>
  using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

  using face_recognizer_type = dlib::loss_metric<dlib::fc_no_bias<128, dlib::avg_pool_everything<alevel0<alevel1<alevel2
      <alevel3<alevel4<dlib::max_pool<3, 3, 2, 2, dlib::relu<dlib::affine<dlib::con<32, 7, 7, 2, 2,
      dlib::input_rgb_image_sized<150>>>>>>>>>>>>>;

  /** The type of encoded faces. */
  using face_encoding_type = dlib::matrix<double, 0, 1>;

  /** The map associating encoded faces with their IDs. */
  std::map<int, face_encoding_type> m_store;

  /** The face detector model. */
  dlib::frontal_face_detector m_face_detector;

  /** The face pose predictor model. */
  dlib::shape_predictor m_face_pose_predictor;

  /** The face recognizer model. */
  face_recognizer_type m_face_recognizer;

public:
  Registry();

  Registry(const Registry& rhs) = delete;

  Registry(Registry&& rhs) noexcept = delete;

  ~Registry() = default;

private:
  face_encoding_type encode_face_pose(dlib::array2d<dlib::rgb_pixel> image, dlib::full_object_detection pose);

  face_encoding_type encode_face(const Image& image);

public:
  /** Register a face image with a face ID. */
  void add_face(int fid, Image image);

  /** Remove a face by its face ID. */
  void remove_face(int fid);
};

Registry::Registry()
    : m_store()
    , m_face_detector()
    , m_face_pose_predictor()
    , m_face_recognizer() {
  // Load dlib standard frontal face detector
  m_face_detector = dlib::get_frontal_face_detector();

  // Load 68-point face pose predictor
  // This dataset generalizes a bunch of known facial feature points
  dlib::deserialize("data/shape_predictor_68_face_landmarks.dat") >> m_face_pose_predictor;

  // Load dlib face recognition network v1
  // This model extracts personally-identifying facial information
  // It takes as input pose prediction data from the model right above
  dlib::deserialize("data/dlib_face_recognition_resnet_model_v1.dat") >> m_face_recognizer;
}

Registry::face_encoding_type Registry::encode_face_pose(dlib::array2d<dlib::rgb_pixel> image,
    dlib::full_object_detection pose) {
  // The following is distilled from:
  // https://github.com/davisking/dlib/blob/master/tools/python/src/face_recognition.cpp
  // The 0.25 value is the chip padding
  std::vector<dlib::chip_details> dets(1, dlib::get_face_chip_details(pose, 150, 0.25));
  dlib::array<dlib::matrix<dlib::rgb_pixel>> face_chips;
  dlib::extract_image_chips(image, dets, face_chips);

  // Get the personally-identifying face encoding
  return dlib::matrix_cast<double>(m_face_recognizer(face_chips, 16)[0]);
}

Registry::face_encoding_type Registry::encode_face(const Image& image) {
  // Convert image to dlib array
  auto image_dlib = image_to_dlib_array(image);

  // Upscale image to improve face recognition accuracy
  dlib::pyramid_up(image_dlib);

  // Find the first face in the image
  auto face = m_face_detector(image_dlib)[0];

  // Predict pose of the face
  auto pose = m_face_pose_predictor(image_dlib, face);

  // Encode the face from its pose
  return encode_face_pose(std::move(image_dlib), pose);
}

void Registry::add_face(int fid, Image image) {
  m_store[fid] = encode_face(image);
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

  /** The face tracker. */
  std::map<int, long> m_faces;

public:
  /** The face registry. */
  Registry* registry;

  Recognizer() = default;

  Recognizer(const Recognizer& rhs) = delete;

  Recognizer(Recognizer&& rhs) noexcept = delete;

  ~Recognizer() = default;

private:
  /** Keep the given face alive. */
  void face_heartbeat(int fid, int l, int t, int r, int b);

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

// FIXME: Remove this eventually
static dlib::image_window window;

void Recognizer::face_heartbeat(int fid, int l, int t, int r, int b) {
  std::cout << "FACE " << fid << "\n";

  // If this face was not already in frame
  if (m_faces.find(fid) == m_faces.end()) {
    {
      // Lock and push face show event
      std::lock_guard lock(m_pend_face_show_mutex);
      m_pend_face_show.push_back(Event {fid, l, t, r - l, b - t});
    }
  } else {
    {
      // Lock and push face move event
      std::lock_guard lock(m_pend_face_move_mutex);
      m_pend_face_move.push_back(Event {fid, l, t, r - l, b - t});
    }
  }

  // Reset lifetime counter for face
  m_faces[fid] = 0;
}

void Recognizer::process_frame(const Image& frame) {
  // Convert frame to dlib array
  auto frame_dlib = image_to_dlib_array(frame);

  // Upscale frame to improve face recognition accuracy
  dlib::pyramid_up(frame_dlib);

  // Blur the image a bit to remove transient artifacts
  // This tries to tone down the horrible speckling and banding in Cozmo's camera
  dlib::array2d<dlib::rgb_pixel> frame_dlib_blurred(frame.height, frame.width);
  dlib::gaussian_blur(frame_dlib, frame_dlib_blurred, 1);

  // FIXME: Remove this eventually
  std::vector<dlib::full_object_detection> shapes;

  // Find all faces in the frame
  for (auto face : registry->m_face_detector(frame_dlib_blurred)) {
    // Predict the pose of the face
    auto pose = registry->m_face_pose_predictor(frame_dlib_blurred, face);

    // FIXME: Remove this eventually
    shapes.push_back(pose);

    // Encode the face pose
    auto encoding = registry->encode_face_pose(std::move(frame_dlib_blurred), pose); // FIXME: const ref rather than move?

    // Whether or not the face was matched
    bool matched = false;

    // Go over all known encodings
    // TODO: Maybe we can speed this up with some sort of hashing?
    for (auto [k, v] : registry->m_store) {
      // Compute difference between this and that encoding
      auto diff = encoding - v;

      // Compute dot product of difference vector with itself
      // This is just Euclidean distance without the square root
      // NOTE: diff is a column vector, so its data is vertical
      double dot = 0;
      for (int row = 0; row < diff.nr(); ++row) {
        dot += diff(row, 0) * diff(row, 0);
      }

      // Check against tolerance
      // TODO: Make this configurable
      if (dot <= 0.6 * 0.6) {
        face_heartbeat(k, face.left(), face.top(), face.right(), face.bottom());

        // We're using a first match algorithm here
        // TODO: Eventually we can select the best match
        matched = true;
        break;
      }
    }

    if (!matched) {
      // Lock and push face show event
      // TODO: Eventually add a new type of event for unrecognized faces (we could also track them, too)
      std::lock_guard lock(m_pend_face_show_mutex);
      m_pend_face_show.push_back(Event {-1, face.left(), face.top(), face.right(), face.bottom()});
    }
  }

  // Handle hidden faces
  std::vector<int> pruned_faces;
  for (auto&& [fid, lifetime] : m_faces) {
    // If face is gone for a certain number of frames, it's dead
    if (lifetime > 3) { // TODO: Make this configurable
      std::cout << "LOST " << fid << "\n";

      // A list of pruned faces
      pruned_faces.push_back(fid);

      {
        // Lock and push face hide event
        std::lock_guard lock(m_pend_face_hide_mutex);
        m_pend_face_hide.push_back(Event {fid, 0, 0, 0, 0}); // TODO: Don't use standard Event struct?
      }
    }

    // Increment lifetime on all faces
    lifetime++;
  }
  for (auto fid : pruned_faces) {
    m_faces.erase(fid);
  }

  // FIXME: Remove this eventually
  window.clear_overlay();
  window.set_image(frame_dlib);
  window.add_overlay(dlib::render_face_detections(shapes));
}

void Recognizer::processor_loop() {
  do {
    // The frame to process
    Image frame;

    {
      // Acquire pending frame lock
      // This will unlock automatically when it goes out of scope
      // It can also be transferred somewhere else (like a condition variable)
      std::unique_lock lock(m_pend_frame_mutex);

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
    std::lock_guard lock(m_pend_face_show_mutex);
    dispatch(m_pend_face_show, m_cbs_face_show);
  }

  {
    // Lock and dispatch face hide events
    std::lock_guard lock(m_pend_face_hide_mutex);
    dispatch(m_pend_face_hide, m_cbs_face_hide);
  }

  {
    // Lock and dispatch face move events
    std::lock_guard lock(m_pend_face_move_mutex);
    dispatch(m_pend_face_move, m_cbs_face_move);
  }
}

void Recognizer::submit_frame(Image frame) {
  // Acquire pending frame lock
  std::lock_guard lock(m_pend_frame_mutex);

  // Replace the pending frame
  m_pend_frame = std::move(frame);

  // If a pending frame is present, we have to drop it
  // This ensures we keep the pipeline going without introducing latency in the video stream
  // This wouldn't be a problem if Cozmo would just deliver a normal video stream!!
  if (m_pend_frame_present) {
//  std::cout << "WARNING: DROPPING FRAME!\n";
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
