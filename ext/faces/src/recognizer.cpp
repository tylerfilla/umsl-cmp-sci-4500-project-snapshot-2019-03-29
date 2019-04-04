/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <stdexcept>
#include <thread>

#include <faces/recognizer.h>

namespace faces {

struct RecognizerImpl {
  /** The continuous recognition thread. */
  std::thread m_crt;

  RecognizerImpl();

  /** Main function for the continuous recognition thread. */
  void crt_main();
};

RecognizerImpl::RecognizerImpl() : m_crt() {
}

void RecognizerImpl::crt_main() {
  // TODO
}

Recognizer::Recognizer() : impl() {
  impl = std::make_unique<RecognizerImpl>();
}

Recognizer::~Recognizer() = default;

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

  // TODO: Throw a kill switch
}

} // namespace faces
