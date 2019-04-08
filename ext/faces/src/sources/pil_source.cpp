/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#include <algorithm>
#include <condition_variable>
#include <mutex>

#include <faces/sources/pil_source.h>
#include <pybind11/stl.h>

namespace faces {
namespace sources {

namespace py = pybind11;

struct PILSourceImpl {
  /** The pending frame. */
  Image m_image;

  /** The pending condition variable. This provides wait/notify primitives. */
  std::condition_variable m_cond;

  /** The pending mutex. This protects the frame and the condition variable. */
  std::mutex m_mutex;

  /** The presence indicator. */
  bool m_present;

  PILSourceImpl();
};

PILSourceImpl::PILSourceImpl()
    : m_image()
    , m_cond()
    , m_mutex()
    , m_present(false) {
}

PILSource::PILSource() : impl() {
  impl = std::make_unique<PILSourceImpl>();
}

PILSource::~PILSource() = default;

void PILSource::update(const py::object& img) {
  // The next frame
  Image image;

  // Get dimensions of image
  image.width = py::cast<int>(img.attr("width"));
  image.height = py::cast<int>(img.attr("height"));

  // Get raw bytes of the image
  // Strings are odd for working with raw bytes, but pybind11 has an implicit conversion available
  // Also, this performs the necessary memory management to keep pointers from dangling
  auto bytes = py::cast<std::string>(img.attr("tobytes")("raw"));

  // Copy image bytes into frame
  image.data.reserve(bytes.size());
  std::copy(bytes.begin(), bytes.end(), std::back_inserter(image.data));

  // Submit the frame
  {
    std::lock_guard lock(impl->m_mutex);

    // If an unread frame is still present
    if (impl->m_present) {
      std::cout << "dropping a frame\n";
    }

    // Move new frame over old frame
    // This discards the old data
    impl->m_image = std::move(image);

    // You've got mail!
    impl->m_present = true;
    impl->m_cond.notify_all();
  }
}

std::optional<Image> PILSource::wait(unsigned long millis) {
  // Acquire pending frame lock
  // This will unlock automatically when it goes out of scope
  // It can also be transferred somewhere else (like a condition variable)
  std::unique_lock lock(impl->m_mutex);

  // Wait while frame is not present
  // Note that the CV steals the lock and periodically toggles it
  // The lock above does NOT stay locked while we wait
  // When waiting is over, it puts the lock back in the variable above
  auto status = impl->m_cond.wait_for(lock, std::chrono::milliseconds(millis), [&]() {
    return impl->m_present;
  });

  // If the condition variable timed out, return nothing
  if (!status) {
    return std::nullopt;
  }

  // Mark pending frame no longer present
  impl->m_present = false;

  // Return the frame
  return impl->m_image;
}

} // namespace sources
} // namespace faces
