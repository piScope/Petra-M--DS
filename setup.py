"""
setup.py file for SWIG example
"""
from setuptools import setup, find_packages
from setuptools.command.build_py import build_py as _build_py
from setuptools.command.install import install as _install
from setuptools.command.bdist_wheel import bdist_wheel as _bdist_wheel
from distutils.command.clean import clean as _clean

# these lines are necesssary for python setup.py clean #
import os, sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "_build_system"))

from build_globals import bglb
from build_config import *
from build_utils import *
from build_mumps import *

from setuptools import setup, find_packages

# To use a consistent encoding
from codecs import open
import sys
import os
import shutil

class BdistWheel(_bdist_wheel):
    def initialize_options(self):
        _bdist_wheel.initialize_options(self)
        initialize_cmd_opts(bglb)

    def finalize_options(self):
        def _has_ext_modules():
            return True
        self.distribution.has_ext_modules = _has_ext_modules
        _bdist_wheel.finalize_options(self)

    def run(self):
        if not bglb.is_configured:
            if bglb.verbose:
                print('!!!!! Running config (bdist wheel)')

            bglb.prefix = abspath(self.bdist_dir)
            bglb.bdist_wheel_dir = abspath(self.bdist_dir)
            bglb.do_bdist_wheel = True

            configure_build(bglb)


            clean_dist_info(bglb.prefix)

            self.verbose = bglb.verbose

            if bglb.keep_temp:
                self.keep_temp = True

        bglb.is_configured = True

        self.run_command("build")

        _bdist_wheel.run(self)

class BuildPy(_build_py):
    '''
    Called when python setup.py build_py
    '''
    user_options = _build_py.user_options

    def initialize_options(self):
        _build_py.initialize_options(self)

    def finalize_options(self):
        _build_py.finalize_options(self)

    def run(self):

        print_config(bglb)

        if bglb.build_mumps:
            clone_build_mumps(bglb)

        bglb.build_py_done = True

        _build_py.run(self)

class Clean(_clean):
    user_options = _clean.user_options + [
        ('ext', None, 'clean exteranal dependencies)'),
    ]

    def initialize_options(self):
        _clean.initialize_options(self)
        self.ext = False
        self.swig= False

    def run(self):
        bglb.dry_run = self.dry_run
        bglb.verbose = bool(self.verbose)


        if self.ext or self.all:
            path = os.path.join(bglb.extdir, 'mumps')
            if os.path.exists(path):
                shutil.rmtree(path)

        if self.swig or self.all:
            clean_wrapper()

        _clean.run(self)


if __name__ == '__main__':
    cmdclass = {'build_py': BuildPy,
                'clean': Clean,
                'bdist_wheel': BdistWheel}

    setup(
        cmdclass=cmdclass,
    )
