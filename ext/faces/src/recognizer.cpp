/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>

#include <faces/recognizer.h>
#include <faces/source.h>

#include <spdyface.h>
#include <spdyface/dlib_ffd_detector.h>
#include <spdyface/dlib_v1_embedder.h>

#include "common_image.h"

namespace faces {

struct RecognizerImpl {
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

  /** The face cache. */
  Cache* m_cache;

  /** The video source. */
  Source* m_source;

  RecognizerImpl();

  ~RecognizerImpl();

  /** Main function for the continuous recognition thread. */
  void crt_main();

  /** The continuous recognition loop. */
  void crt_loop();
};

RecognizerImpl::RecognizerImpl()
    : m_spdy()
    , m_detector()
    , m_embedder()
    , m_com_image()
    , m_crt()
    , m_crt_kill(true)
    , m_crt_mutex()
    , m_cache()
    , m_source() {
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
  // Lock the interface mutex
  // We don't want things changing underneath us
  std::lock_guard lock(m_crt_mutex);

  // Receive the next frame
  // Time out after one hundred milliseconds
  auto frame = m_source->wait(100);

  // If no frame was received, stop the iteration
  if (!frame) {
//  std::cout << "No frame was received\n";
    return;
  }

  // Move frame into view
  m_frame = std::move(*frame);

  // Detect one face
  SFRectangle rect;
  if (sfDetectOne(m_spdy, (SFImage) m_com_image, &rect)) {
    std::cout << "face: " << rect.left << ", " << rect.top << ", " << rect.right << ", " << rect.bottom << "\n";
  }
}

Recognizer::Recognizer() : impl() {
  impl = std::make_unique<RecognizerImpl>();
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

} // namespace faces
