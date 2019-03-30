/*
 * InsertProjectName
 * Copyright (c) 2019 The InsertProjectName Contributors
 * InsertLicenseText
 */

#include <Python.h>

struct module_state {
  PyObject* error;
};

static PyObject* error_out(PyObject* module) {
  auto state = static_cast<module_state*>(PyModule_GetState(module));
  PyErr_SetString(state->error, "something bad happened");
  return nullptr;
}

static PyMethodDef facelib_methods[] = {
  {"error_out", (PyCFunction) error_out, METH_NOARGS, nullptr},
  {nullptr, nullptr},
};

static int facelib_traverse(PyObject* module, visitproc visit, void* arg) {
  auto state = static_cast<module_state*>(PyModule_GetState(module));
  Py_VISIT(state->error);
  return 0;
}

static int facelib_clear(PyObject* module) {
  auto state = static_cast<module_state*>(PyModule_GetState(module));
  Py_CLEAR(state->error);
  return 0;
}

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "facelib",
  nullptr,
  sizeof(struct module_state),
  facelib_methods,
  nullptr,
  facelib_traverse,
  facelib_clear,
  nullptr,
};

PyMODINIT_FUNC
PyInit_facelib() {
  auto module = PyModule_Create(&moduledef);

  if (!module) {
    return nullptr;
  }

  auto state = static_cast<module_state*>(PyModule_GetState(module));

  state->error = PyErr_NewException("facelib.Error", nullptr, nullptr);

  if (!state->error) {
    Py_DECREF(module);
    return nullptr;
  }

  return module;
}
