import os
import multiprocessing

from build_utils import *

__all__ = ("clone_build_mumps",)

def clone_mumps():
    gitclone('mumps', use_sha=True)


def cmake_mumps(bglb):
    path = os.path.join(bglb.extdir, 'mumps')
    root = chdir(path)

    try:
        # we run something like
        # cmake -Bcmbuild -DMUMPS_parallel=True -DMUMPS_scotch=yes -DCMAKE_VERBOSE_MAKEFILE=yes -DMUMPS_find_scotch=yes

        cmake_opts = {'DMUMPS_parallel': 'Yes', }

        if bglb.verbose:
            cmake_opts['DCMAKE_VERBOSE_MAKEFILE'] = 'Yes'

        cmake_opts['DCMAKE_fortran_COMPILER'] = bglb.fc            
        cmake_opts['DMPI_fortran_COMPILER'] = bglb.mpifc
        
        if bglb.mumps_scotch:
            cmake_opts['DMUMPS_scotch'] = 'Yes'
        if bglb.mumps_ptscotch:
            cmake_opts['DMUMPS_ptscotch'] = 'Yes'
        if bglb.mumps_scotch or bglb.mumps_ptscotch:
            cmake_opts['DMUMPS_find_scotch'] = 'Yes'

        if bglb.mumps_metis:
            cmake_opts['DMUMPS_metis'] = 'Yes'
        if bglb.mumps_parmetis:
            cmake_opts['DMUMPS_parmetis'] = 'Yes'
        if bglb.mumps_metis or bglb.mumps_parmetis:
            cmake_opts['DMUMPS_find_metis'] = 'Yes'

        if bglb.smumps:
            cmake_opts['DBUILD_SINGLE'] = 'Yes'
        if bglb.dmumps:
            cmake_opts['DBUILD_DOUBLE'] = 'Yes'
        if bglb.cmumps:
            cmake_opts['DBUILD_COMPLEX'] = 'Yes'
        if bglb.zmumps:
            cmake_opts['DBUILD_COMPLEX16'] = 'Yes'

        if bglb.mumps_int64:
            cmake_opts['DMUMPS_intsize64'] = 'Yes'
        if bglb.mumps_openmp:
            cmake_opts['DMUMPS_openmp'] = 'Yes'
        else:
            cmake_opts['DMUMPS_openmp'] = 'No'

        cmake('-Bcmbuild', **cmake_opts)

    except Exception:
        import traceback
        traceback.print_exc()

    chdir(root)


def build_mumps(bglb):
    #
    #  will call cmake --build cmbuild
    #
    path = os.path.join(bglb.extdir, 'mumps')
    root = chdir(path)

    num_jobs = max(multiprocessing.cpu_count() - 1, 1)
    num_jobs = min(num_jobs, 7)               

    success, stdout, stderr = cmake('--build', 'cmbuild', '-j', str(num_jobs))

    if not success:
        assert False, stderr
    chdir(root)

def clone_build_mumps(bglb):
    clone_mumps()
    cmake_mumps(bglb)
    build_mumps(bglb)
    cmake_mumps(bglb)
    build_mumps(bglb)
