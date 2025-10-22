print('building mumps solver interface')

## 
setup_dir = os.path.dirname(os.path.abspath(os.path.realpath(__file__)))
mumps_solve_incdir = os.path.join(setup_dir, "mumps_solve")
mumps_solve_dir = ''

from distutils.core import *
from distutils      import sysconfig


modules= ["mumps_solve", ]

module_path="petram.ext.mumps."
sdir = "petram/ext/mumps/"
sources = {name: [sdir + name + "_wrap.cxx"] for name in modules}

proxy_names = {name: '_'+name for name in modules}

#mumps_link_args =  [libporda, mumpscommonliba, dmumpsliba, smumpsliba, cmumpsliba,
#                    zmumpsliba]


#include_dirs = [mumps_solve_incdir, mpichincdir, numpyincdir,
#                mpi4pyincdir, mumpsincdir, mumpssrcdir]
import numpy
numpyincdir = numpy.get_include()

import mpi4py
mpi4pyincdir = mpi4py.get_include()

mumps_inc_dir = os.getenv("MUMPS_INC_DIR")
mpi_inc_dir = os.getenv("MPI_INC_DIR")

include_dirs = [mumps_solve_incdir, numpyincdir, mpi4pyincdir,
                mumps_inc_dir, mpi_inc_dir]

include_dirs = [x for x in include_dirs if x.strip() != '']

#lib_list = ["pord", "parmetis", "metis5", "scalapack",  "blas"]
lib_list = []
library_dirs = [os.getenv("MUMPS_SOLVE_DIR")]
#libraries = ["smumps", "dmumps", "cmumps", "zmumps", "mumps_common"]
libraries = ["mumps_solve"]
for lib in lib_list:
    if eval(lib) != "":
        print(lib, eval(lib))
        library_dirs.append(eval(lib+ 'lnkdir'))
        libraries.append(eval(lib+'lib'))
        
mkl = os.getenv("MKL")
ompflag = os.getenv("OMPFLAG")

print("ompflag", ompflag)
ext_modules = []
for kk, name in enumerate(modules):
   extra_link_args = [ompflag]

   '''
   if kk == 0:
       extra_link_args = mumps_link_args + [sdir + name+'.a']
   else:
       extra_link_args = [sdir + name+'.a']

   if whole_archive != '':
       extra_link_args = ['-Wl', whole_archive] +  extra_link_args + [no_whole_archive]
       extra_link_args =  [x for x in extra_link_args if len(x) != 0]   
       extra_link_args =  [','.join(extra_text)]
   '''
   if mkl != '':
       extra_link_args =  ['-shared-intel', mkl] + extra_link_args
   #if nocompactunwind != '':        
   #    extra_link_args.extend([nocompactunwind])
   #extra_link_args =  ['-fopenmp']+[x for x in extra_link_args if len(x) != 0]
   #extra_link_args =  [x for x in extra_link_args if len(x) != 0]   


   ext_modules.append(Extension(module_path+proxy_names[name],
                        sources=sources[name],
                        extra_compile_args = ['-DSWIG_TYPE_TABLE=PyMFEM'],   
                        extra_link_args = extra_link_args,
                        include_dirs = include_dirs,
                        library_dirs = library_dirs,
                        libraries = libraries ))
