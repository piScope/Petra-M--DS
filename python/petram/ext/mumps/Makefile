#
#  
#
SWIG ?= $(shell which swig)
SWIGFLAG = -Wall -c++ -python

IFILE = mumps_solve.i
ALLCXX = $(IFILE:.i=._wrap.cxx)

MPI4PYINCDIR = $(shell $(PYTHON) -c "import mpi4py;print(mpi4py.get_include())")
MPI4PYINC  = -I$(MPI4PYINCDIR)
MUMPSINC   = -I$(MUMPS_INC_DIR)
.PHONEY:allso clean

default: 
all: 

cxx: $(ALLCXX)
%._wrap.cxx: %.i
	$(SWIG) $(SWIGFLAG) -I../../../mumps_solve $(MPI4PYINC) $(MUMPSINC) $<
cleancxx:
	rm *.cxx
clean:
	rm -f *.o  *.a *.so
