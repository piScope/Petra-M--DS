'''
  build parameters
'''

import os
from collections import namedtuple

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
