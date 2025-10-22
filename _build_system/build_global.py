'''
  build parameters
'''

import os
from collections import namedtuple
from shutil import which as find_command


__all__ = ("bglb", "mumps_build_opts",)


class BuildGlobal():
    '''
    Global parameters
    '''
    def __init__(self):
        self.dry_run = False
        self.verbose = False

        rootdir = os.path.join(os.path.abspath(os.path.dirname(__file__)), "..")
        extdir = os.path.join(rootdir, 'external')
        if not os.path.exists(extdir):
            os.mkdir(os.path.join(rootdir, 'external'))
        self.rootdir = rootdir
        self.extdir = extdir

        # compilers
        
        self.cc_command = 'cc' if os.getenv("CC") is None else os.getenv("CC")
        self.cxx_command = 'c++' if os.getenv("CC") is None else os.getenv("CXX")
        self.fc_command = 'c++' if os.getenv("FC") is None else os.getenv("FC")        
        self.mpicc_command = 'mpicc' if os.getenv("MPICC") is None else os.getenv("MPICC")
        self.mpicxx_command = 'mpic++' if os.getenv(
             "MPICXX") is None else os.getenv("MPICXX")
        self.mpifc_command = 'mpic++' if os.getenv(
             "MPIFC") is None else os.getenv("MPIFC")
        self.cxxstd_flag = '-std=c++17' if os.getenv(
             "CXXSTDFLAG") is None else os.getenv("CXXSTDFLAG")
        self.mpiinc = '' if os.getenv("MPIINC") is None else os.getenv("MPIINC")

        self.swig_command = (find_command('swig') if os.getenv("SWIG") is None
                     else os.getenv("SWIG"))
        if swig_command is None:
            assert False, "SWIG is not installed (hint: pip install swig)"

    
        self.build_mumps = True


class MUMPSBuildOpts():
    '''
    Options for building MUMPS
    '''
    
    def __init__(self):
        self.mumps_cmake = "https://github.com/scivision/mumps"
        self.int64 = False
        self.dmumps = True
        self.zmumps = True
        self.smumps = True
        self.cmumps = True
        self.scotch = True
        self.parallel = True
        self.metis = False
        self.parmetis = False

rels = namedtuple('Release', ['version', 'hash', 'tarball'])

REPOS = dict(
    mumps=dict(
        url="https://github.com/scivision/mumps",
        # version, hash, tarball
        releases=[
            rels("5.8.1.2", "e583b83d41823759c83403217d9e3713eedc60e4", None),
        ]
    ),
    ),
        

bglb = BuildGlobal()
mumps_build_opts = MUMPSBuildOpts()
