%module hypre_to_mumps
%{
#include <mpi.h>
#include <iostream>  
#include "linalg/hypre.hpp"
#include "hypre_to_mumps.hpp"
#include "mumps_c_types.h"
#include "numpy/arrayobject.h"    
#include <complex.h>
#define MFEM_USE_MPI  
%}
// initialization required to return numpy array from SWIG
%init %{
import_array();
%}

%include mpi4py/mpi4py.i
%mpi4py_typemap(Comm, MPI_Comm);
%include "mumps_c_types.h"
 //%include "linalg/hypre.hpp"
%import "hypre_int.i"

 // support passing list or python array to set_data
%typemap(in)  (MUMPS_INT *){
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
    $1 = (MUMPS_INT *) PyArray_DATA($input);
    //PyArray_CLEARFLAGS((PyArrayObject *)$input, NPY_ARRAY_OWNDATA);        
    //for (MUMPS_INT i = 0; i < 100; i++) {    
    //   std::cout << std::to_string($1[i]) << "\n";
    //}
  }
}
%typemap(in)  (DMUMPS_REAL *){
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
    $1 = (DMUMPS_REAL *) PyArray_DATA($input);
  }
}
%typemap(in)  (ZMUMPS_REAL *){
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
    $1 = (ZMUMPS_REAL *) PyArray_DATA($input);
  }
}
%typemap(typecheck, precedence=SWIG_TYPECHECK_POINTER) (MUMPS_INT *) {
  $1 = (PyList_Check($input)||PyArray_Check($input)) ? 1 : 0;  
}
%typemap(typecheck, precedence=SWIG_TYPECHECK_POINTER) (DMUMPS_REAL *) {
  $1 = (PyList_Check($input)||PyArray_Check($input)) ? 1 : 0;      
}
%typemap(typecheck, precedence=SWIG_TYPECHECK_POINTER) (ZMUMPS_REAL *) {
  $1 = (PyList_Check($input)||PyArray_Check($input)) ? 1 : 0;          
}


%typemap(in, numinputs=0) MUMPS_INT **irn_p (MUMPS_INT *tmp){
   $1 = &tmp;
}
%typemap(argout)  MUMPS_INT **irn_p{
   //std::cout << "irn_py_object " << $1 << "\n";  
   $result = SWIG_Python_AppendOutput($result,
				      SWIG_NewPointerObj(*$1,
							 $descriptor(MUMPS_INT *),
							 0));
   //std::cout << "irn_py_object " << $result << "\n";
}
%typemap(in, numinputs=0) MUMPS_INT **jcn_p (MUMPS_INT *tmp){
   $1 = &tmp;
}
%typemap(argout)  MUMPS_INT **jcn_p{
   $result = SWIG_Python_AppendOutput($result,
				      SWIG_NewPointerObj(*$1,
							 $descriptor(MUMPS_INT *),
							 0));
  //$result = SWIG_Python_AppendOutput ($result, $1)
}
%typemap(in, numinputs=0) DMUMPS_REAL **a_p (DMUMPS_REAL *tmp){
   $1 = &tmp;
}
%typemap(argout)  DMUMPS_REAL **a_p{
   $result = SWIG_Python_AppendOutput($result,
				      SWIG_NewPointerObj(*$1,
							 $descriptor(DMUMPS_REAL *),
							 0));
}

%typemap(in, numinputs=0) ZMUMPS_COMPLEX **a_p (ZMUMPS_COMPLEX *tmp){
  $1 = &tmp;
}

%typemap(argout) ZMUMPS_COMPLEX **a_p{
     $result = SWIG_Python_AppendOutput($result,
					SWIG_NewPointerObj(*$1,
						   $descriptor(ZMUMPS_COMPLEX *),
						   0));
}


/*
%typemap(in, numpinputs=0) (MUMPS_INT *irn, MUMPS_INT *jcn,  ZMUMPS_COMPLEX *a)(MUMPS_INT tmp1, MUMPS_INT tmp2,  ZMUMPS_COMPLEX tmp3) {
   $1 = &tmp1;
   $2 = &tmp2;
   $3 = &tmp3;   
}

%typemap(argout)  (MUMPS_INT *irn, MUMPS_INT *jcn,  ZMUMPS_COMPLEX *a){
  $result = SWIG_Python_AppendOutput ($result, PyCObject_FromVoidPtr($1, NULL));
  $result = SWIG_Python_AppendOutput ($result, PyCObject_FromVoidPtr($2, NULL));
  $result = SWIG_Python_AppendOutput ($result, PyCObject_FromVoidPtr($3, NULL));    
}
*/
%typemap(in, numinputs=0) (HYPRE_Int *row_start, HYPRE_Int *row_end,
			   HYPRE_Int **ppRow, HYPRE_Int **ppiCol,
			   ZMUMPS_REAL **ppRData, ZMUMPS_REAL **ppIData,
			   HYPRE_Int **ppRowStarts,
			   HYPRE_Int **ppColStarts)  
                         (HYPRE_Int rs, HYPRE_Int re,  HYPRE_Int *pRow,
			  HYPRE_Int *piCol,
			  ZMUMPS_REAL *pRdata, ZMUMPS_REAL *pIdata,
			  HYPRE_Int *pRowStarts,
			  HYPRE_Int *pColStarts) {  
  $1 = &rs;
  $2 = &re;
  $3 = &pRow;
  $4 = &piCol;
  $5 = &pRdata;
  $6 = &pIdata;
  $7 = &pRowStarts;
  $8 = &pColStarts;  
}
%typemap(argout)  (HYPRE_Int *row_start, HYPRE_Int *row_end,
	           HYPRE_Int **ppRow, HYPRE_Int **ppiCol,
 		   ZMUMPS_REAL **ppRData, ZMUMPS_REAL **ppIData,
		   HYPRE_Int **ppRowStarts,
		   HYPRE_Int **ppColStarts) {  
PyObject* temp = 
  $result = SWIG_Python_AppendOutput ($result, PyInt_FromLong(*$1));
  $result = SWIG_Python_AppendOutput ($result, PyInt_FromLong(*$2));
  $result = SWIG_Python_AppendOutput ($result, SWIG_NewPointerObj(*$3, $*3_descriptor, 0));
  $result = SWIG_Python_AppendOutput ($result, SWIG_NewPointerObj(*$4, $*4_descriptor, 0));
  $result = SWIG_Python_AppendOutput ($result, SWIG_NewPointerObj(*$5, $*5_descriptor, 0));
  $result = SWIG_Python_AppendOutput ($result, SWIG_NewPointerObj(*$6, $*6_descriptor, 0));
  $result = SWIG_Python_AppendOutput ($result, SWIG_NewPointerObj(*$7, $*7_descriptor, 0));
  $result = SWIG_Python_AppendOutput ($result, SWIG_NewPointerObj(*$8, $*8_descriptor, 0));    
  //  $result = SWIG_Python_AppendOutput ($result, PyCObject_FromVoidPtr(*$4, NULL));
  //  $result = SWIG_Python_AppendOutput ($result, PyCObject_FromVoidPtr(*$5, NULL));
  //  $result = SWIG_Python_AppendOutput ($result, PyCObject_FromVoidPtr(*$6, NULL));    
}

%pythonprepend PyZMatrix::set_idata%{
  import numpy as np
  try:
     if not args[0].flags.contiguous:
        print('non contiguous array was passed')
        args = (np.ascontiguousarray(args[0]),)
  except:
     pass
%}
%pythonprepend PyZMatrix::set_rdata%{
  import numpy as np
  try:
     if not args[0].flags.contiguous:
        print('non contiguous array was passed')
        args = (np.ascontiguousarray(args[0]),)
  except:
      pass
%}
%pythonprepend PyMatrix::set_jcn%{
  import numpy as np
  try:
     if not args[0].flags.contiguous:
        print('non contiguous array was passed')
        args = (np.ascontiguousarray(args[0], dtype = int),)
  except:
      pass
%}
%pythonprepend PyMatrix::set_irn%{
  import numpy as np
  try:
     if not args[0].flags.contiguous:
        print('non contiguous array was passed')
        args = (np.ascontiguousarray(args[0],dtype = int),)
  except:
      pass
%}

%include "hypre_to_mumps.hpp"
