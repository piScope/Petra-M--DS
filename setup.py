"""
setup.py file for SWIG example
"""
from 
from setuptools import setup, find_packages
from setuptools.command.build_py import build_py as _build_py
from setuptools.command.install import install as _install
from setuptools.command.bdist_wheel import bdist_wheel as _bdist_wheel
from distutils.command.clean import clean as _clean

from _build_system.build_global import *

from setuptools import setup, find_packages

# To use a consistent encoding
from codecs import open
import sys
import os


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
                'install': Install,
                'install_lib': InstallLib,
                'install_egg_info': InstallEggInfo,
                'install_scripts': InstallScripts,
                'clean': Clean,
                'bdist_wheel': BdistWheel}

    setup(
        cmdclass=cmdclass,
    )
