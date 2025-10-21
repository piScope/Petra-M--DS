/**********************************************/
/*  hypre_to_mumps                            */
/**********************************************/

/* main role of this module is to  combine 
   hypre csr matrix and python matrix data
   to form a stiffness matrix */

/*
   mtype = -1: no data (need to ask other nodes..
   mtype = 0:  HYPRE_Matrix
   mtype = 1:  coo fomrat matrix in a structure
               {int nraw, int ncol, int nnz, int  *irn, int * jcn, 
               ZMUMPS_COMPLEX * data}
        
               cmajor = 0 colomun major 
               cmajor = 1 colomun major 
               cmajor = 2 sparse matrix using irn and jcn
*/



#include "mumps_c_types.h"
#include "linalg/hypre.hpp"
#include <mpi.h>

/* from abstract matrix to a matrix data from python */
class PyMatrix{
private:
  MUMPS_INT nrow;
  MUMPS_INT ncol;
  MUMPS_INT nnz;
  MUMPS_INT * irn;
  MUMPS_INT * jcn;
  int cmajor;
public:
  PyMatrix(MUMPS_INT nrow, MUMPS_INT ncol, MUMPS_INT nnz = 0, int cmajor=0);
  virtual ~PyMatrix();
  MUMPS_INT TrueNNZ();
  int NNZ(){return nnz;}
  int N(){return ncol;}
  int M(){return nrow;}
  int get_major(){return cmajor;}
  void set_major(int m){cmajor = m;}
  MUMPS_INT get_irn(MUMPS_INT i);
  MUMPS_INT get_jcn(MUMPS_INT i);
  virtual bool is_complex() = 0;
  virtual void * get_real_data_p(MUMPS_INT i){return NULL;}
  virtual void * get_imag_data_p(MUMPS_INT i){return NULL;}
  void set_coo(MUMPS_INT *irn0, MUMPS_INT *jcn0);
  void set_jcn(MUMPS_INT *jcn0);
  void set_irn(MUMPS_INT *irn0);
  MUMPS_INT get_index(MUMPS_INT r, MUMPS_INT c);
  void print_info();
  virtual bool isTrueNNZ(MUMPS_INT i) = 0;
  virtual void set_data(void *data0, MUMPS_INT i) = 0;
  void set_row(void *pdata, MUMPS_INT r);
  void set_col(void *pdata, MUMPS_INT c);
  
};
class PyEmptyMatrix: public PyMatrix{
public:
  PyEmptyMatrix(MUMPS_INT nrow, MUMPS_INT ncol, MUMPS_INT nnz = 0, int cmajor=0);
  virtual ~PyEmptyMatrix();
  virtual bool is_complex(){return false;}
  virtual bool isTrueNNZ(MUMPS_INT i){return false;}
  virtual void set_data(void *data0, MUMPS_INT i){}
};

class PyDMatrix: public PyMatrix{
private:
  DMUMPS_REAL *data, cdata;
  bool data_alloc;
public:
  PyDMatrix(MUMPS_INT nrow, MUMPS_INT ncol, MUMPS_INT nnz = 0, int cmajor = 0);
  virtual ~PyDMatrix();
  virtual bool is_complex(){return false;}
  virtual void * get_real_data_p(MUMPS_INT i);
  virtual void * get_imag_data_p(MUMPS_INT i){
    std::cout << "get_imag_data_p of PyDMatrix should not be called \n";
    return NULL;
  }
  void set_data(DMUMPS_REAL *data);
  void set_data(DMUMPS_REAL data);
  void set_data(DMUMPS_REAL data0, MUMPS_INT r, MUMPS_INT c);
  virtual void set_data(void *data0, MUMPS_INT i);
  virtual bool isTrueNNZ(MUMPS_INT i);
};

class PyZMatrix: public PyMatrix{
private:
  ZMUMPS_REAL *rdata, crdata;
  ZMUMPS_REAL *idata, cidata;
  bool r_alloc;
  bool i_alloc;
public:
  PyZMatrix(MUMPS_INT nrow, MUMPS_INT ncol, MUMPS_INT nnz = 0, int cmajor = 0);
  virtual ~PyZMatrix();
  virtual bool is_complex(){return true;}
  virtual void * get_real_data_p(MUMPS_INT i);
  virtual void * get_imag_data_p(MUMPS_INT i);
  void set_data(ZMUMPS_REAL *rdata, ZMUMPS_REAL *idata);
  void set_data(ZMUMPS_REAL data, MUMPS_INT r, MUMPS_INT c);

  /* set real/imag part of index i to data */
  /* data0 should have two elements */
  virtual void set_data(void *data0, MUMPS_INT i);
  
  void set_rdata(ZMUMPS_REAL *data);
  void set_idata(ZMUMPS_REAL *data);
  void set_rdata(ZMUMPS_REAL data);
  void set_idata(ZMUMPS_REAL data);
  void set_rdata(ZMUMPS_REAL data, MUMPS_INT r, MUMPS_INT c);
  void set_idata(ZMUMPS_REAL data, MUMPS_INT r, MUMPS_INT c);

  void set_col(ZMUMPS_REAL rdata, ZMUMPS_REAL idata, MUMPS_INT c);
  void set_row(ZMUMPS_REAL rdata, ZMUMPS_REAL idata, MUMPS_INT r);  
    
  virtual bool isTrueNNZ(MUMPS_INT i);
  void Print();
  void PrintNNZ();  
};


/* abstact class for distributed matrix for mumps*/
/* uses coordinate format */
/* is assembled from HypreParMatrix and PythonMatrix */
class MUMPS_LOC_Matrix{
private:
  MPI_Comm  comm;
  MUMPS_INT shape_jc, shape_ir, nc, nr, nnz;
  mfem::HypreParMatrix **rmatrix;
  mfem::HypreParMatrix **imatrix;
  MUMPS_INT * mtype;
  /* data set for coo format, these are one based index */  
  MUMPS_INT * irn;
  MUMPS_INT * jcn;
  MUMPS_INT * ir_start;
  MUMPS_INT * jc_start;

  bool is_irn_alloc;
  bool is_jcn_alloc;
  bool is_assembled;
  bool is_complex;
public:  
  MUMPS_LOC_Matrix(MPI_Comm comm, int m, int n, bool is_complex);
  virtual ~MUMPS_LOC_Matrix();
  int NNZ(){return nnz;}
  int N(){return nc;}
  int M(){return nr;}
  MPI_Comm Comm(){return comm;}
  void add_real_hypre_matrix(mfem::HypreParMatrix *matrix, int i);
  void add_imag_hypre_matrix(mfem::HypreParMatrix *matrix, int i);
  virtual void add_py_matrix(PyMatrix *matrix, int i) = 0;
  void share_py_matrix_info(int i, int rank);/* set matrix as data on other nodes */
  void assemble();
  void assemble_new();  
  MUMPS_INT assemble_from_hypre(mfem::HypreParMatrix * rmatrix,
				mfem::HypreParMatrix * imatrix,
				MUMPS_INT inz,
				MUMPS_INT base_i,
				MUMPS_INT base_j,
				MUMPS_INT *index);
  MUMPS_INT assemble_from_hypre_new(mfem::HypreParMatrix * rmatrix,
				    mfem::HypreParMatrix * imatrix,
				    MUMPS_INT inz,
				    MUMPS_INT base_i,
				    MUMPS_INT base_j);
  MUMPS_INT assemble_from_hypre_new(mfem::HypreParMatrix * rmatrix,
				    MUMPS_INT inz,
				    MUMPS_INT base_i,
				    MUMPS_INT base_j);
  
  MUMPS_INT assemble_from_py(PyMatrix * m,
			     MUMPS_INT innz,
			     MUMPS_INT base_i,
			     MUMPS_INT base_j);
  
  virtual void allocate_data(MUMPS_INT nnz)=0;
  void allocate_irn(MUMPS_INT nnz);
  void allocate_jcn(MUMPS_INT nnz);
  virtual void set_data(void *v, MUMPS_INT I, MUMPS_INT J, MUMPS_INT inz) = 0;
  virtual void set_data_imag(void *v, MUMPS_INT I, MUMPS_INT J,
			     MUMPS_INT inz){};
  MUMPS_INT * get_irn(){return irn;}
  MUMPS_INT * get_jcn(){return jcn;}
  void set_irn(MUMPS_INT idx, MUMPS_INT v){irn[idx] = v;}
  void set_jcn(MUMPS_INT idx, MUMPS_INT v){jcn[idx] = v;}
  virtual PyMatrix ** get_pymatrix() = 0;
  void set_mtype(int idx, int v){mtype[idx] = v;}
  MUMPS_INT * get_mtype(){return mtype;}
  int get_ntile(){return shape_jc*shape_ir;}
  void print_info();
  void print_data();
  void save_data(const char *filename); /* save assembled matrix */
  virtual std::string ToString(MUMPS_INT i) = 0;
  bool isComplex(){return is_complex;}
  bool isAssembled(){return is_assembled;}

  MUMPS_INT nnz_complex_hypre(mfem::HypreParMatrix * rmatrix,
   			      mfem::HypreParMatrix * imatrix,
			      MUMPS_INT base_i,	MUMPS_INT base_j);
  MUMPS_INT union_i_len(HYPRE_Int** j_array, HYPRE_Int** j_array2,
			MUMPS_INT num1,  MUMPS_INT num2);
  MUMPS_INT get_row_data(HYPRE_Int i, HYPRE_Int first_row_index,
			 HYPRE_Int first_col_diag,
			 HYPRE_Int *diag_i,    HYPRE_Int *diag_j,
			 HYPRE_Int *offd_i,    HYPRE_Int *offd_j,
			 HYPRE_Int *col_map_offd,    HYPRE_Complex *diag_data,
			 HYPRE_Complex *offd_data,
			 MUMPS_INT base_i,   MUMPS_INT base_j,
			 HYPRE_Int **j_array, HYPRE_Complex **d_array);

};
  
/* double precision */  
class DMUMPS_LOC_Matrix: public MUMPS_LOC_Matrix{
private:
  DMUMPS_REAL * data;
  PyDMatrix   ** pymatrix;
  bool is_data_alloc;
public:
  DMUMPS_LOC_Matrix(MPI_Comm comm, int m, int n);
  virtual ~DMUMPS_LOC_Matrix();
  virtual void add_py_matrix(PyMatrix *matrix, int i);
  virtual void allocate_data(MUMPS_INT nnz);
  virtual void set_data(void *v, MUMPS_INT I, MUMPS_INT J, MUMPS_INT inz);
  virtual PyMatrix ** get_pymatrix(){return (PyMatrix **)pymatrix;}
  DMUMPS_REAL * get_data(){return data;}
  virtual std::string ToString(MUMPS_INT i);
  virtual bool isComplex(){return false;}
};
  
/* complex double precision */    
class ZMUMPS_LOC_Matrix: public MUMPS_LOC_Matrix{
private:  
  ZMUMPS_COMPLEX * data;
  PyZMatrix   ** pymatrix;
  bool is_data_alloc;  
public:
  ZMUMPS_LOC_Matrix(MPI_Comm comm, int m, int n);
  virtual ~ZMUMPS_LOC_Matrix();
  virtual void add_py_matrix(PyMatrix *matrix, int i);
  virtual void allocate_data(MUMPS_INT nnz);
  virtual void set_data(void *v, MUMPS_INT I, MUMPS_INT J, MUMPS_INT inz);
  virtual void set_data_imag(void * v, MUMPS_INT I, MUMPS_INT J, MUMPS_INT inz);
  virtual PyMatrix ** get_pymatrix(){return (PyMatrix **)pymatrix;}
  ZMUMPS_COMPLEX * get_data(){return data;}
  virtual std::string ToString(MUMPS_INT i);
  MUMPS_INT ToCSR(HYPRE_Int *row_start, HYPRE_Int *row_end,
		  HYPRE_Int **ppRow, HYPRE_Int **ppiCol,
		  ZMUMPS_REAL **ppRData, ZMUMPS_REAL **ppIData,
		  HYPRE_Int **ppRowStarts, HYPRE_Int **ppColStarts);
  virtual bool isComplex(){return true;}  
};
  

/* from single matrix (double) */
HYPRE_Int form_mumps_local_d_array_simple(mfem::HypreParMatrix *pmatrix,
                                  const HYPRE_Int           base_i,
				  const HYPRE_Int           base_j,
    				  MUMPS_INT **irn_p, MUMPS_INT **jcn_p,
 				  DMUMPS_REAL **a_p);

/* functions */
MUMPS_INT get_local_nnz(mfem::HypreParMatrix *pmatrix);
MUMPS_INT check_nz(mfem::HypreParMatrix *pmatrix,
              const HYPRE_Int       search_i,
	      const HYPRE_Int       serach_j);
MUMPS_INT sum_nnz(mfem::HypreParMatrix *pmatrix,
		  mfem::HypreParMatrix *pmatrix2,
		  MUMPS_INT * index);
MUMPS_INT get_HypreParMatrixRow(mfem::HypreParMatrix * rmatrix,
				MUMPS_INT i);
void print_HYPRE_matrix_info(mfem::HypreParMatrix *pmatrix);
HYPRE_Int get_true_local_nnz(mfem::HypreParMatrix *pmatrix);

/* simple sort */
void argsort(MUMPS_INT *ptr, MUMPS_INT len, MUMPS_INT **res);
/* print helper */
std::string to_string(int i);
std::string to_stringf(double f);
 

