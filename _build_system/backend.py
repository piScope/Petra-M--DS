from setuptools import build_meta as _orig
from setuptools.build_meta import *

import build_globals as bglb


def get_requires_for_build_wheel(config_settings=None):
    ret = _orig.get_requires_for_build_wheel(config_settings)
    return ret


def get_requires_for_build_sdist(config_settings=None):
    return _orig.get_requires_for_build_sdist(config_settings)


def build_wheel(*args, **kwargs):
    from build_globals import bglb
    bglb.cfs = args[1]
    if bglb.cfs is None:
        bglb.cfs = {}

    return _orig.build_wheel(*args, **kwargs)
