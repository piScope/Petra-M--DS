'''
  build parameters
'''

import os
import sys
import sysconfig
from collections import namedtuple


__all__ = ("bglb", "REPOS")


class BuildGlobal():
    '''
    Global parameters
    '''

    def __init__(self):
        # flag to monitor build steps

        self.dry_run = False
        self.verbose = False
        self.keep_temp = False
        self.is_configured = False
        self.build_py_done = False

        # directory
        rootdir = os.path.join(os.path.abspath(
            os.path.dirname(__file__)), "..")
        extdir = os.path.join(rootdir, 'external')
        if not os.path.exists(extdir):
            os.mkdir(os.path.join(rootdir, 'external'))
        self.rootdir = rootdir
        self.extdir = extdir

        # platforms
        osx_sysroot = ''
        if sys.platform == "linux" or sys.platform == "linux2":
            self.dylibext = '.so'

        elif sys.platform == "darwin":
            self.dylibext = '.dylib'
            for i, x in enumerate(sysconfig.get_config_vars()['CFLAGS'].split()):
                if x == '-isysroot':
                    osx_sysroot = sysconfig.get_config_vars()['CFLAGS'].split()[
                        i+1]
                    break
        elif sys.platform == "win32":
            assert False, "Windows is not supported yet. Contribution is welcome"

        else:
            assert False, "unsupported platform : " + sys.platform
        self.osx_sysroot = osx_sysroot

        # configuration parameters

        from build_config import initialize_cmd_opts
        initialize_cmd_opts(self)

        #
        self.build_mumps = True


rels = namedtuple('Release', ['version', 'defbranch', 'hash', 'tarball'])

REPOS = dict(
    mumps=dict(
        url="https://github.com/scivision/mumps",
        # version, hash, tarball
        releases=[
            rels("5.8.1.2", "main", "e583b83d41823759c83403217d9e3713eedc60e4", None),
        ]
    ),
)


bglb = BuildGlobal()
