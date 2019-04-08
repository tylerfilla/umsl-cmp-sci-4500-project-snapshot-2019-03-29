/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <atomic>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include <faces/cache.h>
#include <faces/encoding.h>
#include <faces/recognizer.h>
#include <faces/source.h>

#include <spdyface.h>
#include <spdyface/dlib_ffd_detector.h>
#include <spdyface/dlib_v1_embedder.h>

#include "common_image.h"

namespace faces {

struct RecognizerImpl {
  /** The recognizer object. */
  Recognizer& m_recognizer;

  /** The spdyface context. */
  SFContext m_spdy;

  /** The face detector. */
  SFDlibFFDDetector m_detector;

  /** The face embedder. */
  SFDlibV1Embedder m_embedder;

  /** The last video frame received. */
  Image m_frame;

  /**
   * The spdyface common image view. The phrase "common image" is specific to
   * cozmonaut. It preserves the binary format of raw images in Python's PIL
   * (Python Image Library), which is used by the Cozmo SDK.
   */
  SFCommonImage m_com_image;

  /** The continuous recognition thread. */
  std::thread m_crt;

  /** Kill switch for the continuous recognition thread. */
  std::atomic_flag m_crt_kill;

  /** Mutex for interfacing with the continuous recognition thread. */
  std::mutex m_crt_mutex;

  /** All registered face appearance callbacks. */
  std::vector<Recognizer::CbFaceAppear> m_cbs_face_appear;

  /** All registered face disappearance callbacks. */
  std::vector<Recognizer::CbFaceDisappear> m_cbs_face_disappear;

  /** All registered face movement callbacks. */
  std::vector<Recognizer::CbFaceMove> m_cbs_face_move;

  /** Pending face appearance events. */
  std::vector<std::tuple<Recognizer&, int, std::tuple<int, int, int, int>, Encoding>> m_evts_face_appear;

  /** Pending face disappearance events. */
  std::vector<std::tuple<Recognizer&, int>> m_evts_face_disappear;

  /** Pending face movement events. */
  std::vector<std::tuple<Recognizer&, int, std::tuple<int, int, int, int>>> m_evts_face_move;

  /** The face cache. */
  Cache* m_cache;

  /** The video source. */
  Source* m_source;

  /** A map of face IDs to the numbers of frames they've been off screen. */
  std::map<int, int> m_lifetimes;

  explicit RecognizerImpl(Recognizer& p_recognizer);

  ~RecognizerImpl();

  /** Main function for the continuous recognition thread. */
  void crt_main();

  /** The continuous recognition loop. */
  void crt_loop();
};

RecognizerImpl::RecognizerImpl(Recognizer& p_recognizer)
    : m_recognizer(p_recognizer)
    , m_spdy()
    , m_detector()
    , m_embedder()
    , m_com_image()
    , m_crt()
    , m_crt_kill(true)
    , m_crt_mutex()
    , m_cbs_face_appear()
    , m_cbs_face_disappear()
    , m_cbs_face_move()
    , m_evts_face_appear()
    , m_evts_face_disappear()
    , m_evts_face_move()
    , m_cache()
    , m_source()
    , m_lifetimes() {
  // Create spdyface context
  if (sfCreate(&m_spdy)) {
    std::cerr << "Failed to create spdyface context\n";
    return;
  }

  // Create face detector
  if (sfDlibFFDDetectorCreate(&m_detector)) {
    std::cerr << "Failed to create face detector\n";
    return;
  }
  sfUseDetector(m_spdy, (SFDetector) m_detector);

  // Create face embedder
  if (sfDlibV1EmbedderCreate(&m_embedder)) {
    std::cerr << "Failed to create face embedder\n";
    return;
  }
  sfUseEmbedder(m_spdy, (SFEmbedder) m_embedder);

  // Create common image view
  if (sfCommonImageCreate(&m_com_image, m_frame)) {
    std::cerr << "Failed to create common image\n";
    return;
  }
}

RecognizerImpl::~RecognizerImpl() {
  // Clean up spdyface things
  sfCommonImageDestroy(m_com_image);
  sfDlibFFDDetectorDestroy(m_detector);
  sfDlibV1EmbedderDestroy(m_embedder);
  sfDestroy(m_spdy);
}

void RecognizerImpl::crt_main() {
  // Reset the kill switch
  m_crt_kill.test_and_set();

  // While the kill switch has not been triggered
  while (m_crt_kill.test_and_set()) {
    // Do a loop iteration
    crt_loop();
  }
}

void RecognizerImpl::crt_loop() {
  // Receive the next frame
  // Time out after one hundred milliseconds (TODO: Extract this)
  auto frame = m_source->wait(100); // Need to lock m_source

  // If no frame was received, stop the iteration
  if (!frame) {
//  std::cout << "No frame was received\n";
    return;
  }

  // Lock the interface mutex
  // We don't want things changing underneath us
  std::lock_guard lock(m_crt_mutex);

  // Move frame into view
  m_frame = std::move(*frame);

  // Detect all faces in the frame
  sfDetect(m_spdy, (SFImage) m_com_image, [](SFContext ctx, SFImage image, SFRectangle* bounds, void* user) {
    // Recover pointer to implementation struct
    auto impl = static_cast<RecognizerImpl*>(user);

    // Embed the face into a 128-dimensional vector encoding
    std::array<double, 128> vec {};
    sfEmbed(ctx, image, bounds, vec.data());

    // Construct the libfaces encoding for this face
    Encoding enc;
    enc.set_vector(vec);

    // Query for the face in the cache with a tolerance of 0.6 (TODO: Extract this)
    int id = impl->m_cache->query(enc, 0.6);

    // If the queried returned zero, ...
    if (id == 0) {
      // ...then there was a cache miss
      // THIS IS A NEVER-BEFORE-SEEN FACE

      // Insert the face into the cache with an unspecified ID
      // The cache will pick an ID to its liking and return it
      // We know for a fact (by our definition) that the ID will be negative
      // Negative IDs represent faces that the user hasn't specified explicitly
      // When the user loads up faces into the cache, they must use positive IDs
      // Our code, being above the law, can then use negative IDs for its own purposes
      id = impl->m_cache->insert_unknown(enc);
    }

    // Try to find the lifetime for this face
    // FIXME: Renames will cause faces to be lost
    auto lifetime_it = impl->m_lifetimes.find(id);

    // If no lifetime exists for this face
    if (lifetime_it == impl->m_lifetimes.end()) {
      // Enqueue an appearance event
      impl->m_evts_face_appear.emplace_back(impl->m_recognizer, id,
          std::tuple {bounds->left, bounds->top, bounds->right, bounds->bottom}, enc);
    } else {
      // Enqueue a movement event
      impl->m_evts_face_move.emplace_back(impl->m_recognizer, id,
          std::tuple {bounds->left, bounds->top, bounds->right, bounds->bottom});
    }

    // Reset the lifetime of the face
    impl->m_lifetimes[id] = 15; // TODO: Extract this

    // Returning zero means continue with faces in this frame
    // Otherwise, nonzero would tell spdyface to stop looking at this frame
    return 0;
  }, this);

  // Clean up stale face tracks
  std::vector<int> stale_tracks;
  for (auto&&[id, maturity] : m_lifetimes) {
    // Reduce all tracks' lifetimes by one
    // If a track's lifetime drops below zero, the track is stale
    if (--maturity < 0) {
      stale_tracks.push_back(id);
    }
  }
  for (auto&& id : stale_tracks) {
    m_lifetimes.erase(id);

    // Enqueue a disappearance event
    m_evts_face_disappear.emplace_back(m_recognizer, id);
  }
}

Recognizer::Recognizer() : impl() {
  impl = std::make_unique<RecognizerImpl>(*this);
}

Recognizer::~Recognizer() {
  // Make sure the continuous recognition thread is stopped
  //
  // We suppress exceptions here for two reasons:
  //  1. We may not have started the CRT yet. Attempting to stop it will be a
  //     problem (std::runtime_error), and that's just a fact of life.
  //  2. It's bad to throw stuff out of destructors, period.
  try {
    stop();
  } catch (...) {
  }
}

Cache* Recognizer::get_cache() const {
  // Lock the interface mutex
  std::lock_guard lock(impl->m_crt_mutex);

  return impl->m_cache;
}

void Recognizer::set_cache(Cache* p_cache) {
  // Lock the interface mutex
  std::lock_guard lock(impl->m_crt_mutex);

  impl->m_cache = p_cache;
}

Source* Recognizer::get_source() const {
  // Lock the interface mutex
  std::lock_guard lock(impl->m_crt_mutex);

  return impl->m_source;
}

void Recognizer::set_source(Source* p_source) {
  // Lock the interface mutex
  std::lock_guard lock(impl->m_crt_mutex);

  impl->m_source = p_source;
}

void Recognizer::register_face_appear(CbFaceAppear cb) {
  // Lock the interface mutex
  std::lock_guard lock(impl->m_crt_mutex);

  // Save the callback
  impl->m_cbs_face_appear.push_back(cb);
}

void Recognizer::register_face_disappear(CbFaceDisappear cb) {
  // Lock the interface mutex
  std::lock_guard lock(impl->m_crt_mutex);

  // Save the callback
  impl->m_cbs_face_disappear.push_back(cb);
}

void Recognizer::register_face_move(CbFaceMove cb) {
  // Lock the interface mutex
  std::lock_guard lock(impl->m_crt_mutex);

  // Save the callback
  impl->m_cbs_face_move.push_back(cb);
}

void Recognizer::start() {
  // If the continuous recognition thread is joinable
  // This would indicate the thread has not been explicitly stopped
  // Even if the thread dies due to an error, stop() must still be called
  if (impl->m_crt.joinable()) {
    throw std::runtime_error("continuous recognition thread start failed: not stopped");
  }

  // Spin up the continuous recognition thread
  impl->m_crt = std::thread(&RecognizerImpl::crt_main, impl.get());
}

void Recognizer::stop() {
  // If the continuous recognition thread is not joinable
  // This would indicate either (1) it has never started or (2) it was stopped
  if (!impl->m_crt.joinable()) {
    throw std::runtime_error("continuous recognition thread stop failed: not started");
  }

  // Trigger the kill switch and wait for the continuous recognition thread to die
  impl->m_crt_kill.clear();
  impl->m_crt.join();
}

void Recognizer::poll() {
  // Lock the interface mutex
  std::lock_guard lock(impl->m_crt_mutex);

  // Generic event dispatcher algorithm
  auto dispatch = [](auto& evts, const auto& cbs) -> void {
    // For each pending event
    for (auto& evt : evts) {
      // For each receiving callback
      for (auto& cb : cbs) {
        // Send the event to the callback
        // The events here are std::tuple objects with callback arguments
        std::apply(cb, evt);
      }
    }

    // Clear all pending events
    evts.clear();
  };

  // Dispatch all three kinds of events
  dispatch(impl->m_evts_face_appear, impl->m_cbs_face_appear);
  dispatch(impl->m_evts_face_disappear, impl->m_cbs_face_disappear);
  dispatch(impl->m_evts_face_move, impl->m_cbs_face_move);
}

} // namespace faces
