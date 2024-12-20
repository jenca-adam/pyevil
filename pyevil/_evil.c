#include "Python.h"
#include <stdlib.h>
PyObject *id2obj(PyObject *self, PyObject *o) {
  if (!PyLong_Check(o)) {
    PyErr_SetString(PyExc_TypeError, "id2obj arg must be an int");
    return NULL;
  }
  long id = PyLong_AsLong(o);
  if (!id) {
    PyErr_SetString(PyExc_ValueError, "id must be non-zero");
    return NULL;
  }
  return (PyObject *)id;
}
PyObject *rawdump(PyObject *self, PyObject *o) {
  size_t osize = _PySys_GetSizeOf(o);
  char *mem = malloc(osize * sizeof(char)); // No need for terminator since we
                                            // use PyBytes_FromStringAndSize
  memcpy(mem, o, osize);                    // pure evil
  PyObject *bytes = PyBytes_FromStringAndSize(mem, osize);
  free(mem);
  return bytes;
}
PyObject *rawload(PyObject *self, PyObject *o) {
  // if you actually use this get ready for unexplainable segfaults
  if (!PyBytes_Check(o)) {
    PyErr_SetString(PyExc_TypeError, "rawload arg must be bytes");
  }
  char *mem = PyBytes_AsString(o);
  return (PyObject *)mem;
}

PyObject *addrof(PyObject *self, PyObject *o) {
  // for impls where id doesn't return the address
  return PyLong_FromLong((long)o);
}

PyObject *getrefcount(PyObject *self, PyObject *o) {
  return PyLong_FromSsize_t(Py_REFCNT(o));
}
PyObject *setrefcount(PyObject *self, PyObject *args) {
  PyObject *o;
  Py_ssize_t refcnt;
  if (!PyArg_ParseTuple(args, "On", &o, &refcnt)) {
    return NULL;
  }
  o->ob_refcnt = refcnt;
  return Py_None;
}
PyObject *mk_immortal(PyObject *self, PyObject *o) {
  o->ob_refcnt = _Py_IMMORTAL_REFCNT;
  return Py_None;
}
PyObject *settype(PyObject *self, PyObject *args) {
  PyObject *o;
  PyObject *type;
  if (!PyArg_ParseTuple(args, "OO", &o, &type)) {
    return NULL;
  }
  if (!PyType_Check(type)) {
    PyErr_SetString(PyExc_TypeError, "settype arg 2 must be a type");
    return NULL;
  }
  o->ob_type = (PyTypeObject *)type;
  return Py_None;
}
PyObject *getsize(PyObject *self, PyObject *o) {
  return PyLong_FromSsize_t(((PyVarObject *)o)->ob_size);
}
PyObject *setsize(PyObject *self, PyObject *args) {
  PyObject *o;
  Py_ssize_t newsize;
  if (!PyArg_ParseTuple(args, "On", &o, &newsize)) {
    return NULL;
  }
  ((PyVarObject *)o)->ob_size = newsize;
  return Py_None;
}
PyObject *forceset(PyObject *self, PyObject *args) {
  PyObject *tgt, *o;
  if (!PyArg_ParseTuple(args, "OO", &tgt, &o)) {
    return NULL;
  }
  Py_ssize_t tsz = _PySys_GetSizeOf(tgt);

  Py_ssize_t osz = _PySys_GetSizeOf(o);
  if (osz != tsz) {
    PyErr_SetString(PyExc_ValueError, "Objects must have the same size");
    return NULL;
  }

  memcpy((void *)tgt, (void *)o, osz);
  memcpy(tgt, o, osz);
  Py_INCREF(o);
  return Py_None;
}

static PyMethodDef EvilMethods[] = {
    {"id2obj", id2obj, METH_O, NULL},
    {"rawdump", rawdump, METH_O, NULL},
    {"rawload", rawload, METH_O, NULL},
    {"addrof", addrof, METH_O, NULL},
    {"getrefcount", getrefcount, METH_O, NULL},
    {"setrefcount", setrefcount, METH_VARARGS, NULL},
    {"mk_immortal", mk_immortal, METH_O, NULL},
    {"settype", settype, METH_VARARGS, NULL},
    {"getsize", getsize, METH_O, NULL},
    {"setsize", setsize, METH_VARARGS, NULL},
    {"forceset", forceset, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL} /* Sentinel */
};
static struct PyModuleDef _evilmodule = {PyModuleDef_HEAD_INIT, "_evil", NULL,
                                         -1, EvilMethods};
PyMODINIT_FUNC PyInit__evil(void) { return PyModule_Create(&_evilmodule); }
