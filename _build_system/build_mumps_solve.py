import os
import shutil

from build_utils import cmake, chdir, make_call

__all__ = ("cmake_mumps_solve",
           "generate_mumps_solve_wrapper",)

def cmake_mumps_solve(bglb):
    path = os.path.join(bglb.rootdir, 'mumps_solve')
    pwd = chdir(path)

    path2 = os.path.join(bglb.rootdir, 'mumps_solve', 'cmbuild')
    if os.path.exists(path2):
        shutil.rmtree(path2)

    try:
        cmake_opts = {}
        cmake_opts['DCMAKE_VERBOSE_MAKEFILE'] = 'Yes'
        cmake_opts['DCMAKE_INSTALL_PREFIX'] = os.path.join(bglb.bdist_wheel_prefix, "petram", "external")
        cmake('-Bcmbuild', **cmake_opts)
        cmake('--build', 'cmbuild')
        cmake('--install', 'cmbuild')
    except Exception:
        chdir(pwd)
        raise
    chdir(pwd)


def generate_mumps_solve_wrapper(bglb):

    if bglb.dry_run or bglb.verbose:
        print("generating SWIG wrapper")


    swigflag = '-Wall -c++ -python -fastproxy -olddefs -keyword'.split(' ')

    pwd = chdir(os.path.join(bglb.rootdir, 'python', 'petram', 'ext', 'mumps'))

    import mpi4py

    mpi4pyinc = "-I" + mpi4py.get_include()
    mumpsinc = "-I" + os.path.join(bglb.rootdir, "external", "mumps", "cmbuild",  "local", "include")
    mumpssolveinc = "-I" + os.path.join(bglb.rootdir, "mumps_solve")

    command = [bglb.swig]+ swigflag + [mpi4pyinc, mumpsinc, mumpssolveinc] + ["mumps_solve.i"]

    make_call(command)

    chdir(pwd)
