"""
setup.py file for SWIG example
"""
import sys
import os
import shutil
import mpi4py
import numpy

from setuptools import setup, Extension
from setuptools.command.build_py import build_py as _build_py
from setuptools.command.build_ext import build_ext as _build_ext
from setuptools.command.install import install as _install
from setuptools.command.bdist_wheel import bdist_wheel as _bdist_wheel
from distutils.command.clean import clean as _clean

# these lines are necesssary for python setup.py clean #
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "_build_system"))

from build_globals import bglb
from build_config import (print_config,
                          initialize_cmd_opts,
                          configure_build,
                          clean_dist_info,
                          clean_wrapper)
from build_utils import (abspath,
                         find_mpi_include)
from build_mumps import clone_build_mumps
from build_mumps_solve import cmake_mumps_solve, generate_mumps_solve_wrapper


#
#  We set this here. Setting this in pyproject.toml is experimental (2025. Oct)
#
all_extensions = [
    Extension(
        "petram.ext.mumps._mumps_solve",
        sources=["python/petram/ext/mumps/mumps_solve_wrap.cxx",],
        libraries=["mumps_solve"]
    ),
]
common_macros = [('TARGET_PY3', '1'),
                 ('NPY_NO_DEPRECATED_API', 'NPY_1_7_API_VERSION')]

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

            bglb.bdist_wheel_prefix = abspath(self.bdist_dir)

            configure_build(bglb)
            clean_dist_info(bglb.bdist_wheel_prefix)

            self.verbose = bglb.verbose

            if bglb.keep_temp:
                self.keep_temp = True

        bglb.is_configured = True

        print_config(bglb)
        _bdist_wheel.run(self)

        print("end of bdistwheel::run")


class Install(_install):
    def run(self):
        print("Running Install")
        bglb.running_install = True
        _install.run(self)
        bglb.running_install = False

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
        print("Running BuildPy")
        if bglb.do_mumps_steps[0]:
            clone_build_mumps(bglb)

        _build_py.run(self)
        print("end of buildpy::run")

class BuildExt(_build_ext):
    def run(self):
        print("Running BuildExt")

        if bglb.do_mumps_steps[1]:
            cmake_mumps_solve(bglb)
            generate_mumps_solve_wrapper(bglb)

        selected_ext = []
        mpi4pyinc = mpi4py.get_include()
        numpyinc = numpy.get_include()

        mpiinc = find_mpi_include(bglb)

        for item in self.extensions:
            if bglb.do_mumps_steps[2] and item.name == 'petram.ext.mumps._mumps_solve':
                mumpsinc = os.path.join(bglb.rootdir, "external", "mumps", "cmbuild",  "local", "include")
                mumpssolveinc = os.path.join(bglb.rootdir, "mumps_solve")

                item.include_dirs.append(numpyinc)
                item.include_dirs.append(mpiinc)
                item.include_dirs.append(mumpssolveinc)
                item.include_dirs.append(mumpsinc)
                item.include_dirs.append(mpi4pyinc)
                item.define_macros.extend(common_macros)

                selected_ext.append(item)

        if len(selected_ext) == 0:
            return

        self.extensions = selected_ext

        # this is common to all (future) extension libraries
        for item in self.extensions:
            item.library_dirs.append(os.path.join(bglb.bdist_wheel_prefix, "petram", "external", "lib"))
            if sys.platform in ("linux", "linux2"):
                item.runtime_library_dirs.append("$ORIGIN/../../external/lib")
            elif sys.platform == "darwin":
                item.runtime_library_dirs.append("@loader_path/../../external/lib")

            print(item)

        _build_ext.run(self)


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
    cmdclass = {'install': Install,
                'build_py': BuildPy,
                'build_ext': BuildExt,
                'clean': Clean,
                'bdist_wheel': BdistWheel}


    setup(
        ext_modules=all_extensions,
        cmdclass=cmdclass,
    )
