%module(package="petram.ext.mumps") mumps_solve
%{
  /*#include <mpi.h>*/
#include <iostream>
#include <complex.h>
#include "mumps_solve.hpp"
#include "mumps_c_types.h"
#include "numpy/arrayobject.h"
  //typedef struct{double real;double imag;} cdouble;
  //typedef struct{float real;float imag;} cfloat;
%}
// initialization required to return numpy array from SWIG
%init %{
import_array();
%}
%include mpi4py/mpi4py.i
%mpi4py_typemap(Comm, MPI_Comm)

%include "mumps_int_def.h"
%include "mumps_c_types.h"

typedef long long int	int64_t;

%pythonprepend libmumps_solve::DMUMPS::run %{
import sys
sys.stdout.flush()
sys.stderr.flush()
%}
%pythonprepend libmumps_solve::SMUMPS::run %{
import sys
sys.stdout.flush()
sys.stderr.flush()
%}
%pythonprepend libmumps_solve::CMUMPS::run %{
import sys
sys.stdout.flush()
sys.stderr.flush()
%}
%pythonprepend libmumps_solve::ZMUMPS::run %{
import sys
sys.stdout.flush()
sys.stderr.flush()
%}

%include "mumps_solve.hpp"

%inline %{
  int SIZEOF_MUMPS_INT(void){
    return (int) sizeof(MUMPS_INT);
  }
%}

%typemap(in)  (MUMPS_INT *arr){
  if (!(PyList_Check($input)||PyArray_Check($input))){
    PyErr_SetString(PyExc_ValueError, "Expecting a list/array or array");
    return NULL;
  }
  if (PyList_Check($input)){
      MUMPS_INT $2 = PyList_Size($input);
      $1 = (MUMPS_INT *) malloc(($2)*sizeof(MUMPS_INT));
      for (MUMPS_INT i = 0; i < $2; i++) {
	PyObject *s = PyList_GetItem($input,i);
	if (PyInt_Check(s)) {
	  $1[i] = (MUMPS_INT)PyInt_AsLong(s);
	} else {
	  free($1);
	  PyErr_SetString(PyExc_ValueError, "List items must be integer");
	  return NULL;
	}
      }
  }
  if (PyArray_Check($input)){
    $1 = (MUMPS_INT *) PyArray_DATA((const PyArrayObject*)$input);
    //PyArray_CLEARFLAGS((PyArrayObject *)$input, NPY_ARRAY_OWNDATA);
    //for (MUMPS_INT i = 0; i < 100; i++) {
    //   std::cout << std::to_string($1[i]) << "\n";
    //}
  }
}
%typemap(in)  (SMUMPS_REAL *arr){
  if (!(PyList_Check($input)||PyArray_Check($input))){
    PyErr_SetString(PyExc_ValueError, "Expecting a list/array");
    return NULL;
  }
  if (PyList_Check($input)){
     MUMPS_INT $2 = PyList_Size($input);
     $1 = (SMUMPS_REAL *) malloc(($2)*sizeof(SMUMPS_REAL));
     for (MUMPS_INT i = 0; i < $2; i++) {
        PyObject *s = PyList_GetItem($input,i);
        if (PyFloat_Check(s)) {
	  $1[i] = (SMUMPS_REAL)PyFloat_AsDouble(s);
	} else {
	  free($1);
	  PyErr_SetString(PyExc_ValueError, "List items must be float");
	  return NULL;
	}
     }
  }
  if (PyArray_Check($input)){
    $1 = (SMUMPS_REAL *) PyArray_DATA((const PyArrayObject*)$input);
  }
}

%typemap(in)  (DMUMPS_REAL *arr){
  if (!(PyList_Check($input)||PyArray_Check($input))){
    PyErr_SetString(PyExc_ValueError, "Expecting a list/array");
    return NULL;
  }
  if (PyList_Check($input)){
     MUMPS_INT $2 = PyList_Size($input);
     $1 = (DMUMPS_REAL *) malloc(($2)*sizeof(DMUMPS_REAL));
     for (MUMPS_INT i = 0; i < $2; i++) {
        PyObject *s = PyList_GetItem($input,i);
        if (PyFloat_Check(s)) {
	  $1[i] = (DMUMPS_REAL)PyFloat_AsDouble(s);
	} else {
	  free($1);
	  PyErr_SetString(PyExc_ValueError, "List items must be float");
	  return NULL;
	}
     }
  }
  if (PyArray_Check($input)){
    $1 = (DMUMPS_REAL *) PyArray_DATA((const PyArrayObject*)$input);
  }
}

%typemap(in)  (CMUMPS_REAL *arr){
  if (!(PyList_Check($input)||PyArray_Check($input))){
    PyErr_SetString(PyExc_ValueError, "Expecting a list/array");
    return NULL;
  }
  if (PyList_Check($input)){
     MUMPS_INT $2 = PyList_Size($input);
     $1 = (CMUMPS_REAL *) malloc(($2)*sizeof(CMUMPS_REAL));
     for (MUMPS_INT i = 0; i < $2; i++) {
        PyObject *s = PyList_GetItem($input,i);
        if (PyFloat_Check(s)) {
	  $1[i] = (CMUMPS_REAL)PyFloat_AsDouble(s);
	} else {
	  free($1);
	  PyErr_SetString(PyExc_ValueError, "List items must be float");
	  return NULL;
	}
     }
  }
  if (PyArray_Check($input)){
    $1 = (CMUMPS_REAL *) PyArray_DATA((const PyArrayObject*)$input);
  }
}

%typemap(in)  (ZMUMPS_REAL *arr){
  if (!(PyList_Check($input)||PyArray_Check($input))){
    PyErr_SetString(PyExc_ValueError, "Expecting a list/array");
    return NULL;
  }
  if (PyList_Check($input)){
     MUMPS_INT $2 = PyList_Size($input);
     $1 = (ZMUMPS_REAL *) malloc(($2)*sizeof(ZMUMPS_REAL));
     for (MUMPS_INT i = 0; i < $2; i++) {
        PyObject *s = PyList_GetItem($input,i);
        if (PyFloat_Check(s)) {
	  $1[i] = (ZMUMPS_REAL)PyFloat_AsDouble(s);
	} else {
	  free($1);
	  PyErr_SetString(PyExc_ValueError, "List items must be float");
	  return NULL;
	}
     }
  }
  if (PyArray_Check($input)){
    $1 = (ZMUMPS_REAL *) PyArray_DATA((const PyArrayObject*)$input);
  }
}

%typemap(in)  (CMUMPS_COMPLEX *arr){
  if (!(PyList_Check($input)||PyArray_Check($input))){
    PyErr_SetString(PyExc_ValueError, "Expecting a list/array");
    return NULL;
  }
  MUMPS_INT $2;
  if (PyList_Check($input)){
     $2 = PyList_Size($input);
     $1 = (CMUMPS_COMPLEX *) malloc(($2)*sizeof(CMUMPS_COMPLEX));
     for (MUMPS_INT i = 0; i < $2; i++) {
        PyObject *s = PyList_GetItem($input,i);
        if (PyComplex_Check(s)) {
           $1[i].r = (CMUMPS_REAL)PyComplex_RealAsDouble(s);
           $1[i].i = (CMUMPS_REAL)PyComplex_ImagAsDouble(s);
        } else {
           free($1);
           PyErr_SetString(PyExc_ValueError, "List items must be complx");
           return NULL;
        }
      }

  } else {
    $2 = PyArray_Size($input);
    $1 = (CMUMPS_COMPLEX *) PyArray_DATA((const PyArrayObject*)$input);
    //for (MUMPS_INT i = 0; i < 100; i++) {
    //   std::cout << std::to_string($1[i].r) << ", "<< std::to_string($1[i].i) << "\n";
    //}

  }
}
%typemap(in)  (ZMUMPS_COMPLEX *arr){
  if (!(PyList_Check($input)||PyArray_Check($input))){
    PyErr_SetString(PyExc_ValueError, "Expecting a list/array");
    return NULL;
  }
  MUMPS_INT $2;
  if (PyList_Check($input)){
     $2 = PyList_Size($input);
     $1 = (ZMUMPS_COMPLEX *) malloc(($2)*sizeof(ZMUMPS_COMPLEX));
     for (MUMPS_INT i = 0; i < $2; i++) {
        PyObject *s = PyList_GetItem($input,i);
        if (PyComplex_Check(s)) {
           $1[i].r = (ZMUMPS_REAL)PyComplex_RealAsDouble(s);
           $1[i].i = (ZMUMPS_REAL)PyComplex_ImagAsDouble(s);
        } else {
           free($1);
           PyErr_SetString(PyExc_ValueError, "List items must be complx");
           return NULL;
        }
      }

  } else {
    $2 = PyArray_Size($input);
    $1 = (ZMUMPS_COMPLEX *) PyArray_DATA((const PyArrayObject*)$input);
    //PyArray_CLEARFLAGS((PyArrayObject *)$input, NPY_ARRAY_OWNDATA);
    //PyObject_SetAttrString(self,"_ref_arr", $input);
    //for (MUMPS_INT i = 0; i < 100; i++) {
    //   std::cout << std::to_string($1[i].r) << ", "<< std::to_string($1[i].i) << "\n";
    //}

  }
}

%typemap(typecheck) (MUMPS_INT *arr) {
  $1 = (PyList_Check($input)||PyArray_Check($input)) ? 1 : 0;
}
%typemap(typecheck) (SMUMPS_REAL *arr) {
  $1 = (PyList_Check($input)||PyArray_Check($input)) ? 1 : 0;
}
%typemap(typecheck) (DMUMPS_REAL *arr) {
  $1 = (PyList_Check($input)||PyArray_Check($input)) ? 1 : 0;
}
%typemap(typecheck) (CMUMPS_COMPLEX *arr) {
  $1 = (PyList_Check($input)||PyArray_Check($input)) ? 1 : 0;
}
%typemap(typecheck) (ZMUMPS_COMPLEX *arr) {
  $1 = (PyList_Check($input)||PyArray_Check($input)) ? 1 : 0;
}
%typemap(typecheck) (CMUMPS_REAL *arr) {
  $1 = (PyList_Check($input)||PyArray_Check($input)) ? 1 : 0;
}
%typemap(typecheck) (ZMUMPS_REAL *arr) {
  $1 = (PyList_Check($input)||PyArray_Check($input)) ? 1 : 0;
}
/*
%pythonappend i_array%{
   setattr(val, "_ref_data", arr)
%}
%pythonappend s_array%{
   setattr(val, "_ref_data", arr)
%}
%pythonappend d_array%{
   setattr(val, "_ref_data", arr)
%}
%pythonappend c_array%{
   setattr(val, "_ref_data", arr)
%}
%pythonappend z_array%{
   setattr(val, "_ref_data", arr)
%}
%pythonappend c_real_array%{
   setattr(val, "_ref_data", arr)
%}
%pythonappend z_real_array%{
   setattr(val, "_ref_data", arr)
%}
*/
%inline %{
MUMPS_INT * i_array(MUMPS_INT *arr){return arr;}
SMUMPS_REAL * s_array(SMUMPS_REAL *arr){return arr;}
DMUMPS_REAL * d_array(DMUMPS_REAL *arr){return arr;}
CMUMPS_COMPLEX * c_array(CMUMPS_COMPLEX *arr){return arr;}
ZMUMPS_COMPLEX * z_array(ZMUMPS_COMPLEX *arr){return arr;}
CMUMPS_REAL * c_real_array(CMUMPS_REAL *arr){return arr;}
ZMUMPS_REAL * z_real_array(ZMUMPS_REAL *arr){return arr;}
%}

%clear (MUMPS_INT *arr);
%clear (SMUMPS_REAL *arr);
%clear (DMUMPS_REAL *arr);
%clear (CMUMPS_COMPLEX *arr);
%clear (ZMUMPS_COMPLEX *arr);

%inline %{
MUMPS_INT i_array_getitem(MUMPS_INT *arr, MUMPS_INT i){return arr[i];}
DMUMPS_REAL d_array_getitem(DMUMPS_REAL *arr, MUMPS_INT i){return arr[i];}
SMUMPS_REAL s_array_getitem(SMUMPS_REAL *arr, MUMPS_INT i){return arr[i];}
ZMUMPS_REAL z_array_real_getitem(ZMUMPS_COMPLEX *arr, MUMPS_INT i){return arr[i].r;}
CMUMPS_REAL c_array_real_getitem(CMUMPS_COMPLEX *arr, MUMPS_INT i){return arr[i].r;}
ZMUMPS_REAL z_array_imag_getitem(ZMUMPS_COMPLEX *arr, MUMPS_INT i){return arr[i].i;}
CMUMPS_REAL c_array_imag_getitem(CMUMPS_COMPLEX *arr, MUMPS_INT i){return arr[i].i;}
%}
%pythoncode %{
def i_to_list(A, l):
    return [i_array_getitem(A, i) for i in range(l)]
def d_to_list(A, l):
    return [d_array_getitem(A, i) for i in range(l)]
def s_to_list(A, l):
    return [s_array_getitem(A, i) for i in range(l)]
def c_to_list(A, l):
    return [c_array_real_getitem(A, i) +
	    1j* c_array_imag_getitem(A, i) for i in range(l)]
def z_to_list(A, l):
    return [z_array_real_getitem(A, i) +
	    1j* z_array_imag_getitem(A, i) for i in range(l)]
%}

%extend libmumps_solve::ZMUMPS{
  PyObject * get_real_rhs(void) {
    MUMPS_INT nrhs = self->get_struct()->nrhs;
    MUMPS_INT lrhs = self->get_struct()->lrhs;
    ZMUMPS_COMPLEX *rhs = self->get_struct()->rhs;
    npy_intp dims[] = {nrhs*lrhs};
    PyObject *pArray = PyArray_SimpleNew(1, dims, NPY_DOUBLE);
    if (!pArray){return NULL;}
    for (MUMPS_INT i=0; i< nrhs*lrhs; i++){
      ((double *)PyArray_DATA((const PyArrayObject*)pArray))[i] = (double) rhs[i].r;
    }
    return  pArray;
  }
  PyObject * get_imag_rhs(void) {
    MUMPS_INT nrhs = self->get_struct()->nrhs;
    MUMPS_INT lrhs = self->get_struct()->lrhs;
    ZMUMPS_COMPLEX *rhs = self->get_struct()->rhs;
    npy_intp dims[] = {nrhs*lrhs};
    PyObject *pArray = PyArray_SimpleNew(1, dims, NPY_DOUBLE);
    if (!pArray){return NULL;}
    for (MUMPS_INT i=0; i< nrhs*lrhs; i++){
      ((double *)PyArray_DATA((const PyArrayObject*)pArray))[i] = (double) rhs[i].i;
    }
    return  pArray;
  }
  /*
  PyObject * get_sol_loc(void) {
    MUMPS_INT nrhs = self->get_struct()->nrhs;
    MUMPS_INT lrhs = self->get_struct()->lsol_loc;
    ZMUMPS_COMPLEX *sol = self->get_struct()->sol_loc;
    npy_intp dims[] = {nrhs*lrhs};
    PyObject *pArray = PyArray_SimpleNew(1, dims, NPY_CDOUBLE);
    if (!pArray){return NULL;}

    for (MUMPS_INT i=0; i< nrhs*lrhs; i++){
      ((cdouble *)PyArray_DATA(pArray))[i].real = (double) sol[i].r;
      ((cdouble *)PyArray_DATA(pArray))[i].imag = (double) sol[i].i;
    }
    return  pArray;
  }
  */
};
%extend libmumps_solve::DMUMPS{
  PyObject * get_real_rhs(void) {
    MUMPS_INT nrhs = self->get_struct()->nrhs;
    MUMPS_INT lrhs = self->get_struct()->lrhs;
    DMUMPS_REAL *rhs = self->get_struct()->rhs;
    npy_intp dims[] = {nrhs*lrhs};
    PyObject *pArray = PyArray_SimpleNew(1, dims, NPY_DOUBLE);
    if (!pArray){return NULL;}
    for (MUMPS_INT i=0; i< nrhs*lrhs; i++){
      ((double *)PyArray_DATA((const PyArrayObject*)pArray))[i] = (double) rhs[i];
    }
    return  pArray;
  }
  /*
  PyObject * get_sol_loc(void) {
    MUMPS_INT nrhs = self->get_struct()->nrhs;
    MUMPS_INT lrhs = self->get_struct()->lsol_loc;
    DMUMPS_REAL *sol = self->get_struct()->sol_loc;
    npy_intp dims[] = {nrhs*lrhs};
    PyObject *pArray = PyArray_SimpleNew(1, dims, NPY_DOUBLE);
    if (!pArray){return NULL;}
    for (MUMPS_INT i=0; i< nrhs*lrhs; i++){
      ((double *)PyArray_DATA(pArray))[i] = (double) sol[i];
    }
    return  pArray;
  }
  */
};
%extend libmumps_solve::CMUMPS{
  PyObject * get_real_rhs(void) {
    MUMPS_INT nrhs = self->get_struct()->nrhs;
    MUMPS_INT lrhs = self->get_struct()->lrhs;
    CMUMPS_COMPLEX *rhs = self->get_struct()->rhs;
    npy_intp dims[] = {nrhs*lrhs};
    PyObject *pArray = PyArray_SimpleNew(1, dims, NPY_DOUBLE);
    if (!pArray){return NULL;}
    for (MUMPS_INT i=0; i< nrhs*lrhs; i++){
      ((double *)PyArray_DATA((const PyArrayObject*)pArray))[i] = (double) rhs[i].r;
    }
    return  pArray;
  }
  PyObject * get_imag_rhs(void) {
    MUMPS_INT nrhs = self->get_struct()->nrhs;
    MUMPS_INT lrhs = self->get_struct()->lrhs;
    CMUMPS_COMPLEX *rhs = self->get_struct()->rhs;
    npy_intp dims[] = {nrhs*lrhs};
    PyObject *pArray = PyArray_SimpleNew(1, dims, NPY_DOUBLE);
    if (!pArray){return NULL;}
    for (MUMPS_INT i=0; i< nrhs*lrhs; i++){
      ((double *)PyArray_DATA((const PyArrayObject*)pArray))[i] = (double) rhs[i].i;
    }
    return  pArray;
  }
  /*
  PyObject * get_sol_loc(void) {
    MUMPS_INT nrhs = self->get_struct()->nrhs;
    MUMPS_INT lrhs = self->get_struct()->lsol_loc;
    CMUMPS_COMPLEX *sol = self->get_struct()->sol_loc;
    npy_intp dims[] = {nrhs*lrhs};
    PyObject *pArray = PyArray_SimpleNew(1, dims, NPY_CDOUBLE);
    if (!pArray){return NULL;}
    for (MUMPS_INT i=0; i< nrhs*lrhs; i++){
      ((cdouble *)PyArray_DATA(pArray))[i].real = (double) sol[i].r;
      ((cdouble *)PyArray_DATA(pArray))[i].imag = (double) sol[i].i;
    }
    return  pArray;
  }
  */
};
%extend libmumps_solve::SMUMPS{
  PyObject * get_real_rhs(void) {
    MUMPS_INT nrhs = self->get_struct()->nrhs;
    MUMPS_INT lrhs = self->get_struct()->lrhs;
    SMUMPS_REAL *rhs = self->get_struct()->rhs;
    npy_intp dims[] = {nrhs*lrhs};
    PyObject *pArray = PyArray_SimpleNew(1, dims, NPY_DOUBLE);
    if (!pArray){return NULL;}
    for (MUMPS_INT i=0; i< nrhs*lrhs; i++){
      ((double *)PyArray_DATA((const PyArrayObject*)pArray))[i] = (double) rhs[i];
    }
    return  pArray;
  }
  /*
  PyObject * get_sol_loc(void) {
    MUMPS_INT nrhs = self->get_struct()->nrhs;
    MUMPS_INT lrhs = self->get_struct()->lsol_loc;
    SMUMPS_REAL *sol = self->get_struct()->sol_loc;
    npy_intp dims[] = {nrhs*lrhs};
    PyObject *pArray = PyArray_SimpleNew(1, dims, NPY_DOUBLE);
    if (!pArray){return NULL;}
    for (MUMPS_INT i=0; i< nrhs*lrhs; i++){
      ((double *)PyArray_DATA(pArray))[i] = (double) sol[i];
    }
    return  pArray;
  }
  */
};
