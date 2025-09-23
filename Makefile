##   Makefile
##
##   default variable setting
##
MAKE=$(shell which make)
PYTHON ?= $(shell which python)
PYTHONCONFIG ?= $(shell which python3-config)
CC ?= $(shell which gcc)
UNAME := $(shell uname)

PREFIX=$(shell echo $(PetraM))
ifeq ($(PREFIX),)
   PREFIX := /usr/local/PetraM
endif

### compilers
MPILIB ?= mpi
MPICC ?= mpicc
MPICXX ?= mpicxx
MPIFC ?= mpifort
MPIFL ?= mpifort

#
#  these are to absorb the difference between linux and macOS
#
# no-compact-unwind (this option is not used anymore...)
NOCOMPACTUNWIND ?=
ifeq ($(UNAME), Darwin)
   NOCOMPACTUNWIND = -Wl,-no_compact_unwind
endif

MPI_INC_DIR ?= $(PREFIX)/include
MUMPS_SOLVE_DIR ?= $(PREFIX)/lib
MUMPS_INC_DIR   ?= $(PREFIX)/include
MKL ?=
CC=${MPICC}
CXX=${MPICXX}

export 

default: so
.PHONEY:all install

##
cxx: 
	$(MAKE) -C petram/ext cxx
cleancxx: 
	$(MAKE) -C petram/ext cleancxx
so:
	$(PYTHON) setup.py build

install:
	mkdir -p $(PREFIX)
	for dir in $(PREFIX); do mkdir -p $(PREFIX)/$$dir; done
	$(PYTHON) setup.py install --prefix=$(PREFIX)
#
#install:
#	mkdir -p $(PREFIX)
#	for dir in $(PREFIX); do mkdir -p $(PREFIX)/$$dir; done
#       $(PYTHON) -m pip install ./ --prefix=$(PREFIX) --no-build-isolation
#
clean:
	$(MAKE) -C petram/ext clean
	rm -rf build/*

