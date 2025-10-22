import sys
import configparser
from urllib import request
import itertools
import site
import re
import subprocess
import multiprocessing
import ssl
import tarfile
import shutil
from collections import namedtuple
from shutil import which as find_command

__all__ = ["print_config",
           "initialize_cmd_options",
           "cmd_options",
           "process_cmd_options",
           "configure_build",
           "clean_dist_info",
           ]

def print_config():
    print("----configuration----")
    print(" prefix", bglb.prefix)
    print(" build mumps : " + ("Yes" if bglb.build_mumps else "No"))
    print(" c compiler : " + bglb.cc_command)
    print(" c++ compiler : " + bglb.cxx_command)
    print(" mpi-c compiler : " + bglb.mpicc_command)
    print(" mpi-c++ compiler : " + bglb.mpicxx_command)

    print(" verbose : " + ("Yes" if bglb.verbose else "No"))
    print(" SWIG : " + bglb.swig_command)

    if bglb.blas_libraries != "":
        print(" BLAS libraries : " + bglb.blas_libraries)
    if bglb.lapack_libraries != "":
        print(" Lapack libraries : " + bglb.lapack_libraries)

    print("")

