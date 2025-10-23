"""
setup.py file for SWIG example
"""
from setuptools import setup, find_packages
from setuptools.command.build_py import build_py as _build_py
from setuptools.command.install import install as _install
from setuptools.command.bdist_wheel import bdist_wheel as _bdist_wheel
from distutils.command.clean import clean as _clean

from build_global import bglb
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
        initialize_cmd_options(self)

    def finalize_options(self):
        def _has_ext_modules():
            return True
        self.distribution.has_ext_modules = _has_ext_modules
        _bdist_wheel.finalize_options(self)

    def run(self):
        import build_globals as bglb

        if not bglb.is_configured:
            if bglb.verbose:
                print('!!!!! Running config (bdist wheel)')
            bglb.prefix = abspath(self.bdist_dir)
            bglb.ext_prefix = os.path.join(bglb.prefix, 'mfem', 'external')
            bglb.bdist_wheel_dir = abspath(self.bdist_dir)
            bglb.do_bdist_wheel = True

            configure_build(self)
            clean_dist_info(bglb.prefix)
            if bglb.keep_temp:
                self.keep_temp = True
            print_config()

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
        bglb.build_py_done = True

        if bglb.build_mumps:
            clone_mumps()
            cmake_mumps(opts)
            build_mumps()

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

        os.chdir(bglb.extdir)


        if self.ext:
            path = os.path.join(bglb.extdir, 'mumps')
            if os.path.exists(path):
                shutil.rmtree(path)
                
        if self.swig or self.all:
            clean_wrapper()

        clean_so(all=self.all)

        os.chdir(rootdir)
        _clean.run(self)


if __name__ == '__main__':
    cmdclass = {'build_py': BuildPy,
                'clean': Clean,
                'bdist_wheel': BdistWheel}

    setup(
        cmdclass=cmdclass,
    )
