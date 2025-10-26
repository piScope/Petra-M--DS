import sys
import os
import shutil
from collections import namedtuple


__all__ = ["print_config",
           "initialize_cmd_opts",
           "configure_build",
           "clean_dist_info",
           "clean_wrapper",
           ]

from shutil import which as find_command
swig_command = find_command('swig')
cc_command = find_command('cc')
cxx_command = find_command('c++')
fc_command = find_command('gfortran')
mpicc_command = find_command('mpicc')
mpicxx_command = find_command('mpic++')
mpifort_command = find_command('mpifort')

cmd_opts = [
    ('MUMPS-PTSCOTCH=', 'YES', 'build mumps with PTScotch'),
    ('MUMPS-SCOTCH=', 'YES', 'build mumps with Scotch'),
    ('MUMPS-METIS=', 'YES', 'build mumps with METIS'),
    ('MUMPS-ParMETIS=', 'YES', 'build mumps with ParMETIS'),
    ('MUMPS-OpenMP=', 'YES', 'use OpenMP in MUMPS'),
    ('MUMPS-int64=', 'No', 'use OpenMP in MUMPS'),
    ('SMUMPS=', 'YES', 'build single version'),
    ('DMUMPS=', 'YES', 'build double version'),
    ('CMUMPS=', 'YES', 'build complex version'),
    ('ZMUMPS=', 'YES', 'build double complex version'),
    ('no-mumps', False, 'Do not build mumps'),
    ('ext-only', False, 'Build external libaries (MUMPS etc) only'),
    ('swig-only', False, 'Run swig wrapper only (ext-only is required before this option)'),
    ('skip-ext', False, 'Skip building external libaries (MUMPS etc)'),
    ('MPINC=', '', 'directory of mpi.h (typically mpi compiler wrapper will find it automatically)'),
    ('CC=', cc_command, 'c compiler'),
    ('CXX=', cxx_command, 'c++ compiler'),
    ('FC=', fc_command, 'fortran compiler'),
    ('MPICC=', mpicc_command, 'mpic compiler'),
    ('MPICXX=', mpicxx_command, 'mpic++ compiler'),
    ('MPIFC=', mpifort_command, 'mpi fortran compiler'),
    ('SWIG=', swig_command, 'swig wrapper generator'),
    ('GIT-SSH', False, 'use ssh to clone repository'),
]


def print_config(bglb):
    print("----configuration----")
    print(" prefix", bglb.prefix)
    print(" build mumps : " + ("Yes" if bglb.build_mumps else "No"))
    print(" c compiler : " + bglb.cc)
    print(" c++ compiler : " + bglb.cxx)
    print(" fc compiler : " + bglb.fc)
    print(" mpi-c compiler : " + bglb.mpicc)
    print(" mpi-c++ compiler : " + bglb.mpicxx)
    print(" mpi-fort compiler : " + bglb.mpifc)

    print(" verbose : " + ("Yes" if bglb.verbose else "No"))
    print(" SWIG : " + bglb.swig)

    print("")


def clean_dist_info(wheeldir):
    if not os.path.isdir(wheeldir):
        return
    for x in os.listdir(wheeldir):
        if x.endswith(".dist-info"):
            fname = os.path.join(wheeldir, x)
            print("!!! removing existing ", fname)
            shutil.rmtree(fname)


def clean_wrapper():
    print("!!!!!!!!! not implemented  !!!!!!!!!!!!!!!!!!! (clean wrapper)")
    pass


def initialize_cmd_opts(bglb):

    for param, value, _help in cmd_opts:
        if param.endswith('='):
            param = param[:-1]

        attr = '_'.join(param.split('-'))
        value = value if os.getenv(attr) is None else os.getenv(attr)

        setattr(bglb, attr.lower(), value)


def _process_cmd_opts(bglb, cfs):
    '''
    called when install workflow is used
    '''
    for param, _none, hit in cmd_opts:
        attr = ("_".join(param.split("-"))).lower()

        if param.endswith("="):
            param = param[:-1]
            attr = attr[:-1]
            value = cfs.pop(param, "")
            if value != "":
                if not hasattr(bglb, attr):
                    assert False, str(bglb) + " does not have " + attr
                setattr(bglb, attr, value)
        else:
            value = cfs.pop(param, "No")
            if not hasattr(bglb, attr):
                assert False, str(bglb) + " does not have " + attr

            if value.upper() in ("YES", "TRUE", "1"):
                setattr(bglb, attr, True)
            else:
                setattr(bglb, attr, False)


def _process_setup_opts(bglb, args):
    for item in args:
        if item.startswith('--'):
            item = item[2:]
        if item.startswith('-'):
            item = item[1:]

        if len(item.split('=')) == 2:
            param = item.split('=')[0]
            value = item.split('=')[1]
        else:
            param = item.strip()
            value = True
        attr = "_".join(param.split("-"))

        setattr(bglb, attr, value)

'''
def _external_install_prefix(prefix, verbose=True):
    import site
    if hasattr(site, "getusersitepackages"):
        usersite = site.getusersitepackages()
    else:
        usersite = site.USER_SITE

    if verbose:
        print("running external_install_prefix with the following parameters")
        print("   sys.argv :", sys.argv)
        print("   sys.prefix :", sys.prefix)
        print("   usersite :", usersite)
        print("   prefix :", prefix)

    if '--user' in sys.argv:
        path = usersite
        if not os.path.exists(path):
            os.makedirs(path)
        path = os.path.join(path, 'petram', 'external')
        return path

    else:
        # when prefix is given...let's borrow pip._internal to find the location ;D
        import pip._internal.locations
        path = pip._internal.locations.get_scheme(
            "petram", prefix=prefix).platlib
        if not os.path.exists(path):
            os.makedirs(path)
        path = os.path.join(path, 'petram', 'external')
        return path
'''

def configure_build(bglb):
    '''
    called when install workflow is used

    '''
    if sys.argv[0] == 'setup.py' and sys.argv[1] == 'install':
        _process_setup_opts(bglb, sys.argv[2:])
    else:
        if bglb.verbose:
            print("!!!!!!!!  command-line input (pip): ", bglb.cfs)
        _process_cmd_opts(bglb, bglb.cfs)

    if bglb.no_mumps:
        bglb.build_mumps = False
        bglb.do_mumps_steps = [False, False, False] # mumps, mumps_solve/swig, extension
    else:
        if bglb.ext_only:
            bglb.keep_temp = True
            bglb.do_mumps_steps = [True, False, False]
        if bglb.swig_only:
            bglb.keep_temp = True
            bglb.do_mumps_steps = [False, True, False]
        if bglb.skip_ext:
            bglb.keep_temp = True
            bglb.do_mumps_steps = [False, False, True]

    path = os.path.join(bglb.rootdir, 'external', 'mumps', 'cmbuild', 'CMakeCache.txt')
    bglb.cmakecache_4_mpi = path
