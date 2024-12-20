#define PY_SSIZE_T_CLEAN
#include "Python.h"
#include <stdlib.h>
#include <string.h>
Py_ssize_t getsizeof_fallback(PyObject *obj) {
  // since _PySys_GetSizeOf is not public
  PyObject *sys_module = PyImport_ImportModule("sys");
  if (sys_module == NULL) {
    return -1;
  }

  // Get the sys.getsizeof function
  PyObject *getsizeof_func = PyObject_GetAttrString(sys_module, "getsizeof");
  if (getsizeof_func == NULL) {
    Py_XDECREF(sys_module);
    return -1;
  }
  PyObject *size_pyobj = PyObject_CallFunctionObjArgs(getsizeof_func, obj, NULL);
  Py_XDECREF(sys_module);
  Py_XDECREF(getsizeof_func);
  if (!size_pyobj) {
    return -1;
  }
  Py_ssize_t size = PyLong_AsSize_t(size_pyobj);
  Py_XDECREF(size_pyobj);
  return size;
}
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
  Py_ssize_t osize = getsizeof_fallback(o);
  if (osize == -1) {
    return NULL;
  }
  abort();
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
  Py_RETURN_NONE;
}
PyObject *has_immortal(PyObject *self, PyObject *noargs){
  #ifdef _Py_IMMORTAL_REFCNT
  Py_RETURN_TRUE;
  #else
  Py_RETURN_FALSE;
  #endif
}
PyObject *mk_immortal(PyObject *self, PyObject *o) {
#ifdef _Py_IMMORTAL_REFCNT
  o->ob_refcnt = _Py_IMMORTAL_REFCNT;
  Py_RETURN_NONE;
#else
#warning "immortal objects don't exist in this version of Python; mk_immortal() and eternize() will not work"
  PyErr_SetString(PyExc_SystemError,
                  "immortal objects don't exist in this version of Python");
  return NULL;
#endif
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
  Py_RETURN_NONE;
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
  Py_RETURN_NONE;
}

PyObject *forceset(PyObject *self, PyObject *args) {
  PyObject *tgt, *o;
  if (!PyArg_ParseTuple(args, "OO", &tgt, &o)) {
    return NULL;
  }
  Py_ssize_t refcnt = Py_REFCNT(tgt);
  Py_ssize_t tsz = getsizeof_fallback(tgt);
  Py_ssize_t osz = getsizeof_fallback(o);
  if (osz == -1 || tsz == -1) {
    return NULL;
  }
  if (osz != tsz) {
    PyErr_SetString(PyExc_ValueError, "Objects must have the same size");
    return NULL;
  }
  memcpy((void *)tgt, (void *)o, osz);
  Py_SET_REFCNT(tgt,refcnt);
  Py_RETURN_NONE;
}

PyObject *settupleitem(PyObject *self, PyObject *args){
  PyObject *tup, *o;
  Py_ssize_t index;
  if(!PyArg_ParseTuple(args,"O!nO",&PyTuple_Type,&tup,&index,&o)){
  	return NULL;
  }
  // catch index errors before they mess stuff up
  Py_ssize_t tupsz = PyTuple_Size(tup);
  if(tupsz==-1){
	  return NULL;
  }
  if (index<0 || index>=tupsz){
  	PyErr_SetString(PyExc_IndexError, "tuple assignment index out of range");
	return NULL;
  }
  Py_ssize_t refcnt = Py_REFCNT(tup);
  Py_SET_REFCNT(tup,1);
  if(PyTuple_SetItem(tup, index, o)==-1){
  	return NULL;
  }
  Py_SET_REFCNT(tup, refcnt);
  Py_RETURN_NONE;
}
PyObject *setbytesitem(PyObject *self, PyObject *args){
  PyBytesObject *bts;
  Py_ssize_t index;
  unsigned char q;
  if(!PyArg_ParseTuple(args,"O!nc", &PyBytes_Type, &bts, &index, &q)){
  	return NULL;
  }
  Py_ssize_t btsz = PyBytes_Size((PyObject*)bts);
  if(index<0 || index>=btsz){
  	PyErr_SetString(PyExc_IndexError, "bytes assignment index out of range");
  	return NULL;
  }
  bts->ob_sval[index]=q;
  Py_RETURN_NONE;

  
}
static PyMethodDef EvilMethods[] = {
    {"id2obj", id2obj, METH_O, NULL},
    {"rawdump", rawdump, METH_O, NULL},
    {"rawload", rawload, METH_O, NULL},
    {"addrof", addrof, METH_O, NULL},
    {"getrefcount", getrefcount, METH_O, NULL},
    {"setrefcount", setrefcount, METH_VARARGS, NULL},
    {"has_immortal", has_immortal, METH_NOARGS, NULL},
    {"mk_immortal", mk_immortal, METH_O, NULL},
    {"settype", settype, METH_VARARGS, NULL},
    {"getsize", getsize, METH_O, NULL},
    {"setsize", setsize, METH_VARARGS, NULL},
    {"forceset", forceset, METH_VARARGS, NULL},
    {"settupleitem", settupleitem, METH_VARARGS, NULL},
    {"setbytesitem", setbytesitem, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL} /* Sentinel */
};
static struct PyModuleDef _evilmodule = {PyModuleDef_HEAD_INIT, "_evil", NULL,
                                         -1, EvilMethods};
PyMODINIT_FUNC PyInit__evil(void) { return PyModule_Create(&_evilmodule); }
