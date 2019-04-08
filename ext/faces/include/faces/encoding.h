/*
 * Cozmonaut
 * Copyright (c) 2019 The Cozmonaut Contributors
 *
 * InsertLicenseText
 */

#ifndef FACES_ENCODING_H
#define FACES_ENCODING_H

#include <array>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace faces {

/** A face encoding. */
class Encoding {
public:
  /** The type of a face vector. */
  using vector_type = std::array<double, 128>;

private:
  /** The face vector. */
  vector_type m_vector;

public:
  Encoding();

  Encoding(const Encoding& rhs);

  Encoding(Encoding&& rhs) noexcept;

  ~Encoding();

  Encoding& operator=(const Encoding& rhs);

  Encoding& operator=(Encoding&& rhs) noexcept;

  /**
   * @return The face vector
   */
  vector_type get_vector() const {
    return m_vector;
  }

  /**
   * @param p_vector The face vector
   */
  void set_vector(const vector_type& p_vector) {
    m_vector = p_vector;
  }

  /**
   * Compare this face encoding with another face encoding, and return a measure
   * of their dissimilarity. Specifically, this method returns the square of the
   * Euclidean distance between this encoding's vector and the other encoding's
   * vector (aka the magnitude of the vector difference between them).
   *
   * @param rhs The other face encoding
   * @return The dissimilarity measure as specified
   */
  double compare(const Encoding& rhs) const;
};

namespace encoding {

template<class Module>
void bind(Module&& m) {
  namespace py = pybind11;

  py::class_<Encoding>(m, "Encoding")
      .def(py::init<>())
      .def_property("vector", &Encoding::get_vector, &Encoding::set_vector)
      .def("compare", &Encoding::compare);
}

} // namespace encoding
} // namespace faces

#endif // #ifndef FACES_ENCODING_H
