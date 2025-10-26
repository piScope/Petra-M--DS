import os
import multiprocessing
import shutil

from build_utils import chdir, gitclone, cmake

__all__ = ("clone_build_mumps",)

def _clone_mumps(bglb):
    path = os.path.join(bglb.extdir)
    root = chdir(path)

    try:
        gitclone('mumps', use_sha=True)
        path2 = os.path.join(bglb.extdir, 'mumps', 'cmbuild')
        if os.path.exists(path2):
             shutil.rmtree(path2)
    except BaseException:
        chdir(root)
        raise

    chdir(root)

def _cmake_mumps(bglb):
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


def _build_mumps(bglb):
    #
    #  will call cmake --build cmbuild
    #
    path = os.path.join(bglb.extdir, 'mumps')
    root = chdir(path)

    num_jobs = max(multiprocessing.cpu_count() - 1, 1)
    num_jobs = min(num_jobs, 7)

    success, stdout, stderr = cmake('--build', 'cmbuild', '-j', str(num_jobs))

    if not success:
        chdir(root)
        assert False, stderr
    chdir(root)

def _install_mumps(bglb):
    path = os.path.join(bglb.extdir, 'mumps')
    root = chdir(path)

    try:
        cmake('--install', 'cmbuild')
    except BaseException:
        chdir(root)
        raise

    chdir(root)

def clone_build_mumps(bglb):
    _clone_mumps(bglb)
    _cmake_mumps(bglb)
    _build_mumps(bglb)
    _cmake_mumps(bglb)
    _build_mumps(bglb)
    _install_mumps(bglb)
