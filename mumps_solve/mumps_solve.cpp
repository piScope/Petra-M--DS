/*
 * mumps_solve.cpp
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"
#include "dmumps_c.h"
#include "zmumps_c.h"
#include "smumps_c.h"
#include "cmumps_c.h"
#include "mumps_solve.hpp"
#include <iostream>

#define ICNTL(I) icntl[(I)-1] /* macro s.t. indices match documentation */

namespace libmumps_solve
{

//  dmumps
DMUMPS::DMUMPS(int par, int sym, MPI_Comm comm_c)
{
  id = (DMUMPS_STRUC_C *) malloc(sizeof(DMUMPS_STRUC_C));
  id->job=JOB_INIT;
  id->par=par;
  id->sym=sym;
  if (comm_c == MPI_COMM_NULL){
    id->comm_fortran = USE_COMM_WORLD;
  } else {
    id->comm_fortran = (MUMPS_INT) MPI_Comm_c2f(comm_c);
  }
  dmumps_c(id);
  initialized = true;

}
DMUMPS::~DMUMPS()
{
  if (initialized){
    finish();
  }
  free(id);
}
void DMUMPS::run()
{
  dmumps_c(id);
}
void DMUMPS::finish()
{
  id->job=JOB_END;
  dmumps_c(id); /* Terminate instance */
  initialized = false;  
}

//  zmumps
ZMUMPS::ZMUMPS(int par, int sym, MPI_Comm comm_c)
{
  id = (ZMUMPS_STRUC_C *) malloc(sizeof(ZMUMPS_STRUC_C));
  id->job=JOB_INIT;
  id->par=par;
  id->sym=sym;
  if (comm_c == MPI_COMM_NULL){
    id->comm_fortran = USE_COMM_WORLD;
  } else {
    id->comm_fortran = (MUMPS_INT) MPI_Comm_c2f(comm_c);
  }
  zmumps_c(id);
  initialized = true;
}
ZMUMPS::~ZMUMPS()
{
  if (initialized){
    finish();
  }
  free(id);
}
void ZMUMPS::run()
{
  zmumps_c(id);
}
void ZMUMPS::finish()
{
  id->job=JOB_END;
  zmumps_c(id); /* Terminate instance */
  initialized = false;  
}

//  smumps
SMUMPS::SMUMPS(int par, int sym, MPI_Comm comm_c)
{
  id = (SMUMPS_STRUC_C *) malloc(sizeof(SMUMPS_STRUC_C));
  id->job=JOB_INIT;
  id->par=par;
  id->sym=sym;
  if (comm_c == MPI_COMM_NULL){
    id->comm_fortran = USE_COMM_WORLD;
  } else {
    id->comm_fortran = (MUMPS_INT) MPI_Comm_c2f(comm_c);
  }
  smumps_c(id);
  initialized = true;
}
SMUMPS::~SMUMPS()
{
  if (initialized){
    finish();
  }
  free(id);
}
void SMUMPS::run()
{
  smumps_c(id);
}
void SMUMPS::finish()
{
  id->job=JOB_END;
  smumps_c(id); /* Terminate instance */
  initialized = false;  
}

//  cmumps 
CMUMPS::CMUMPS(int par, int sym, MPI_Comm comm_c)
{
  id = (CMUMPS_STRUC_C *) malloc(sizeof(CMUMPS_STRUC_C));
  id->job=JOB_INIT;
  id->par=par;
  id->sym=sym;
  if (comm_c == MPI_COMM_NULL){
    id->comm_fortran = USE_COMM_WORLD;
  } else {
    id->comm_fortran = (MUMPS_INT) MPI_Comm_c2f(comm_c);
  }
  cmumps_c(id);
  initialized = true;
}
CMUMPS::~CMUMPS()
{
  if (initialized){
    finish();
  }
  free(id);
}
void CMUMPS::run()
{
  cmumps_c(id);
}
void CMUMPS::finish()
{
  id->job=JOB_END;
  cmumps_c(id); /* Terminate instance */
  initialized = false;  
}
}  // end of namespace

/*
  distributed sparse left hand side matrix
  dense right hand side matrix
*/
int libmumps_solve_example_dist(MPI_Comm comm)
{
  libmumps_solve::DMUMPS s = libmumps_solve::DMUMPS(1, 0, comm);  

  MUMPS_INT ierr;
  int       num_procs, myid;
  
  ierr = MPI_Comm_size(comm, &num_procs);
  ierr = MPI_Comm_rank(comm, &myid);  

  MUMPS_INT n = 2*num_procs;     // size of matrix
  MUMPS_INT nz = 2*num_procs;    // non zero elements
  MUMPS_INT irn[] = {2*myid+1,2*myid+2};
  MUMPS_INT jcn[] = {2*myid+1,2*myid+2};
  double a[] = {2*myid+1.,2*myid+2.};
  
  /* Define rhs as all one vector */  
  double *rhs  = (double*) malloc(sizeof(double)* 2*num_procs);
  for( int i = 0; i < n; i = i + 1 )
  {
    rhs[i] = 1;
  }  


  /* Initialize a MUMPS instance. Use MPI_COMM_WORLD */
  //std::cout << "here\n";
  //std::cout << "here\n"; 
  DMUMPS_STRUC_C *id = (s.get_struct());
  
  id->ICNTL(5)=0;  // assembled format
  id->ICNTL(18)=3; // distributed pattern and entry
  /* Define the problem on the host */
  if (myid == 0) {
      id->n = n;
      id->nz = nz;
      id->rhs = rhs;
  }
  /* Setting for all nodes */
  id->nz_loc = 2;   // valid when par = 1
  id->irn_loc = irn;
  id->jcn_loc = jcn;
  id->a_loc  = a;

/* No outputs */
  id->ICNTL(1)=-1; id->ICNTL(2)=-1; id->ICNTL(3)=-1; id->ICNTL(4)=0; 
/* Call the MUMPS package. */
  id->job=6;
  //std::cout << "here" << std::to_string((s->get_struct())->job)<<"\n";
  s.run();
  //std::cout << "here\n";
  s.finish();  
  if (myid == 0) {
    for( int i = 0; i < n; i = i + 1 )
    {
      printf("Solution is : (%d %8.4f)\n", i, rhs[i]);
    }   
  }
  return 0;
}

/*  
  This example is based on c_examplie in  MUMPS 5.0.1, released
  modified to test coupling with SWIG
  
  Example program using the C interface to the 
  double real arithmetic version of MUMPS, dmumps_c.
  We solve the system A x = RHS with
    A = diag(1 2) and RHS = [1 4]^T
  Solution is [1 2]^T
*/

int libmumps_solve_example(MPI_Comm comm)
{
  libmumps_solve::DMUMPS s = libmumps_solve::DMUMPS(1, 0, comm);  

  MUMPS_INT n = 2;
  MUMPS_INT nz = 2;
  MUMPS_INT irn[] = {1,2};
  MUMPS_INT jcn[] = {1,2};
  double a[2];
  double rhs[2];

  int myid, ierr;
  
  ierr = MPI_Comm_rank(comm, &myid);

  /* Define A and rhs */
  rhs[0]=1.0;rhs[1]=4.0;
  a[0]=1.0;a[1]=2.0;

  /* Initialize a MUMPS instance. Use MPI_COMM_WORLD */
  //std::cout << "here\n";
  //std::cout << "here\n"; 
  DMUMPS_STRUC_C *id = (s.get_struct());
  /* Define the problem on the host */
  if (myid == 0) {
    id->n = n; id->nz =nz; id->irn=irn; id->jcn=jcn;
    id->a = a; id->rhs = rhs;
  }

/* No outputs */
  id->ICNTL(1)=-1; id->ICNTL(2)=-1; id->ICNTL(3)=-1; id->ICNTL(4)=0;
/* Call the MUMPS package. */
  id->job=6;
  //std::cout << "here" << std::to_string((s->get_struct())->job)<<"\n";
  s.run();
  //std::cout << "here\n";
  s.finish();  
  if (myid == 0) {
    printf("Solution is : (%8.2f  %8.2f)\n", rhs[0], rhs[1]);	   
  }
  return 0;
}


