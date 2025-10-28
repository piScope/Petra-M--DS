
/*
 Utility to generate MUMPS input array
 from HypreParMatrix of mfem
*/
#include <mpi.h>
#include <iostream>
#include <fstream>
#include "mumps_c_types.h"
#include "dmumps_c.h"
#include "_hypre_parcsr_mv.h"
#include "_hypre_parcsr_ls.h"
#include "seq_mv.h"
#include "hypre_to_mumps.hpp"
#define DEBUG false
PyMatrix::PyMatrix(MUMPS_INT nrow, MUMPS_INT ncol, MUMPS_INT nnz, int cmajor):nrow(nrow), ncol(ncol), nnz(nnz), cmajor(cmajor){
}
void PyMatrix::set_coo(MUMPS_INT *irn0, MUMPS_INT *jcn0){
  irn = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)*NNZ());
  jcn = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)*NNZ());
  
  for (MUMPS_INT i = 0; i < NNZ(); i++){
    irn[i] = irn0[i];
    jcn[i] = jcn0[i];    
  }
  cmajor = 2;
}
void PyMatrix::set_jcn(MUMPS_INT *jcn0){
  jcn = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)*NNZ());
  
  for (MUMPS_INT i = 0; i < NNZ(); i++){
    jcn[i] = jcn0[i];    
  }
  cmajor = 2;
}
void PyMatrix::set_irn(MUMPS_INT *irn0){
  irn = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)*NNZ());
  
  for (MUMPS_INT i = 0; i < NNZ(); i++){
    irn[i] = irn0[i];
  }
  cmajor = 2;
}

MUMPS_INT PyMatrix::get_irn(MUMPS_INT i){
  if (cmajor == 2){
    return irn[i];
  } 
  else if (cmajor == 0){
    return i - (i/nrow)*nrow;  /* column major */
  }
  else {
    return (i/ncol);  /* row major */
  }  
}
MUMPS_INT PyMatrix::get_jcn(MUMPS_INT i){
  if (cmajor == 2){  
     return jcn[i];
  }
  else if (cmajor == 0){    
    return i/nrow; /* column major */
  }
  else {
    return i - (i/ncol)*ncol; /* row major */    
  }
}

MUMPS_INT PyMatrix::get_index(MUMPS_INT r, MUMPS_INT c){
  if (r >= nrow){ return -1;}
  if (c >= ncol){ return -1;}  
  if (cmajor == 2){
      for (MUMPS_INT i = 0; i < NNZ(); i++){
	if (irn[i] == r && jcn[i] == c){return i;}
      }
  }
  else if (cmajor == 0){    
    return c/nrow + r; /* column major */
  }
  else {
    return r/ncol + c;
  }
  return -1;
}

void PyMatrix::set_row(void *pdata, MUMPS_INT r){
   if (cmajor == 2){
      for (MUMPS_INT i = 0; i < NNZ(); i++){
	if (irn[i] == r){
	  set_data(pdata, i);
	}
      }
   }
   else if (cmajor == 0){
      for (MUMPS_INT i = 0; i < ncol; i++){
	  set_data(pdata, i*nrow + r);      
      }
   }
   else {
      for (MUMPS_INT i = 0; i < ncol; i++){
	  set_data(pdata, r*ncol + i);      
      }
   }
}

void PyMatrix::set_col(void *pdata, MUMPS_INT c){
   if (cmajor == 2){
      for (MUMPS_INT i = 0; i < NNZ(); i++){
	if (jcn[i] == c){
	  set_data(pdata, i);
	}
      }
   }
   else if (cmajor == 0){
      for (MUMPS_INT i = 0; i < nrow; i++){
	  set_data(pdata, c*nrow + i);
      }
   }
   else {
      for (MUMPS_INT i = 0; i < nrow; i++){
	  set_data(pdata, i*ncol + c);
      }
   }
}

void PyMatrix::print_info(){
  std::cout << "M:" << to_string(nrow) << " N:" 
            << to_string(ncol) << " NNZ:" 
            << to_string(nnz) << "\n";
}

MUMPS_INT PyMatrix::TrueNNZ(){
  MUMPS_INT nnz = 0;
  for (MUMPS_INT i=0; i < NNZ(); i++){
    if (isTrueNNZ(i)) { nnz++;}
  }
  return nnz;
}

PyMatrix::~PyMatrix(){}

PyEmptyMatrix::PyEmptyMatrix(MUMPS_INT nrow, MUMPS_INT ncol, MUMPS_INT nnz, int cmajor):PyMatrix(nrow, ncol, 0, 0){
}
PyEmptyMatrix::~PyEmptyMatrix(){
}

PyDMatrix::PyDMatrix(MUMPS_INT nrow, MUMPS_INT ncol, MUMPS_INT nnz, int cmajor):PyMatrix(nrow, ncol, nnz, cmajor), data_alloc(false), cdata(0.0){
}
void PyDMatrix::set_data(DMUMPS_REAL *data0){
  if (!data_alloc){
    data = (DMUMPS_REAL *)malloc(sizeof(DMUMPS_REAL)*NNZ());
    data_alloc = true;
  }
  for (MUMPS_INT i=0 ; i < NNZ() ; i++){data[i] = data0[i];}    
}
void PyDMatrix::set_data(DMUMPS_REAL data0, MUMPS_INT r, MUMPS_INT c){
  MUMPS_INT i =  get_index(r, c);
  if (i == -1) {return;}
  if (data_alloc){
    data[i] = data0;
  }
}
void PyDMatrix::set_data(void *data0, MUMPS_INT i){
  if (data_alloc){
    data[i] = * (static_cast<DMUMPS_REAL *>(data0));
  }
}
void PyDMatrix::set_data(DMUMPS_REAL data0){
  if (data_alloc){
    free(data);
    data_alloc = false;
  }
  cdata = data0;
}

void * PyDMatrix::get_real_data_p(MUMPS_INT i){
  if (data_alloc){
    return &data[i];
  } else {
    return &cdata;
  }
}
bool PyDMatrix::isTrueNNZ(MUMPS_INT i){
  return  ((*(DMUMPS_REAL *) get_imag_data_p(i)) != 0.0);
}


PyDMatrix::~PyDMatrix(){
  if (data_alloc){free(data);}
}

/* Z */
PyZMatrix::PyZMatrix(MUMPS_INT nrow, MUMPS_INT ncol, MUMPS_INT nnz, int cmajor):PyMatrix(nrow, ncol, nnz, cmajor), r_alloc(false), i_alloc(false), crdata(0.0), cidata(0.0){
}
void PyZMatrix::set_data(ZMUMPS_REAL *rdata0, ZMUMPS_REAL *idata0){
  set_rdata(rdata0);
  set_idata(idata0);  
}
void PyZMatrix::set_data(ZMUMPS_REAL data, MUMPS_INT r, MUMPS_INT c){
  set_rdata(data, r, c);
  set_idata(0.0);
}
void PyZMatrix::set_data(void *data0, MUMPS_INT i){
  if (r_alloc){
    rdata[i] = static_cast<ZMUMPS_REAL *>(data0)[0];    
  }
  if (i_alloc){
    idata[i] = static_cast<ZMUMPS_REAL *>(data0)[1];    
  }
}
void PyZMatrix::set_rdata(ZMUMPS_REAL *rdata0){
  if (!r_alloc){
    rdata = (ZMUMPS_REAL *)malloc(sizeof(ZMUMPS_REAL)*NNZ());
    r_alloc = true;
  }
  for (MUMPS_INT i=0 ; i < NNZ() ; i++){
    rdata[i] = rdata0[i];
  }
}
void PyZMatrix::set_idata(ZMUMPS_REAL *idata0){
  if (!i_alloc){
    idata = (ZMUMPS_REAL *)malloc(sizeof(ZMUMPS_REAL)*NNZ());
    i_alloc = true;
  }
  long nnz = 0;
  for (MUMPS_INT i=0 ; i < NNZ() ; i++){
    idata[i] = idata0[i];
    if (idata[i] != 0.0){nnz = nnz+1;}
  }
  //std::cout <<
  //    "idata nnz" << std::to_string(nnz) << "\n " ;
  
}
void PyZMatrix::set_rdata(ZMUMPS_REAL data){
  if (r_alloc){
    free(rdata);
    r_alloc = false;
  }
  crdata = data;
}
void PyZMatrix::set_idata(ZMUMPS_REAL data){
  if (i_alloc){
    free(idata);
    i_alloc = false;
  }
  cidata = data;
}
void PyZMatrix::set_rdata(ZMUMPS_REAL data, MUMPS_INT r, MUMPS_INT c){
  MUMPS_INT i =  get_index(r, c);
  if (i == -1) {return;}  
  if (r_alloc){
    rdata[i] = data;
 }
}
void PyZMatrix::set_idata(ZMUMPS_REAL data, MUMPS_INT r, MUMPS_INT c){
  MUMPS_INT i =  get_index(r, c);
  if (i == -1) {return;}    
  if (i_alloc){
    idata[i] = data;
 }
}
void * PyZMatrix::get_real_data_p(MUMPS_INT i){
  if (r_alloc){
    return &(rdata[i]);
  } else {
    return &crdata;
  }
}
void * PyZMatrix::get_imag_data_p(MUMPS_INT i){
  if (i_alloc){
    return &(idata[i]);
  } else {
    return &cidata;
  }
}
bool PyZMatrix::isTrueNNZ(MUMPS_INT i){
  bool imag_zero = *((ZMUMPS_REAL *)get_imag_data_p(i)) != 0.0;
  bool real_zero = *((ZMUMPS_REAL *)get_real_data_p(i)) != 0.0;
  return imag_zero || real_zero;
}

void PyZMatrix::set_row(ZMUMPS_REAL rdata,
			ZMUMPS_REAL idata,
			MUMPS_INT r){
  ZMUMPS_REAL data[] = {rdata, idata};
  PyMatrix::set_row(&data, r);
}
void PyZMatrix::set_col(ZMUMPS_REAL rdata,
			ZMUMPS_REAL idata,
			MUMPS_INT c){
  ZMUMPS_REAL data[] = {rdata, idata};  
  PyMatrix::set_col(&data, c);  
}

void PyZMatrix::Print(){
    for (MUMPS_INT i = 0; i < NNZ(); i++){
      ZMUMPS_REAL rvalue = (*(ZMUMPS_REAL *)get_real_data_p(i));
      ZMUMPS_REAL ivalue = (*(ZMUMPS_REAL *)get_imag_data_p(i));      
      std::cout <<
	"r: " << to_string(get_irn(i)) << " " <<
	"c: " << to_string(get_jcn(i)) << " " <<		       
        "  " << to_string(rvalue) << 
        "  " << to_string(ivalue) << "\n";   
  }
}
void PyZMatrix::PrintNNZ(){
    for (MUMPS_INT i = 0; i < NNZ(); i++){
      if (!isTrueNNZ(i)) {continue;}
      ZMUMPS_REAL rvalue = (*(ZMUMPS_REAL *)get_real_data_p(i));
      ZMUMPS_REAL ivalue = (*(ZMUMPS_REAL *)get_imag_data_p(i));      
      std::cout <<
	"r: " << to_string(get_irn(i)) << " " <<
	"c: " << to_string(get_jcn(i)) << " " <<		       
        "  " << to_string(rvalue) << 
        "  " << to_string(ivalue) << "\n";   
  }
}

PyZMatrix::~PyZMatrix(){
  if (r_alloc){free(rdata);}
  if (i_alloc){free(idata);}
}

/* MUMPS_LOC_Matrix */
MUMPS_LOC_Matrix::MUMPS_LOC_Matrix(MPI_Comm comm, int m, int n, bool is_complex):comm(comm), is_irn_alloc(false), is_jcn_alloc(false), shape_ir(m), shape_jc(n), is_assembled(false), is_complex(is_complex){
  mtype = (MUMPS_INT *) malloc(sizeof(MUMPS_INT *)*m*n);
  for (int i = 0; i<m*n ; i++){mtype[i]=-1;}
  rmatrix = (mfem::HypreParMatrix **) calloc(m*n, sizeof(mfem::HypreParMatrix *));
  imatrix = (mfem::HypreParMatrix **) calloc(m*n,sizeof(mfem::HypreParMatrix *));
  nnz = 0;
  nc = 0;
  nr = 0;
}
void MUMPS_LOC_Matrix::allocate_irn(MUMPS_INT nnz){
    irn = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)*nnz);
    is_irn_alloc = true;
}
void MUMPS_LOC_Matrix::allocate_jcn(MUMPS_INT nnz){
    jcn = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)*nnz);
    is_jcn_alloc = true;
}  
void MUMPS_LOC_Matrix::add_real_hypre_matrix(mfem::HypreParMatrix *matrix, int i){
  rmatrix[i] = matrix;
  set_mtype(i, 0);
}
void MUMPS_LOC_Matrix::add_imag_hypre_matrix(mfem::HypreParMatrix *matrix, int i){
  imatrix[i] = matrix;
  set_mtype(i, 0);
}
void MUMPS_LOC_Matrix::share_py_matrix_info(int i, int rank){
  int myid;
  int data[3];

  MPI_Comm_rank(comm, &myid);
  PyMatrix ** pymatrix = get_pymatrix();

  if (myid == rank){
    data[0] = pymatrix[i] -> M();
    data[1] = pymatrix[i] -> N();
    data[2] = pymatrix[i] -> NNZ();
    //    std::cout << std::to_string(data[0]) << " " 
    //              << std::to_string(data[1]) << " " 
    //              << std::to_string(data[2]) << "\n"; 
              
  }
  MPI_Bcast((void*) data,  3,  MPI_INT,  rank  , comm);

  if (myid != rank){
    if (!pymatrix[i]){
      // std::cout << std::to_string(data[0]) << " " 
      //        << std::to_string(data[1]) << " " 
      //        << std::to_string(data[2]) << "\n"; 
      
       PyMatrix *matrix = new PyEmptyMatrix(data[0], data[1]); /* i need to clean this */
       pymatrix[i] = matrix;
       //pymatrix[i]->print_info();
       set_mtype(i, 1);
    }
  }
}

MUMPS_LOC_Matrix::~MUMPS_LOC_Matrix(){
  if (is_irn_alloc) free(irn);
  if (is_jcn_alloc) free(jcn);
  free(rmatrix);
  free(imatrix);
  free(mtype);
  if (is_assembled){
    free(jc_start);
    free(ir_start);
  }
}
/* DMUMPS_LOC_Matrix */
DMUMPS_LOC_Matrix::DMUMPS_LOC_Matrix(MPI_Comm comm, int m, int n)
  :MUMPS_LOC_Matrix(comm, m,n, false){
  is_data_alloc = false;
  pymatrix = (PyDMatrix **)calloc(m*n, sizeof(PyDMatrix *));
}
void DMUMPS_LOC_Matrix::add_py_matrix(PyMatrix *matrix, int i){
  pymatrix[i] = dynamic_cast<PyDMatrix *> (matrix);
  set_mtype(i, 1);    
}
void DMUMPS_LOC_Matrix::allocate_data(MUMPS_INT nnz){
    data = (DMUMPS_REAL *)malloc(sizeof(DMUMPS_REAL )*nnz);
    for (MUMPS_INT i=0; i<nnz ; i++){
      data[i] = 0.0;
    }
    
    is_data_alloc = true;
}
void DMUMPS_LOC_Matrix::set_data(void *v, MUMPS_INT I, MUMPS_INT J,
					 MUMPS_INT inz){
    data[inz] = *(DMUMPS_REAL *)v;
    set_irn(inz, I);
    set_jcn(inz, J);
}
DMUMPS_LOC_Matrix::~DMUMPS_LOC_Matrix(){
    if (is_data_alloc) free(data);        
    free(pymatrix);
}									\

/* ZMUMPS_LOC_Matrix */
ZMUMPS_LOC_Matrix::ZMUMPS_LOC_Matrix(MPI_Comm comm, int m, int n)
  :MUMPS_LOC_Matrix(comm, m,n, true){
  is_data_alloc = false;
  pymatrix = (PyZMatrix **)calloc(m*n, sizeof(PyZMatrix *));
}

void ZMUMPS_LOC_Matrix::add_py_matrix(PyMatrix *matrix, int i){
  pymatrix[i] = dynamic_cast<PyZMatrix *> (matrix);
  set_mtype(i, 1);
  MUMPS_INT *t = get_mtype();
  //std::cout << "!!!!!!!"<<std::to_string(pymatrix[i]->NNZ()) << "\n";

}
void ZMUMPS_LOC_Matrix::allocate_data(MUMPS_INT nnz){
    data = (ZMUMPS_COMPLEX *)malloc(sizeof(ZMUMPS_COMPLEX )*nnz);
    for (MUMPS_INT i=0; i<nnz ; i++){
      data[i].r = 0.0;
      data[i].i = 0.0;      
    }
    is_data_alloc = true;
}
void ZMUMPS_LOC_Matrix::set_data(void *v, MUMPS_INT I, MUMPS_INT J,
					 MUMPS_INT inz){
    data[inz].r = *(ZMUMPS_REAL *)v;
    set_irn(inz, I);
    set_jcn(inz, J);
}
void ZMUMPS_LOC_Matrix::set_data_imag(void *v, MUMPS_INT I, MUMPS_INT J,
					 MUMPS_INT inz){
    data[inz].i = *(ZMUMPS_REAL *)v;
    set_irn(inz, I);
    set_jcn(inz, J);
    /*
     std::cout << std::to_string(I) << " " <<
                 std::to_string(J) << " " <<
                 std::to_string(data[inz].i) << "\n"; 
    */
}

ZMUMPS_LOC_Matrix::~ZMUMPS_LOC_Matrix(){
    if (is_data_alloc) free(data);
    free(pymatrix);
}									\

void MUMPS_LOC_Matrix::assemble(){
  int myid;
  int data[3];
  MPI_Comm_rank(comm, &myid);

  MUMPS_INT len = shape_ir * shape_jc;
  
  /* count nnz */  
  nnz = 0;
  MUMPS_INT ** index_ptr = (MUMPS_INT **)calloc(len, sizeof(MUMPS_INT *));
  PyMatrix **pymatrix;
  pymatrix = get_pymatrix();
  for (int i = 0; i < len; i++){
    if (DEBUG){
      std::cout << "counting nnz, mtype" << to_string(mtype[i]) <<
	to_string(myid) << "\n";
    }
    if (mtype[i] == 0){
      if (imatrix[i]){
         std::cout << "complex matrix" << to_string(mtype[i]) << "\n";	
	 index_ptr[i] = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)*get_true_local_nnz(imatrix[i]));
         nnz = nnz + sum_nnz(rmatrix[i], imatrix[i], index_ptr[i]);
      }else{
         std::cout << "real matrix" << to_string(mtype[i]) << "\n"; 
 	 nnz = nnz + get_true_local_nnz(rmatrix[i]);
      }
    } else if (mtype[i] == 1) {
      nnz = nnz + pymatrix[i]->TrueNNZ();
    } else {
      /* nodata on this node */
    }
    if (DEBUG){
      std::cout << to_string(nnz) << "\n";
    }
  }
  /* decide the size of matrix */
  /*   
     for each row and column at least one element
     should have a data. Otherwise, the matrix has
     completely empty row or column
  */
  nr = 0;
  nc = 0;
  jc_start = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)* shape_jc);
  ir_start = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)* shape_ir);

  for (int i = 0; i < shape_ir; i++){
    ir_start[i] = nr;
    for (int j = 0; j < shape_jc; j++){    
      if (mtype[i*shape_jc+j] == 0){
	nr = nr + rmatrix[i*shape_jc+j]->M();
	break;
      } else if (mtype[i*shape_jc+j] == 1){
	nr = nr + pymatrix[i*shape_jc+j]->M();
	break;
      } else {
	;
      }
    }
  }
    
  for (int j = 0; j < shape_jc; j++){
    jc_start[j] = nc;    
    for (int i = 0; i < shape_ir; i++){        
      if (mtype[i*shape_jc+j] == 0){
	nc = nc + rmatrix[i*shape_jc+j]->N();
	break;
      } else if (mtype[i*shape_jc+j] == 1){
	nc = nc + pymatrix[i*shape_jc+j]->N();
	break;
      } else {
	;
      }
    }
  }

  if (DEBUG){
    if (myid == 0){
       std::cout<<"start index\n";     
     for (int i = 0; i < shape_jc; i++){
       std::cout<<to_string(jc_start[i])<<"\n";
     }
     std::cout<<to_string(nc)<<"\n";       
     for (int i = 0; i < shape_ir; i++){
       std::cout<<to_string(ir_start[i])<<"\n";
     }
     std::cout<<to_string(nr)<<"\n";
    }
  }
  
  /* allocate memory */
  allocate_irn(nnz);
  allocate_jcn(nnz);  
  allocate_data(nnz);

  MUMPS_INT inz = 0;
  int k = 0;
  if (DEBUG){
     std::cout << "shapr_ir " << to_string(shape_ir) << "\n";
     std::cout << "shapr_jc " << to_string(shape_jc) << "\n";
  }
  for (int i = 0; i < shape_ir; i++){
      for (int j = 0; j < shape_jc; j++){    
         if (mtype[k] == 0){
           //std::cout << "nnz " << std::to_string(nnz) << "\n";	   
           inz = assemble_from_hypre(rmatrix[k], imatrix[k], inz,
				     ir_start[i], jc_start[j], index_ptr[k]);
           if (imatrix[i]){free(index_ptr[i]);}
         } else if (mtype[k] == 1){
           inz = assemble_from_py(pymatrix[k], inz, ir_start[i], jc_start[j]);
	 } else {
	   /* nodata on this node */
	 }
	 k++;
      }
  }
  is_assembled = true;  
}

void MUMPS_LOC_Matrix::assemble_new(){
  int myid;
  int data[3];
  MPI_Comm_rank(comm, &myid);

  MUMPS_INT len = shape_ir * shape_jc;
  
  /* count nnz */  
  nnz = 0;
  MUMPS_INT ** index_ptr = (MUMPS_INT **)calloc(len, sizeof(MUMPS_INT *));
  PyMatrix **pymatrix;
  pymatrix = get_pymatrix();
  for (int i = 0; i < len; i++){
    if (DEBUG){
       std::cout << "counting nnz, mtype" << to_string(mtype[i]) <<
	  to_string(myid) << "\n";
    }
    if (mtype[i] == 0){
      if (imatrix[i]){
	if (DEBUG){
	       std::cout << "complex matrix" << to_string(mtype[i]) << "\n";
	}
	nnz = nnz + nnz_complex_hypre(rmatrix[i], imatrix[i], 0, 0);
      }else{
	if (DEBUG){	
	  std::cout << "real matrix" << to_string(mtype[i]) << "\n";
	}
	nnz = nnz + get_true_local_nnz(rmatrix[i]);
      }
    } else if (mtype[i] == 1) {
      nnz = nnz + pymatrix[i]->TrueNNZ();
    } else {
      /* nodata on this node */
    }
    if (DEBUG){
      std::cout << to_string(nnz) << "\n";
    }
  }
  /* decide the size of matrix */
  /*   
     for each row and column at least one element
     should have a data. Otherwise, the matrix has
     completely empty row or column
  */
  nr = 0;
  nc = 0;
  jc_start = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)* shape_jc);
  ir_start = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)* shape_ir);

  for (int i = 0; i < shape_ir; i++){
    ir_start[i] = nr;
    for (int j = 0; j < shape_jc; j++){    
      if (mtype[i*shape_jc+j] == 0){
	nr = nr + rmatrix[i*shape_jc+j]->M();
	break;
      } else if (mtype[i*shape_jc+j] == 1){
	nr = nr + pymatrix[i*shape_jc+j]->M();
	break;
      } else {
	;
      }
    }
  }
 
  if (DEBUG) {
    if (myid == 0){std::cout<<"counting jc\n";}
  }
  for (int j = 0; j < shape_jc; j++){
    if (myid == 0) {std::cout<< to_string(nc) << "\n";}
    jc_start[j] = nc;    
    for (int i = 0; i < shape_ir; i++){        
      if (mtype[i*shape_jc+j] == 0){
	nc = nc + rmatrix[i*shape_jc+j]->N();
        if (myid == 0) {std::cout<< to_string(nc) << " here 1 \n";}
	break;
      } else if (mtype[i*shape_jc+j] == 1){
	nc = nc + pymatrix[i*shape_jc+j]->N();
        if (DEBUG && myid == 0) {
	  pymatrix[i*shape_jc+j]->print_info();
          std::cout<< to_string(nc) << " here 2 \n";}
	break;
      } else {
	;
      }
    }
  }
  if (myid == 0) {std::cout<< to_string(nc) << "\n";}
  if (DEBUG){
    if (myid == 0){
       std::cout<<"start index\n";     
     for (int i = 0; i < shape_jc; i++){
       std::cout<<to_string(jc_start[i])<<"\n";
     }
     std::cout<<to_string(nc)<<"\n";       
     for (int i = 0; i < shape_ir; i++){
       std::cout<<to_string(ir_start[i])<<"\n";
     }
     std::cout<<to_string(nr)<<"\n";
    }
    std::cout<<"allocate memory " << to_string(myid) << " "
             << to_string(nnz)<<"\n";    
  }
  
  /* allocate memory */
  allocate_irn(nnz);
  allocate_jcn(nnz);  
  allocate_data(nnz);

  MUMPS_INT inz = 0;
  int k = 0;
  if (DEBUG){
     std::cout << "shapr_ir " << to_string(shape_ir) << "\n";
     std::cout << "shapr_jc " << to_string(shape_jc) << "\n";
  }
  for (int i = 0; i < shape_ir; i++){
      for (int j = 0; j < shape_jc; j++){    
         if (mtype[k] == 0){
           //std::cout << "nnz " << std::to_string(nnz) << "\n";
           if (imatrix[i]){
	     inz = assemble_from_hypre_new(rmatrix[k], imatrix[k], inz,
					   ir_start[i], jc_start[j]);
	   } else {
	     inz = assemble_from_hypre_new(rmatrix[k], inz,
					   ir_start[i], jc_start[j]);
	   }
         } else if (mtype[k] == 1){
           inz = assemble_from_py(pymatrix[k], inz, ir_start[i], jc_start[j]);
	 } else {
	   /* nodata on this node */
	 }
	 k++;
      }
  }
  is_assembled = true;  
}

MUMPS_INT MUMPS_LOC_Matrix::assemble_from_hypre(mfem::HypreParMatrix * rmatrix,
						mfem::HypreParMatrix * imatrix,
						MUMPS_INT inz,
						MUMPS_INT base_i,
						MUMPS_INT base_j,
						MUMPS_INT *index){
   /* first simply copy everything from real part */
   hypre_ParCSRMatrix *matrix =  static_cast<hypre_ParCSRMatrix *>(*rmatrix);  
   if (!matrix)
   {
      /*hypre_error_in_arg(1);*/
     return 0;
   }
   HYPRE_Complex    *offd_data;
   HYPRE_Int        *offd_j;
   HYPRE_Int         myid, num_procs, i, j, I, J;
   HYPRE_Int         num_nonzeros_offd;
   HYPRE_Int         ilower, iupper, jlower, jupper;
   
   MPI_Comm             comm = hypre_ParCSRMatrixComm(matrix);
   hypre_MPI_Comm_rank(comm, &myid);
   hypre_MPI_Comm_size(comm, &num_procs);

   HYPRE_Int first_row_index = hypre_ParCSRMatrixFirstRowIndex(matrix);
   HYPRE_Int first_col_diag  = hypre_ParCSRMatrixFirstColDiag(matrix);
   hypre_CSRMatrix *diag     = hypre_ParCSRMatrixDiag(matrix);
   hypre_CSRMatrix *offd     = hypre_ParCSRMatrixOffd(matrix);
   HYPRE_Int *col_map_offd   = hypre_ParCSRMatrixColMapOffd(matrix);
   HYPRE_Int num_rows        = hypre_ParCSRMatrixNumRows(matrix);
   HYPRE_Int *row_starts     = hypre_ParCSRMatrixRowStarts(matrix);
   HYPRE_Int *col_starts     = hypre_ParCSRMatrixColStarts(matrix);
   num_nonzeros_offd = hypre_CSRMatrixNumNonzeros(offd);

   HYPRE_Complex *diag_data  = hypre_CSRMatrixData(diag);
   HYPRE_Int *diag_i    = hypre_CSRMatrixI(diag);
   HYPRE_Int *diag_j    = hypre_CSRMatrixJ(diag);
   HYPRE_Int *offd_i    = hypre_CSRMatrixI(offd);
   if (num_nonzeros_offd)
   {
      offd_data = hypre_CSRMatrixData(offd);
      offd_j    = hypre_CSRMatrixJ(offd);
   }

#ifdef HYPRE_NO_GLOBAL_PARTITION
   ilower = row_starts[0]+base_i;
   iupper = row_starts[1]+base_i - 1;
   jlower = col_starts[0]+base_j;
   jupper = col_starts[1]+base_j - 1;
#else
   ilower = row_starts[myid]  +base_i;
   iupper = row_starts[myid+1]+base_i - 1;
   jlower = col_starts[myid]  +base_j;
   jupper = col_starts[myid+1]+base_j - 1;
#endif
   for (i = 0; i < num_rows; i++)
   {
      I = first_row_index + i + base_i;
      for (j = diag_i[i]; j < diag_i[i+1]; j++)
      {
         J = first_col_diag + diag_j[j] + base_j;
         if ( diag_data ){
	   if (diag_data[j] != 0){
	     set_data(&diag_data[j], I+1, J+1, inz);
	     inz ++;
	   }
	 }
      }
      if ( num_nonzeros_offd )
      {
         for (j = offd_i[i]; j < offd_i[i+1]; j++)
         {
            J = col_map_offd[offd_j[j]] + base_j;
            if ( offd_data ) {
	      if (offd_data[j] != 0){
		set_data(&offd_data[j], I+1, J+1, inz);
		inz ++;
	      }
	    }
         }
      }
   }

   if (!imatrix) return inz;
   
   /* second loop for imaginary part */
   matrix =  static_cast<hypre_ParCSRMatrix *>(*imatrix);  
   if (!matrix)
   {
      /*hypre_error_in_arg(1);*/
     return 0;
   }
   first_row_index = hypre_ParCSRMatrixFirstRowIndex(matrix);
   first_col_diag  = hypre_ParCSRMatrixFirstColDiag(matrix);
   diag     = hypre_ParCSRMatrixDiag(matrix);
   offd     = hypre_ParCSRMatrixOffd(matrix);
   col_map_offd   = hypre_ParCSRMatrixColMapOffd(matrix);
   num_rows        = hypre_ParCSRMatrixNumRows(matrix);
   row_starts     = hypre_ParCSRMatrixRowStarts(matrix);
   col_starts     = hypre_ParCSRMatrixColStarts(matrix);
   num_nonzeros_offd = hypre_CSRMatrixNumNonzeros(offd);

   diag_data  = hypre_CSRMatrixData(diag);
   diag_i    = hypre_CSRMatrixI(diag);
   diag_j    = hypre_CSRMatrixJ(diag);
   offd_i    = hypre_CSRMatrixI(offd);
   if (num_nonzeros_offd)
   {
      offd_data = hypre_CSRMatrixData(offd);
      offd_j    = hypre_CSRMatrixJ(offd);
   }

#ifdef HYPRE_NO_GLOBAL_PARTITION
   ilower = row_starts[0]+base_i;
   iupper = row_starts[1]+base_i - 1;
   jlower = col_starts[0]+base_j;
   jupper = col_starts[1]+base_j - 1;
#else
   ilower = row_starts[myid]  +base_i;
   iupper = row_starts[myid+1]+base_i - 1;
   jlower = col_starts[myid]  +base_j;
   jupper = col_starts[myid+1]+base_j - 1;
#endif
   MUMPS_INT idx2 = 0;
   for (i = 0; i < num_rows; i++)
   {
      I = first_row_index + i + base_i;
      for (j = diag_i[i]; j < diag_i[i+1]; j++)
      {
         J = first_col_diag + diag_j[j] + base_j;
         if ( diag_data ){
	   if ( diag_data[j] != 0.0 ){
           //std::cout << "idx " << std::to_string(index[idx2]) << "\n";    
	      if (index[idx2] == -1){
		set_data_imag(&(diag_data[j]), I+1, J+1, inz);
		inz ++;
	      } else {
		set_data_imag(&(diag_data[j]), I+1, J+1, index[idx2]);
	      }
           //std::cout << "idx2 " << std::to_string(idx2) << "\n";
	      idx2 ++;
	    }

         }
      }
      if ( num_nonzeros_offd )
      {
         for (j = offd_i[i]; j < offd_i[i+1]; j++)
         {
            J = col_map_offd[offd_j[j]] + base_j;
            if ( offd_data ) {
	      if ( offd_data[j] != 0.0 ){
		if (index[idx2] == -1){
		  set_data_imag(&(offd_data[j]), I+1, J+1, inz);
		  inz ++;
		} else {
		  set_data_imag(&(offd_data[j]), I+1, J+1, index[idx2]);
		}
              //std::cout << "idx2 " << std::to_string(idx2) << "\n";	      
		idx2 ++;
	      }
	    }
         }
      }
   }

   return inz;
}

MUMPS_INT  MUMPS_LOC_Matrix::get_row_data(HYPRE_Int i, HYPRE_Int first_row_index,
				    HYPRE_Int first_col_diag,
				    HYPRE_Int *diag_i,
				    HYPRE_Int *diag_j,
				    HYPRE_Int *offd_i,
				    HYPRE_Int *offd_j,
				    HYPRE_Int *col_map_offd,
				    HYPRE_Complex *diag_data,
				    HYPRE_Complex *offd_data,
				    MUMPS_INT base_i,
				    MUMPS_INT base_j,
				    HYPRE_Int **j_array,
				    HYPRE_Complex **d_array){

  MUMPS_INT  inz;
  HYPRE_Int  j, J;
  inz = 0;
  // count inz
  //I = first_row_index + i + base_i;
  for (j = diag_i[i]; j < diag_i[i+1]; j++)
    {
      J = first_col_diag + diag_j[j] + base_j;
      if ( diag_data ){
	if (diag_data[j] != 0){
	  //	  set_data(&diag_data[j], I+1, J+1, inz);
	  inz ++;
	}
      }
    }
  if (offd_i){
    for (j = offd_i[i]; j < offd_i[i+1]; j++){
      J = col_map_offd[offd_j[j]] + base_j;
      if ( offd_data ) {
	if (offd_data[j] != 0){
	  //set_data(&offd_data[j], I+1, J+1, inz);
	  inz ++;
	}
      }
    }
  }
  *j_array  =(HYPRE_Int *)malloc(sizeof(HYPRE_Int *)* inz);
  *d_array  =  (HYPRE_Complex *)malloc(sizeof(HYPRE_Complex *)* inz);
  //I = first_row_index + i + base_i;
  inz = 0;
  for (j = diag_i[i]; j < diag_i[i+1]; j++)
    {
      J = first_col_diag + diag_j[j] + base_j;
      if ( diag_data ){
	if (diag_data[j] != 0){
	  (*j_array)[inz] = J;
	  (*d_array)[inz] = diag_data[j];
	  inz ++;
	}
      }
    }
  if (offd_i){
    for (j = offd_i[i]; j < offd_i[i+1]; j++){
      J = col_map_offd[offd_j[j]] + base_j;
      if ( offd_data ) {
	if (offd_data[j] != 0){
	  (*j_array)[inz] = J;
	  (*d_array)[inz] = offd_data[j];
	  inz ++;
	}
      }
    }
  }
  //for (j=0; j<inz; j++){
  //  std::cout << "data: " << std::to_string((*j_array)[j]) << "\n";
  //}
  return inz;
}

MUMPS_INT  MUMPS_LOC_Matrix::union_i_len(HYPRE_Int** j_array, HYPRE_Int** j_array2,
					 MUMPS_INT num1,  MUMPS_INT num2){
  /* length of union array */
  MUMPS_INT i, j, len;
  len = 0;
  for (j=0; j<num2; j++){
    for (i=0; i<num1; i++){
      if ((*j_array)[i] == (*j_array2)[j]){
	  len ++;
	  break;
	}
    }
  }
  return num1+num2-len;
}
					 
MUMPS_INT MUMPS_LOC_Matrix::nnz_complex_hypre(mfem::HypreParMatrix * rmatrix,
   					      mfem::HypreParMatrix * imatrix,
 					      MUMPS_INT base_i,	MUMPS_INT base_j){

   /* first simply copy everything from real part */
   hypre_ParCSRMatrix *matrix  =  static_cast<hypre_ParCSRMatrix *>(*rmatrix);
   hypre_ParCSRMatrix *matrix2 =  static_cast<hypre_ParCSRMatrix *>(*imatrix);     
   if (!matrix)
   {
      /*hypre_error_in_arg(1);*/
     return 0;
   }
   HYPRE_Complex    *offd_data, *offd_data2;
   HYPRE_Int        *offd_j, *offd_j2;
   HYPRE_Int         myid, num_procs, i, j;
   HYPRE_Int         num_nonzeros_offd, num_nonzeros_offd2;
   HYPRE_Int         ilower, iupper, jlower, jupper;
   
   MPI_Comm             comm = hypre_ParCSRMatrixComm(matrix);
   hypre_MPI_Comm_rank(comm, &myid);
   hypre_MPI_Comm_size(comm, &num_procs);

   HYPRE_Int first_row_index = hypre_ParCSRMatrixFirstRowIndex(matrix);
   HYPRE_Int first_col_diag  = hypre_ParCSRMatrixFirstColDiag(matrix);
   hypre_CSRMatrix *diag     = hypre_ParCSRMatrixDiag(matrix);
   hypre_CSRMatrix *offd     = hypre_ParCSRMatrixOffd(matrix);
   HYPRE_Int *col_map_offd   = hypre_ParCSRMatrixColMapOffd(matrix);
   HYPRE_Int num_rows        = hypre_ParCSRMatrixNumRows(matrix);
   HYPRE_Int *row_starts     = hypre_ParCSRMatrixRowStarts(matrix);
   HYPRE_Int *col_starts     = hypre_ParCSRMatrixColStarts(matrix);
   num_nonzeros_offd = hypre_CSRMatrixNumNonzeros(offd);

   HYPRE_Complex *diag_data  = hypre_CSRMatrixData(diag);
   HYPRE_Int *diag_i    = hypre_CSRMatrixI(diag);
   HYPRE_Int *diag_j    = hypre_CSRMatrixJ(diag);
   HYPRE_Int *offd_i    = hypre_CSRMatrixI(offd);
   if (num_nonzeros_offd)
   {
      offd_data = hypre_CSRMatrixData(offd);
      offd_j    = hypre_CSRMatrixJ(offd);
   } else {
     offd_data = NULL;
     offd_j    = NULL;
   }
   
   HYPRE_Int first_row_index2 = hypre_ParCSRMatrixFirstRowIndex(matrix2);
   HYPRE_Int first_col_diag2  = hypre_ParCSRMatrixFirstColDiag(matrix2);
   hypre_CSRMatrix *diag2     = hypre_ParCSRMatrixDiag(matrix2);
   hypre_CSRMatrix *offd2     = hypre_ParCSRMatrixOffd(matrix2);
   HYPRE_Int *col_map_offd2   = hypre_ParCSRMatrixColMapOffd(matrix2);
   HYPRE_Int num_rows2        = hypre_ParCSRMatrixNumRows(matrix2);
   HYPRE_Int *row_starts2     = hypre_ParCSRMatrixRowStarts(matrix2);
   HYPRE_Int *col_starts2     = hypre_ParCSRMatrixColStarts(matrix2);
   num_nonzeros_offd2 = hypre_CSRMatrixNumNonzeros(offd2);

   HYPRE_Complex *diag_data2  = hypre_CSRMatrixData(diag2);
   HYPRE_Int *diag_i2    = hypre_CSRMatrixI(diag2);
   HYPRE_Int *diag_j2    = hypre_CSRMatrixJ(diag2);
   HYPRE_Int *offd_i2    = hypre_CSRMatrixI(offd2);
   if (num_nonzeros_offd2)
   {
      offd_data2 = hypre_CSRMatrixData(offd2);
      offd_j2    = hypre_CSRMatrixJ(offd2);
   } else {
     offd_data2 = NULL;
     offd_j2    = NULL;
   }   

#ifdef HYPRE_NO_GLOBAL_PARTITION
   ilower = row_starts[0]+base_i;
   iupper = row_starts[1]+base_i - 1;
   jlower = col_starts[0]+base_j;
   jupper = col_starts[1]+base_j - 1;
#else
   ilower = row_starts[myid]  +base_i;
   iupper = row_starts[myid+1]+base_i - 1;
   jlower = col_starts[myid]  +base_j;
   jupper = col_starts[myid+1]+base_j - 1;
#endif

   HYPRE_Int *j_array, *j_array2;
   HYPRE_Complex *d_array, *d_array2;
   MUMPS_INT num1, num2, nnz;
   nnz = 0;
   for (i = 0; i < num_rows; i++)
   {
     //std::cout << "i: " << std::to_string(i) << "\n";     
     num1 = get_row_data(i, first_row_index, first_col_diag,
			 diag_i, diag_j, offd_i, offd_j,
			 col_map_offd,
			 diag_data, offd_data, base_i, base_j,
			 &j_array, &d_array);
     //std::cout << std::to_string(num1) << "\n";
     num2 = get_row_data(i, first_row_index2, first_col_diag2,
			 diag_i2, diag_j2, offd_i2, offd_j2,
			 col_map_offd2, diag_data2, offd_data2, base_i, base_j,
			 &j_array2, &d_array2);
     //std::cout << std::to_string(num2) << "\n";     
     nnz = nnz + union_i_len(&j_array, &j_array2, num1, num2);

     free(j_array);
     free(d_array);
     free(j_array2);
     free(d_array2);     
     
   }
   if (DEBUG){
       std::cout << "after nnz: " << to_string(nnz) << "\n";
   }
   return nnz;
}
MUMPS_INT MUMPS_LOC_Matrix::assemble_from_hypre_new(mfem::HypreParMatrix * rmatrix,
						   MUMPS_INT innz,
						   MUMPS_INT base_i,
						   MUMPS_INT base_j){
   /* first simply copy everything from real part */
   hypre_ParCSRMatrix *matrix  =  static_cast<hypre_ParCSRMatrix *>(*rmatrix);
   if (!matrix)
   {
      /*hypre_error_in_arg(1);*/
     return 0;
   }
   HYPRE_Complex    *offd_data, *offd_data2;
   HYPRE_Int        *offd_j, *offd_j2;
   HYPRE_Int         myid, num_procs, i, j, I;
   HYPRE_Int         num_nonzeros_offd;
   HYPRE_Int         ilower, iupper, jlower, jupper;
   
   MPI_Comm             comm = hypre_ParCSRMatrixComm(matrix);
   hypre_MPI_Comm_rank(comm, &myid);
   hypre_MPI_Comm_size(comm, &num_procs);

   HYPRE_Int first_row_index = hypre_ParCSRMatrixFirstRowIndex(matrix);
   HYPRE_Int first_col_diag  = hypre_ParCSRMatrixFirstColDiag(matrix);
   hypre_CSRMatrix *diag     = hypre_ParCSRMatrixDiag(matrix);
   hypre_CSRMatrix *offd     = hypre_ParCSRMatrixOffd(matrix);
   HYPRE_Int *col_map_offd   = hypre_ParCSRMatrixColMapOffd(matrix);
   HYPRE_Int num_rows        = hypre_ParCSRMatrixNumRows(matrix);
   HYPRE_Int *row_starts     = hypre_ParCSRMatrixRowStarts(matrix);
   HYPRE_Int *col_starts     = hypre_ParCSRMatrixColStarts(matrix);
   num_nonzeros_offd = hypre_CSRMatrixNumNonzeros(offd);

   HYPRE_Complex *diag_data  = hypre_CSRMatrixData(diag);
   HYPRE_Int *diag_i    = hypre_CSRMatrixI(diag);
   HYPRE_Int *diag_j    = hypre_CSRMatrixJ(diag);
   HYPRE_Int *offd_i    = hypre_CSRMatrixI(offd);
   if (num_nonzeros_offd)
   {
      offd_data = hypre_CSRMatrixData(offd);
      offd_j    = hypre_CSRMatrixJ(offd);
   } else {
     offd_data = NULL;
     offd_j    = NULL;
   }
   HYPRE_Int *j_array;
   HYPRE_Complex *d_array;
   MUMPS_INT num1, nnz;
   for (i = 0; i < num_rows; i++)
   {
     I = first_row_index + i + base_i;     
     num1 = get_row_data(i, first_row_index, first_col_diag,
			 diag_i, diag_j, offd_i, offd_j,
			 col_map_offd,
			 diag_data, offd_data, base_i, base_j,
			 &j_array, &d_array);
     for (j = 0; j < num1; j++){
       set_data(&(d_array[j]), I+1, j_array[j]+1, innz);
       innz ++;
     }
     free(j_array);
     free(d_array);
   }
   return innz;
}
 
MUMPS_INT MUMPS_LOC_Matrix::assemble_from_hypre_new(mfem::HypreParMatrix * rmatrix,
						   mfem::HypreParMatrix * imatrix,
						   MUMPS_INT innz,
						   MUMPS_INT base_i,
						   MUMPS_INT base_j){

   /* first simply copy everything from real part */
   hypre_ParCSRMatrix *matrix  =  static_cast<hypre_ParCSRMatrix *>(*rmatrix);
   hypre_ParCSRMatrix *matrix2 =  static_cast<hypre_ParCSRMatrix *>(*imatrix);     
   if (!matrix)
   {
      /*hypre_error_in_arg(1);*/
     return 0;
   }
   HYPRE_Complex    *offd_data, *offd_data2;
   HYPRE_Int        *offd_j, *offd_j2;
   HYPRE_Int         myid, num_procs, i, j, k, I;
   HYPRE_Int         num_nonzeros_offd, num_nonzeros_offd2;
   HYPRE_Int         ilower, iupper, jlower, jupper;
   
   MPI_Comm             comm = hypre_ParCSRMatrixComm(matrix);
   hypre_MPI_Comm_rank(comm, &myid);
   hypre_MPI_Comm_size(comm, &num_procs);

   HYPRE_Int first_row_index = hypre_ParCSRMatrixFirstRowIndex(matrix);
   HYPRE_Int first_col_diag  = hypre_ParCSRMatrixFirstColDiag(matrix);
   hypre_CSRMatrix *diag     = hypre_ParCSRMatrixDiag(matrix);
   hypre_CSRMatrix *offd     = hypre_ParCSRMatrixOffd(matrix);
   HYPRE_Int *col_map_offd   = hypre_ParCSRMatrixColMapOffd(matrix);
   HYPRE_Int num_rows        = hypre_ParCSRMatrixNumRows(matrix);
   HYPRE_Int *row_starts     = hypre_ParCSRMatrixRowStarts(matrix);
   HYPRE_Int *col_starts     = hypre_ParCSRMatrixColStarts(matrix);
   num_nonzeros_offd = hypre_CSRMatrixNumNonzeros(offd);

   HYPRE_Complex *diag_data  = hypre_CSRMatrixData(diag);
   HYPRE_Int *diag_i    = hypre_CSRMatrixI(diag);
   HYPRE_Int *diag_j    = hypre_CSRMatrixJ(diag);
   HYPRE_Int *offd_i    = hypre_CSRMatrixI(offd);
   if (num_nonzeros_offd)
   {
      offd_data = hypre_CSRMatrixData(offd);
      offd_j    = hypre_CSRMatrixJ(offd);
   } else {
     offd_data = NULL;
     offd_j    = NULL;
   }
   
   HYPRE_Int first_row_index2 = hypre_ParCSRMatrixFirstRowIndex(matrix2);
   HYPRE_Int first_col_diag2  = hypre_ParCSRMatrixFirstColDiag(matrix2);
   hypre_CSRMatrix *diag2     = hypre_ParCSRMatrixDiag(matrix2);
   hypre_CSRMatrix *offd2     = hypre_ParCSRMatrixOffd(matrix2);
   HYPRE_Int *col_map_offd2   = hypre_ParCSRMatrixColMapOffd(matrix2);
   HYPRE_Int num_rows2        = hypre_ParCSRMatrixNumRows(matrix2);
   HYPRE_Int *row_starts2     = hypre_ParCSRMatrixRowStarts(matrix2);
   HYPRE_Int *col_starts2     = hypre_ParCSRMatrixColStarts(matrix2);
   num_nonzeros_offd2 = hypre_CSRMatrixNumNonzeros(offd2);

   HYPRE_Complex *diag_data2  = hypre_CSRMatrixData(diag2);
   HYPRE_Int *diag_i2    = hypre_CSRMatrixI(diag2);
   HYPRE_Int *diag_j2    = hypre_CSRMatrixJ(diag2);
   HYPRE_Int *offd_i2    = hypre_CSRMatrixI(offd2);
   if (num_nonzeros_offd2)
   {
      offd_data2 = hypre_CSRMatrixData(offd2);
      offd_j2    = hypre_CSRMatrixJ(offd2);
   } else {
     offd_data2 = NULL;
     offd_j2    = NULL;
   }   

#ifdef HYPRE_NO_GLOBAL_PARTITION
   ilower = row_starts[0]+base_i;
   iupper = row_starts[1]+base_i - 1;
   jlower = col_starts[0]+base_j;
   jupper = col_starts[1]+base_j - 1;
#else
   ilower = row_starts[myid]  +base_i;
   iupper = row_starts[myid+1]+base_i - 1;
   jlower = col_starts[myid]  +base_j;
   jupper = col_starts[myid+1]+base_j - 1;
#endif

   HYPRE_Int *j_array, *j_array2;
   HYPRE_Complex *d_array, *d_array2;
   MUMPS_INT num1, num2, nnz;
   /*
      std::cout << "num rows" <<  std::to_string(num_rows) << "   " 
                           <<  std::to_string(num_rows2) <<"\n";
   */
   for (i = 0; i < num_rows; i++) {
     //if (myid == 0){std::cout << std::to_string(i) <<"\n";}
     I = first_row_index + i + base_i;          
     num1 = get_row_data(i, first_row_index, first_col_diag,
			 diag_i, diag_j, offd_i, offd_j,
			 col_map_offd,
			 diag_data, offd_data, base_i, base_j,
			 &j_array, &d_array);
     num2 = get_row_data(i, first_row_index2, first_col_diag2,
			 diag_i2, diag_j2, offd_i2, offd_j2,
			 col_map_offd2, diag_data2, offd_data2, base_i, base_j,
			 &j_array2, &d_array2);
     //nnz = nnz + union_i_len(&j_array, &j_array2, num1, num2);
     bool *flag = (bool *) malloc(sizeof(bool)*num2);
     for (k = 0; k < num2; k++){flag[k] = true;}
     for (j = 0; j < num1; j++){
       /*
       if (myid == 0){std::cout << "innz" << std::to_string(innz) << " " 
				<< std::to_string(d_array[j]) << " " 
				<< std::to_string(j)	   
	                        <<"\n";}       
       */
       set_data(&(d_array[j]), I+1, j_array[j]+1, innz);
       for (k = 0; k < num2; k++){
	   if (j_array2[k] == j_array[j]){
	     set_data_imag(&(d_array2[k]), I+1, j_array[j]+1, innz);
	       flag[k] = false;
	       break;
	     }
       }
       innz ++;
     }	 
     for (k = 0; k < num2; k++){
       if (flag[k]){
	 set_data_imag(&(d_array2[k]), I+1, j_array2[k]+1, innz);
	 innz ++;
       }
     }
     free(j_array);
     free(d_array);
     free(j_array2);
     free(d_array2);
     free(flag);
   }
   return innz;
}


MUMPS_INT MUMPS_LOC_Matrix::assemble_from_py(PyMatrix * m,
					     MUMPS_INT inz,
					     MUMPS_INT base_i,
					     MUMPS_INT base_j){
  if (DEBUG){
     std::cout << "assemble_from_py" << "\n";    
     std::cout << "base_i " << to_string(base_i) << "\n";
     std::cout << "base_j " << to_string(base_j) << "\n";
  }

  for (MUMPS_INT i = 0;i < m -> NNZ(); i++){
     if (!m-> isTrueNNZ(i)) {continue;}
     set_data(m -> get_real_data_p(i),
	      m -> get_irn(i)+base_i+1 ,
	      m -> get_jcn(i)+base_j+1,
	      inz);
     if (m -> is_complex()){
        set_data_imag(m -> get_imag_data_p(i),
		      m -> get_irn(i)+base_i+1 ,
		      m -> get_jcn(i)+base_j+1,
		      inz);
     }
     inz = inz + 1;
  }
  return inz;
}
void MUMPS_LOC_Matrix::print_data(){
  if (!isAssembled()){return;}
  for (MUMPS_INT i = 0; i < NNZ(); i++){
    std::cout << ToString(i) << "\n";
  }
}
void MUMPS_LOC_Matrix::save_data(const char *filename){
  if (!isAssembled()){return;}  
  std::ofstream file;
  file.open (filename);
  for (MUMPS_INT i = 0; i < NNZ(); i++){  
     file << ToString(i) << "\n";
  }
  file.close();
}
std::string DMUMPS_LOC_Matrix::ToString(MUMPS_INT i){
  std::string I (to_string(get_irn()[i]));
  std::string J (to_string(get_jcn()[i]));
  std::string data (to_stringf(get_data()[i]));
  I += " ";
  J += " ";
  return I + J + data;
}

std::string ZMUMPS_LOC_Matrix::ToString(MUMPS_INT i){
  std::string I (to_string(get_irn()[i]));
  std::string J (to_string(get_jcn()[i]));
  std::string re (to_stringf(get_data()[i].r));
  std::string im (to_stringf(get_data()[i].i));
  I += " ";
  J += " ";
  re += " ";
  return I + J + re + im;
}

void MUMPS_LOC_Matrix::print_info(){
  int myid;
  MPI_Comm_rank(comm, &myid);
  std::cout << "======= MUMPS_LOC_Matrix info (myid : " << to_string(myid) << ") =====\n"; 
  if (!is_assembled){
      std::cout << "tile" << "\n";
      std::cout << "shapr_ir " << to_string(shape_ir) << "\n";
      std::cout << "shapr_jc " << to_string(shape_jc) << "\n";
      std::cout << "not yet assembled \n";      
      return;
  }
  std::cout << "M : " << to_string(M()) << "\n";
  std::cout << "N : " << to_string(N()) << "\n";
  std::cout << "NNZ(local) : " << to_string(NNZ()) << "\n";    
  std::cout << "* tile" << "\n";
  std::cout << "shapr_ir " << to_string(shape_ir) << "\n";
  std::cout << "shapr_jc " << to_string(shape_jc) << "\n";

  std::cout << "* columns" << "\n";  
  for (int i = 0; i < shape_jc; i++){
    std::cout<< "  " << to_string(jc_start[i])<<"\n";
  }
  std::cout<< " ncol:" << to_string(nc)<<"\n";
  std::cout << "* rows" << "\n";       
  for (int i = 0; i < shape_ir; i++){
    std::cout<< "  " << to_string(ir_start[i])<<"\n";
  }
  std::cout<<" nrow:" << to_string(nr)<<"\n";
  std::cout << "* nnz contributions  "<< "\n";         

  int k = 0;
  PyMatrix **pymatrix = get_pymatrix();
  for (int i = 0; i < shape_ir; i++){
      for (int j = 0; j < shape_jc; j++){    
         if (mtype[k] == 0){
           std::cout << "nnz (" << to_string(k)
                     << ") > " << to_string(get_local_nnz(rmatrix[k])) << "\n";
         } else if (mtype[k] == 1){
           std::cout << "nnz (" << to_string(k) << ")"
		     << to_string(pymatrix[k]->TrueNNZ()) << "\n";
	 } else {
           std::cout << "nnz (" << to_string(k) << ")"
		     << to_string(pymatrix[k]->NNZ()) << "\n";
	 }
	 
	 k++;
      }
  }

}

MUMPS_INT ZMUMPS_LOC_Matrix::ToCSR(HYPRE_Int *row_start,
				   HYPRE_Int *row_end,
				   HYPRE_Int **ppRow,
				   HYPRE_Int **ppiCol,
				   ZMUMPS_REAL **ppRData,
				   ZMUMPS_REAL **ppIData,
				   HYPRE_Int **ppRowStarts,
				   HYPRE_Int **ppColStarts){  
  int myid, size;
  HYPRE_Int mysi, myei, si, ei;
  double msize;

  HYPRE_Int *pRow, *iCol, *pRow_tmp, *iCol_tmp, *rcount, *disp, *pRowStarts,
    *pRowEnds, *pColStarts;
  ZMUMPS_REAL *csr_rdata, *csr_idata;
  MUMPS_INT datasize = 0;      

  void * rbuf;
  void * rdata_addr;
  void * idata_addr;
  
  MPI_Comm comm = Comm();
  MUMPS_INT *irn = get_irn();
  MUMPS_INT *jcn = get_jcn();
  ZMUMPS_REAL *rdata_tmp, *idata_tmp;
  
  msize= (double)M();
  MPI_Comm_rank(comm, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  mysi = (HYPRE_Int)((float)M()/size*myid);
  myei = (HYPRE_Int)((float)M()/size*(myid+1))-1;
  pRow = (HYPRE_Int *) malloc(sizeof(HYPRE_Int)*(myei-mysi+1));
  rcount = (HYPRE_Int *)malloc(sizeof(HYPRE_Int)*size);
  disp = (HYPRE_Int *)malloc(sizeof(HYPRE_Int)*size);  

  int data_len_sum = 0;
  for (int i = 0; i<size ; i++){
    if ((myid == i) && (DEBUG)){
      std::cout << "preparing node " << to_string(i) << "\n";
      std::cout << "mysi" << to_string(mysi) << "\n";
      std::cout << "myei" << to_string(myei) << "\n";
    }
    si = (HYPRE_Int)((float)M()/size*i);
    ei = (HYPRE_Int)((float)M()/size*(i+1)-1);

    pRow_tmp = (HYPRE_Int *) malloc(sizeof(HYPRE_Int)*(ei-si));
    //count number of elements in row.. (filling row pointer)
    for (int j = si ; j <= ei; j++){pRow_tmp[j-si] = 0;}
    for (int k = 0 ; k < NNZ() ; k++){
      if ((irn[k]-1 >= si) && (irn[k]-1 <=ei)){pRow_tmp[irn[k]-1-si]++;}
    }
    //collect row pointer  to node i
    MPI_Reduce(pRow_tmp,pRow, ei-si+1, MPI_INT, MPI_SUM, i, comm);


    if (myid == i){
      //for (int j=0; j<ei-si+1; j++){
      //    std::cout << "pRow " << to_string(pRow[j]) << "\n";	
      //}
      for (int j = mysi ; j <= myei; j++){datasize  = datasize + pRow[j-mysi];}
      if (true) {std::cout << "datasize " << to_string(datasize) << "\n";}      
      iCol = (HYPRE_Int *) malloc(sizeof(HYPRE_Int)* datasize);
      csr_rdata = (ZMUMPS_REAL *) malloc(sizeof(ZMUMPS_REAL)* datasize);
      csr_idata = (ZMUMPS_REAL *) malloc(sizeof(ZMUMPS_REAL)* datasize);
      

    }
    
    for (int j = si ; j <= ei; j++){
       iCol_tmp = (HYPRE_Int *) malloc(sizeof(HYPRE_Int)*pRow_tmp[j-si]);
       rdata_tmp = (ZMUMPS_REAL *) malloc(sizeof(ZMUMPS_REAL)*pRow_tmp[j-si]);
       idata_tmp = (ZMUMPS_REAL *) malloc(sizeof(ZMUMPS_REAL)*pRow_tmp[j-si]);       
       int ii  = 0;
       for (int k = 0 ; k < NNZ() ; k++){
   	  if (irn[k]-1 == j){
   	     iCol_tmp[ii] = jcn[k]-1;
	     rdata_tmp[ii] = data[k].r;
	     idata_tmp[ii] = data[k].i;
	     ii ++;
	  }
       }
       
       MPI_Gather(&ii, 1, MPI_INT, rcount, 1, MPI_INT, i, comm);
       if (i==myid){
	 ii = 0;
	 for (int k = si; k < j; k++){ii = ii + pRow[k-si];} 
	 rbuf = (void *) &iCol[ii];
	 rdata_addr = (void *) &csr_rdata[ii];
	 idata_addr = (void *) &csr_idata[ii];
         //std::cout << "start_index " << to_string(ii) << "\n";	 
	 disp[0] = 0;
	 for (int iii = 1; iii < size; iii++){
	   disp[iii] = disp[iii-1] + rcount[iii-1];
	 }

       }
       else{
	 rbuf = NULL;
	 rdata_addr = NULL;
	 idata_addr = NULL;
	 
       }
       MPI_Gatherv(iCol_tmp, pRow_tmp[j-si], MPI_INT,
		   rbuf, rcount, disp, MPI_INT, i,  comm);
       MPI_Gatherv(rdata_tmp, pRow_tmp[j-si], MPI_DOUBLE,
		   rdata_addr,rcount, disp, MPI_DOUBLE, i, comm);
       MPI_Gatherv(idata_tmp, pRow_tmp[j-si], MPI_DOUBLE,
		   idata_addr, rcount, disp, MPI_DOUBLE, i, comm);
       if (i==myid){
         int data_len = disp[size-1] + rcount[size-1];	 
	 if (false){
            std::cout << "row " << to_string(j) << "\n";
	    for (int k=0; k<data_len; k++){
               std::cout << "col " << to_string(iCol[ii+k]) << "\n";	      
	    }
	 }
	 MUMPS_INT *sorted_idx;
	 argsort(&iCol[ii], data_len, &sorted_idx);
	 // reorder
         //for (int k=0; k < data_len; k++){
         //  std::cout << "sorted idx " << to_string(sorted_idx[k]) << "\n";
	 //}
	 for (int k=0; k < data_len; k++){
	   ZMUMPS_REAL  tmp_real, tmp_imag;
	   ZMUMPS_REAL  *data_real, *data_imag;	   
	   MUMPS_INT   tmp_idx;
	   data_real = (ZMUMPS_REAL*) rdata_addr ;
	   data_imag = (ZMUMPS_REAL*) idata_addr ;	   
	   // swap k <-> sorted_idx[k]
	   tmp_real = data_real[k];
	   tmp_imag = data_imag[k];
	   tmp_idx  = iCol[ii + k];
	   data_real[k] = data_real[sorted_idx[k]];
	   data_imag[k] = data_imag[sorted_idx[k]];
	   //std::cout << "swap" << to_string(iCol[ii + k])
	   //     << " " <<  to_string(iCol[ii + sorted_idx[k]]) << "\n";
	   iCol[ii + k]= iCol[ii + sorted_idx[k]];
	   data_real[sorted_idx[k]] = tmp_real;
	   data_imag[sorted_idx[k]] = tmp_imag;
	   iCol[ii + sorted_idx[k]] = tmp_idx;
	   for (int kkk=0; kkk < data_len; kkk++){
	     if (sorted_idx[kkk]  == k){
	       sorted_idx[kkk] = sorted_idx[k];
	       sorted_idx[k] = k;
	       break;
	     }
           }
           //std::cout << "noew sorted idx \n";
           //for (int k=0; k < data_len; k++){
           //   std::cout << "sorted idx " << to_string(sorted_idx[k]) << "\n";
	   //}
	 }
	 if (false){
	   std::cout << "Row " << to_string(j) << "\n"; 
	   for (int k=0; k < data_len; k++){
	      std::cout << "iCol " << to_string(iCol[ii + k]) << "\n";
	   }
	 }
	 free(sorted_idx);
         //std::cout << "index " << to_string(j) << " " << 
	 //  to_string(data_len) <<  "\n";	 	 
	   
	 data_len_sum  = data_len_sum + data_len;
         //std::cout << "index " << to_string(j) << " " << 
	 //  to_string(data_len) <<  "\n";	 	 
       }

       free(iCol_tmp);
       free(rdata_tmp);
       free(idata_tmp);
    }
    if (i==myid){    
      if (DEBUG){
	std::cout << "data_len_sum : node " << to_string(myid) << ":" <<
	  to_string(data_len_sum) << "\n";
      }
    }
    free(pRow_tmp);
  }
  free(rcount);
  free(disp);

  /* share offset */
  pRowStarts = (HYPRE_Int *)malloc(sizeof(HYPRE_Int) *(size+1));
  pRowEnds = (HYPRE_Int *)malloc(sizeof(HYPRE_Int) *(size+1));  
  MPI_Allgather(&mysi, 1, MPI_INT, pRowStarts, 1, MPI_INT, comm);
  MPI_Allgather(&myei, 1, MPI_INT, pRowEnds, 1, MPI_INT, comm);
  pRowStarts[size] = pRowEnds[size-1]+1;
  //if (myid == 0){
  //  for (int i=0; i < size+1; i++){
  //    std::cout << std::to_string(pRowStarts[i]) << " "
  //		<< std::to_string(pRowEnds[i]) << "\n";
  //  }
  //}
  free(pRowEnds);
  if (HYPRE_AssumedPartitionCheck()){
    pColStarts = (HYPRE_Int *)malloc(sizeof(HYPRE_Int) *(3));
    pColStarts[0] = mysi;
    pColStarts[1] = myei+1;
    pColStarts[2] = pRowEnds[size-1];    
  } else {
    pColStarts = (HYPRE_Int *)malloc(sizeof(HYPRE_Int) *(size+1));
    for (int i = 0; i < size+1; i++){pColStarts[i] = 0;}
  }
  /* cumsum pRow */
  int v = 0;
  int tmp = 0;
  for (int i = 0; i < myei-mysi+1; i++){
    tmp = pRow[i];
    pRow[i] = v;
    v = v + tmp;
  }
  pRow[myei-mysi+1] = v;
  
  /* final output */
  *row_start = mysi;
  *row_end = myei;
  *ppRow = pRow;
  *ppiCol = iCol;
  *ppRData = csr_rdata;
  *ppIData = csr_idata;
  *ppRowStarts  = pRowStarts;
  *ppColStarts  = pColStarts;  
  return datasize;
}

/* from block matrix (dcomplex) */
HYPRE_Int form_mumps_local_d_array_simple(mfem::HypreParMatrix *pmatrix,
                                  const HYPRE_Int           base_i,
				  const HYPRE_Int           base_j,
       				  MUMPS_INT **irn_p, MUMPS_INT **jcn_p,
			          DMUMPS_REAL **a_p)
{
   hypre_ParCSRMatrix *matrix =  static_cast<hypre_ParCSRMatrix *>(*pmatrix);
   HYPRE_Int nnz = get_local_nnz(pmatrix);
   HYPRE_Int innz = 0;

   MPI_Comm          comm;
   HYPRE_Int         first_row_index;
   HYPRE_Int         first_col_diag;
   hypre_CSRMatrix  *diag;
   hypre_CSRMatrix  *offd;
   HYPRE_Int        *col_map_offd;
   HYPRE_Int         num_rows;
   HYPRE_Int        *row_starts;
   HYPRE_Int        *col_starts;
   HYPRE_Complex    *diag_data;
   HYPRE_Int        *diag_i;
   HYPRE_Int        *diag_j;
   HYPRE_Complex    *offd_data;
   HYPRE_Int        *offd_i;
   HYPRE_Int        *offd_j;
   HYPRE_Int         myid, num_procs, i, j, I, J;
   HYPRE_Int         num_nonzeros_offd;
   HYPRE_Int         ilower, iupper, jlower, jupper;

   if (!matrix)
   {
      /*hypre_error_in_arg(1);*/
     return innz;
   }
   comm = hypre_ParCSRMatrixComm(matrix);
   first_row_index = hypre_ParCSRMatrixFirstRowIndex(matrix);
   first_col_diag  = hypre_ParCSRMatrixFirstColDiag(matrix);
   diag            = hypre_ParCSRMatrixDiag(matrix);
   offd            = hypre_ParCSRMatrixOffd(matrix);
   col_map_offd    = hypre_ParCSRMatrixColMapOffd(matrix);
   num_rows        = hypre_ParCSRMatrixNumRows(matrix);
   row_starts      = hypre_ParCSRMatrixRowStarts(matrix);
   col_starts      = hypre_ParCSRMatrixColStarts(matrix);
   hypre_MPI_Comm_rank(comm, &myid);
   hypre_MPI_Comm_size(comm, &num_procs);
   num_nonzeros_offd = hypre_CSRMatrixNumNonzeros(offd);

   diag_data = hypre_CSRMatrixData(diag);
   diag_i    = hypre_CSRMatrixI(diag);
   diag_j    = hypre_CSRMatrixJ(diag);
   offd_i    = hypre_CSRMatrixI(offd);
   if (num_nonzeros_offd)
   {
      offd_data = hypre_CSRMatrixData(offd);
      offd_j    = hypre_CSRMatrixJ(offd);
   }

#ifdef HYPRE_NO_GLOBAL_PARTITION
   ilower = row_starts[0]+base_i;
   iupper = row_starts[1]+base_i - 1;
   jlower = col_starts[0]+base_j;
   jupper = col_starts[1]+base_j - 1;
#else
   ilower = row_starts[myid]  +base_i;
   iupper = row_starts[myid+1]+base_i - 1;
   jlower = col_starts[myid]  +base_j;
   jupper = col_starts[myid+1]+base_j - 1;
#endif
   
   MUMPS_INT* irn = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)*nnz);
   MUMPS_INT* jcn = (MUMPS_INT *)malloc(sizeof(MUMPS_INT)*nnz);
   DMUMPS_REAL* a = (DMUMPS_REAL *)malloc(sizeof(DMUMPS_REAL)*nnz);
   *irn_p = irn;
   *jcn_p = jcn;
   *a_p   = a;

   for (i = 0; i < num_rows; i++)
   {
      I = first_row_index + i + base_i;
      for (j = diag_i[i]; j < diag_i[i+1]; j++)
      {
         J = first_col_diag + diag_j[j] + base_j;
         if ( diag_data )
	 {
           irn[innz]= (MUMPS_INT)I+1;
           jcn[innz] = (MUMPS_INT)J+1;
           a[innz] = (DMUMPS_REAL)diag_data[j];
	   innz = innz + 1;		     
         }		     
		     
      }
      if ( num_nonzeros_offd )
      {
         for (j = offd_i[i]; j < offd_i[i+1]; j++)
         {
            J = col_map_offd[offd_j[j]] + base_j;
            if ( offd_data )
            {
 	       irn[innz]= (MUMPS_INT)I+1;
               jcn[innz] = (MUMPS_INT)J+1;
               a[innz] = (DMUMPS_REAL)offd_data[j];
    	       innz = innz + 1;		     	       
     	    }
         }
      }
   }
   return innz;
}

/* get nnz on a compute node */
MUMPS_INT get_local_nnz(mfem::HypreParMatrix *pmatrix)
{
   hypre_ParCSRMatrix *matrix =  static_cast<hypre_ParCSRMatrix *>(*pmatrix);
   
   MPI_Comm          comm;
   hypre_CSRMatrix  *diag;
   hypre_CSRMatrix  *offd;
   if (!matrix)
   {
      /*hypre_error_in_arg(1);*/
     return 0;
   }
   comm = hypre_ParCSRMatrixComm(matrix);
   diag            = hypre_ParCSRMatrixDiag(matrix);
   offd            = hypre_ParCSRMatrixOffd(matrix);
   return (MUMPS_INT)hypre_CSRMatrixNumNonzeros(diag) + (MUMPS_INT)hypre_CSRMatrixNumNonzeros(offd);
}

MUMPS_INT check_nz(mfem::HypreParMatrix *pmatrix,
                 const HYPRE_Int       search_i,
                 const HYPRE_Int       search_j){
  /*
     check 
     serach_I and search_J is non zero
     if also count true_inz of search_I and search_J

  */
   hypre_ParCSRMatrix *matrix =  static_cast<hypre_ParCSRMatrix *>(*pmatrix);
   if (!matrix)
   {
      /*hypre_error_in_arg(1);*/
     return 0;
   }
   HYPRE_Int    base_i = 0;
   HYPRE_Int    base_j = 0;   
   HYPRE_Complex    *offd_data;
   HYPRE_Int        *offd_j;
   HYPRE_Int         myid, num_procs, i, j, I, J;
   HYPRE_Int         num_nonzeros_offd;
   HYPRE_Int         ilower, iupper, jlower, jupper;
   
   MPI_Comm             comm = hypre_ParCSRMatrixComm(matrix);
   hypre_MPI_Comm_rank(comm, &myid);
   hypre_MPI_Comm_size(comm, &num_procs);

   HYPRE_Int first_row_index = hypre_ParCSRMatrixFirstRowIndex(matrix);
   HYPRE_Int first_col_diag  = hypre_ParCSRMatrixFirstColDiag(matrix);
   hypre_CSRMatrix *diag     = hypre_ParCSRMatrixDiag(matrix);
   hypre_CSRMatrix *offd     = hypre_ParCSRMatrixOffd(matrix);
   HYPRE_Int *col_map_offd   = hypre_ParCSRMatrixColMapOffd(matrix);
   HYPRE_Int num_rows        = hypre_ParCSRMatrixNumRows(matrix);
   HYPRE_Int *row_starts     = hypre_ParCSRMatrixRowStarts(matrix);
   HYPRE_Int *col_starts     = hypre_ParCSRMatrixColStarts(matrix);
   num_nonzeros_offd = hypre_CSRMatrixNumNonzeros(offd);

   HYPRE_Complex *diag_data  = hypre_CSRMatrixData(diag);
   HYPRE_Int *diag_i    = hypre_CSRMatrixI(diag);
   HYPRE_Int *diag_j    = hypre_CSRMatrixJ(diag);
   HYPRE_Int *offd_i    = hypre_CSRMatrixI(offd);
   if (num_nonzeros_offd)
   {
      offd_data = hypre_CSRMatrixData(offd);
      offd_j    = hypre_CSRMatrixJ(offd);
   }

#ifdef HYPRE_NO_GLOBAL_PARTITION
   ilower = row_starts[0]+base_i;
   iupper = row_starts[1]+base_i - 1;
   jlower = col_starts[0]+base_j;
   jupper = col_starts[1]+base_j - 1;
#else
   ilower = row_starts[myid]  +base_i;
   iupper = row_starts[myid+1]+base_i - 1;
   jlower = col_starts[myid]  +base_j;
   jupper = col_starts[myid+1]+base_j - 1;
#endif
   MUMPS_INT inz = 0;
   //std::cout << "idx " << std::to_string(search_i) << "  " << std::to_string(search_j) << "\n";   
   for (i = 0; i < num_rows; i++)
   {
      I = first_row_index + i + base_i;
      for (j = diag_i[i]; j < diag_i[i+1]; j++)
      {
	 
         J = first_col_diag + diag_j[j] + base_j;
         if ( diag_data ){
	   if ((J==search_j) && (I==search_i) && (diag_data[j] != 0)){ return inz;}
	   if ((J==search_j) && (I==search_i)){ return -1;}
           if (diag_data[j] != 0) {inz = inz + 1;}
	 }
      }
      if ( num_nonzeros_offd )
      {
         for (j = offd_i[i]; j < offd_i[i+1]; j++)
         {
            J = col_map_offd[offd_j[j]] + base_j;
            if ( offd_data ) {
	      if ((J==search_j) && (I==search_i) && (offd_data[j] != 0)) {return inz;}
   	      if ((J==search_j) && (I==search_i)){ return -1;}
	      if (offd_data[j] != 0) {inz = inz + 1;}
	    }
	   
         }
      }
   }
   return -1;
}   
MUMPS_INT sum_nnz(mfem::HypreParMatrix *pmatrix,
		  mfem::HypreParMatrix *pmatrix2,
		  MUMPS_INT *index){
  
   MUMPS_INT nnz = get_true_local_nnz(pmatrix);
   
   hypre_ParCSRMatrix *matrix =  static_cast<hypre_ParCSRMatrix *>(*pmatrix);
   hypre_ParCSRMatrix *matrix2 =  static_cast<hypre_ParCSRMatrix *>(*pmatrix2);

   if (!matrix)
   {
      /*hypre_error_in_arg(1);*/
     return 0;
   }
   HYPRE_Int    base_i = 0;
   HYPRE_Int    base_j = 0;   
   HYPRE_Complex    *offd_data;
   HYPRE_Int        *offd_j;
   HYPRE_Int         myid, num_procs, i, j, I, J;
   HYPRE_Int         num_nonzeros_offd;
   HYPRE_Int         ilower, iupper, jlower, jupper;
   
   MPI_Comm             comm = hypre_ParCSRMatrixComm(matrix);
   hypre_MPI_Comm_rank(comm, &myid);
   hypre_MPI_Comm_size(comm, &num_procs);

   HYPRE_Int first_row_index = hypre_ParCSRMatrixFirstRowIndex(matrix2);
   HYPRE_Int first_col_diag  = hypre_ParCSRMatrixFirstColDiag(matrix2);
   hypre_CSRMatrix *diag     = hypre_ParCSRMatrixDiag(matrix2);
   hypre_CSRMatrix *offd     = hypre_ParCSRMatrixOffd(matrix2);
   HYPRE_Int *col_map_offd   = hypre_ParCSRMatrixColMapOffd(matrix2);
   HYPRE_Int num_rows        = hypre_ParCSRMatrixNumRows(matrix2);
   HYPRE_Int *row_starts     = hypre_ParCSRMatrixRowStarts(matrix2);
   HYPRE_Int *col_starts     = hypre_ParCSRMatrixColStarts(matrix2);
   num_nonzeros_offd = hypre_CSRMatrixNumNonzeros(offd);

   HYPRE_Complex *diag_data  = hypre_CSRMatrixData(diag);
   HYPRE_Int *diag_i    = hypre_CSRMatrixI(diag);
   HYPRE_Int *diag_j    = hypre_CSRMatrixJ(diag);
   HYPRE_Int *offd_i    = hypre_CSRMatrixI(offd);
   if (num_nonzeros_offd)
   {
      offd_data = hypre_CSRMatrixData(offd);
      offd_j    = hypre_CSRMatrixJ(offd);
   }

#ifdef HYPRE_NO_GLOBAL_PARTITION
   ilower = row_starts[0]+base_i;
   iupper = row_starts[1]+base_i - 1;
   jlower = col_starts[0]+base_j;
   jupper = col_starts[1]+base_j - 1;
#else
   ilower = row_starts[myid]  +base_i;
   iupper = row_starts[myid+1]+base_i - 1;
   jlower = col_starts[myid]  +base_j;
   jupper = col_starts[myid+1]+base_j - 1;
#endif
   

   MUMPS_INT inz = 0;
   for (i = 0; i < num_rows; i++)
   {
      I = first_row_index + i + base_i;
      for (j = diag_i[i]; j < diag_i[i+1]; j++)
      {
         J = first_col_diag + diag_j[j] + base_j;
         if ( diag_data ){
	   if (diag_data[j] != 0){
	       index[inz] = check_nz(pmatrix, I, J);
	       if (index[inz] == -1){nnz = nnz + 1;}
	       inz ++;
	   }
	 }
      }
      if ( num_nonzeros_offd )
      {
         for (j = offd_i[i]; j < offd_i[i+1]; j++)
         {
            J = col_map_offd[offd_j[j]] + base_j;
            if ( offd_data ) {
	      if (offd_data[j] != 0){
		index[inz] = check_nz(pmatrix, I, J);
		if (index[inz] == -1){nnz = nnz + 1;}	      
		inz ++;
	      }
	    }
         }
      }
   }
   // for (i = 0; i < inz; i++){
   //  std::cout << "idx " << std::to_string(index[i]) << " ";   	     
   //}
   return (MUMPS_INT)nnz;
}

MUMPS_INT get_true_local_nnz(mfem::HypreParMatrix *pmatrix)
{
   hypre_ParCSRMatrix *matrix =  static_cast<hypre_ParCSRMatrix *>(*pmatrix);
   const HYPRE_Int           base_i = 0;
   const HYPRE_Int           base_j = 0;
   
   MPI_Comm          comm;
   HYPRE_Int         first_row_index;
   HYPRE_Int         first_col_diag;
   hypre_CSRMatrix  *diag;
   hypre_CSRMatrix  *offd;
   HYPRE_Int        *col_map_offd;
   HYPRE_Int         num_rows;
   HYPRE_Int        *row_starts;
   HYPRE_Int        *col_starts;
   HYPRE_Complex    *diag_data;
   HYPRE_Int        *diag_i;
   HYPRE_Int        *diag_j;
   HYPRE_Complex    *offd_data;
   HYPRE_Int        *offd_i;
   HYPRE_Int        *offd_j;
   HYPRE_Int         myid, num_procs, i, j, I, J;
   HYPRE_Int         num_nonzeros_offd;
   HYPRE_Int         ilower, iupper, jlower, jupper;

   if (!matrix)
   {
      /*hypre_error_in_arg(1);*/
     return 0;
   }
   comm = hypre_ParCSRMatrixComm(matrix);
   first_row_index = hypre_ParCSRMatrixFirstRowIndex(matrix);
   first_col_diag  = hypre_ParCSRMatrixFirstColDiag(matrix);
   diag            = hypre_ParCSRMatrixDiag(matrix);
   offd            = hypre_ParCSRMatrixOffd(matrix);
   col_map_offd    = hypre_ParCSRMatrixColMapOffd(matrix);
   num_rows        = hypre_ParCSRMatrixNumRows(matrix);
   row_starts      = hypre_ParCSRMatrixRowStarts(matrix);
   col_starts      = hypre_ParCSRMatrixColStarts(matrix);
   hypre_MPI_Comm_rank(comm, &myid);
   hypre_MPI_Comm_size(comm, &num_procs);
   num_nonzeros_offd = hypre_CSRMatrixNumNonzeros(offd);

   diag_data = hypre_CSRMatrixData(diag);
   diag_i    = hypre_CSRMatrixI(diag);
   diag_j    = hypre_CSRMatrixJ(diag);
   offd_i    = hypre_CSRMatrixI(offd);
   if (num_nonzeros_offd)
   {
      offd_data = hypre_CSRMatrixData(offd);
      offd_j    = hypre_CSRMatrixJ(offd);
   }

#ifdef HYPRE_NO_GLOBAL_PARTITION
   ilower = row_starts[0]+base_i;
   iupper = row_starts[1]+base_i - 1;
   jlower = col_starts[0]+base_j;
   jupper = col_starts[1]+base_j - 1;
#else
   ilower = row_starts[myid]  +base_i;
   iupper = row_starts[myid+1]+base_i - 1;
   jlower = col_starts[myid]  +base_j;
   jupper = col_starts[myid+1]+base_j - 1;
#endif
   MUMPS_INT nnz = 0;

   for (i = 0; i < num_rows; i++)
   {
      I = first_row_index + i + base_i;
      for (j = diag_i[i]; j < diag_i[i+1]; j++)
      {
         J = first_col_diag + diag_j[j] + base_j;
         if ( diag_data ){if (diag_data[j] != 0){nnz = nnz + 1;}}
      }
      if ( num_nonzeros_offd )
      {
         for (j = offd_i[i]; j < offd_i[i+1]; j++)
         {
            J = col_map_offd[offd_j[j]] + base_j;
            if ( offd_data ) {if (offd_data[j] != 0){nnz = nnz + 1;}}
         }
      }
   }
   return nnz;
}

MUMPS_INT get_HypreParMatrixRow(mfem::HypreParMatrix * rmatrix,
			    MUMPS_INT i){
   hypre_ParCSRMatrix *matrix =  static_cast<hypre_ParCSRMatrix *>(*rmatrix);  
   hypre_CSRMatrix *diag     = hypre_ParCSRMatrixDiag(matrix);
   HYPRE_Int *diag_i    = hypre_CSRMatrixI(diag);
   HYPRE_Int first_row_index = hypre_ParCSRMatrixFirstRowIndex(matrix);
   return (MUMPS_INT)(i + first_row_index);
}

void argsort(MUMPS_INT *ptr, MUMPS_INT len, MUMPS_INT **pRes){
  MUMPS_INT tmp;
  MUMPS_INT idx = 0;
  MUMPS_INT idx0 = -1;
  
  *pRes = (MUMPS_INT *) malloc(sizeof(MUMPS_INT)*len);
  MUMPS_INT *res = *pRes;
  
  tmp = -1;
  for (int j = 0; j<len; j++){
    if (j > 0) {
      idx0 = ptr[res[j-1]];
      tmp = -1;
    }
    for (int i = 0; i<len; i++){
      if (tmp == -1){
	if (ptr[i] > idx0){
	  idx = i;
	  tmp = ptr[i]; 
        }
      } else if ((ptr[i]>idx0) && (ptr[i] < tmp)){
          idx = i;
          tmp = ptr[i];
      }
    }
    res[j] = idx;
  }
  //  if (false){
  //     std::cout << "sort idx \n";
  //     for (int i = 0; i<len; i++){
  //       std::cout << std::to_string(res[i]) << ":" <<  std::to_string(ptr[res[i]]) << "\n ";   	     
  //     }
  //  }
}

void print_HYPRE_matrix_info(mfem::HypreParMatrix *pmatrix)
{
   hypre_ParCSRMatrix *matrix =  static_cast<hypre_ParCSRMatrix *>(*pmatrix);
   
   HYPRE_Int    base_i = 0;
   HYPRE_Int    base_j = 0;   
   HYPRE_Complex    *offd_data;
   HYPRE_Int        *offd_j;
   HYPRE_Int         myid, num_procs, i, j, I, J;
   HYPRE_Int         num_nonzeros_offd;
   HYPRE_Int         ilower, iupper, jlower, jupper;
   
   MPI_Comm             comm = hypre_ParCSRMatrixComm(matrix);
   hypre_MPI_Comm_rank(comm, &myid);
   hypre_MPI_Comm_size(comm, &num_procs);

   HYPRE_Int first_row_index = hypre_ParCSRMatrixFirstRowIndex(matrix);
   HYPRE_Int first_col_diag  = hypre_ParCSRMatrixFirstColDiag(matrix);
   hypre_CSRMatrix *diag     = hypre_ParCSRMatrixDiag(matrix);
   hypre_CSRMatrix *offd     = hypre_ParCSRMatrixOffd(matrix);
   HYPRE_Int *col_map_offd   = hypre_ParCSRMatrixColMapOffd(matrix);
   HYPRE_Int num_rows        = hypre_ParCSRMatrixNumRows(matrix);
   HYPRE_Int *row_starts     = hypre_ParCSRMatrixRowStarts(matrix);
   HYPRE_Int *col_starts     = hypre_ParCSRMatrixColStarts(matrix);

   num_nonzeros_offd = hypre_CSRMatrixNumNonzeros(offd);

   HYPRE_Complex *diag_data  = hypre_CSRMatrixData(diag);
   HYPRE_Int *diag_i    = hypre_CSRMatrixI(diag);
   HYPRE_Int *diag_j    = hypre_CSRMatrixJ(diag);
   HYPRE_Int *offd_i    = hypre_CSRMatrixI(offd);
   if (num_nonzeros_offd)
   {
      offd_data = hypre_CSRMatrixData(offd);
      offd_j    = hypre_CSRMatrixJ(offd);
   }

#ifdef HYPRE_NO_GLOBAL_PARTITION
   ilower = row_starts[0]+base_i;
   iupper = row_starts[1]+base_i - 1;
   jlower = col_starts[0]+base_j;
   jupper = col_starts[1]+base_j - 1;
#else
   ilower = row_starts[myid]  +base_i;
   iupper = row_starts[myid+1]+base_i - 1;
   jlower = col_starts[myid]  +base_j;
   jupper = col_starts[myid+1]+base_j - 1;
#endif
  
  std::cout << "first row index " << to_string(first_row_index) << "\n";
  std::cout << "first col diag " << to_string(first_col_diag) << "\n";  
  std::cout << "num rows " << to_string(num_rows) << "\n";                
  std::cout << "nnz " << to_string(get_local_nnz(pmatrix)) << "\n";
  std::cout << "ilower " << to_string(ilower) << "\n";
  std::cout << "iupper " << to_string(iupper) << "\n";
  std::cout << "jlower " << to_string(jlower) << "\n";
  std::cout << "jupper " << to_string(jupper) << "\n";
}

// Helper string functions. Will go away in C++11
std::string to_string(int i)
{
  std::stringstream ss;
  ss << i;

   // trim leading spaces
  std::string out_str = ss.str();
  out_str = out_str.substr(out_str.find_first_not_of(" \t"));
  return out_str;
}
std::string to_stringf(double f)
{
  std::stringstream ss;
  ss.precision(5);
  ss << f;

   // trim leading spaces
  std::string out_str = ss.str();
  out_str = out_str.substr(out_str.find_first_not_of(" \t"));
  return out_str;
}


