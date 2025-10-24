"""
Helper functions for setup.py
"""

import os
import sys
import configparser
from urllib import request
import itertools
import site
import re
import subprocess
import multiprocessing
import ssl
import tarfile
from collections import namedtuple
from shutil import which as find_command

__all__ = ("abspath", "make_call", "chdir", "remove_files",
           "make", "make_install", "download", "gitclone",
           "cmake",)

from build_globals import bglb, REPOS

# ----------------------------------------------------------------------------------------
# Utilities
# ----------------------------------------------------------------------------------------


def abspath(path):
    return os.path.abspath(os.path.expanduser(path))


def make_call(command, target='', force_verbose=False, env=None):
    '''
    call command
    '''
    print("current working dir", os.getcwd())
    print("calling ... " + " ".join(command))

    if bglb.dry_run:
        return
    kwargs = {'universal_newlines': True, 'env': env}
    if env is not None:
        env.update(os.environ)

    myverbose = bglb.verbose or force_verbose
    if not myverbose:
        kwargs['stdout'] = subprocess.DEVNULL
        kwargs['stderr'] = subprocess.DEVNULL
    else:
        kwargs['capture_output'] = True

    try:
        p = subprocess.run(command, **kwargs)

    except subprocess.CalledProcessErro:
        if target == '':
            target = " ".join(command)
        print("Failed when calling command: " + target)
        print("Subprocess stdout:", p.stdout.strip())
        print("Subprocess stderr:", p.stderr.strip())

        return False, p.stdout.strip(), p.stdout.strip()

    if myverbose:
        print(p.stdout.strip())

    return True, '', ''


def chdir(path):
    '''
    change directory to `path`; returns the previous directory
    '''
    pwd = os.getcwd()
    os.chdir(path)
    if bglb.verbose:
        print("Moving to a directory : " + path)
    return pwd


def remove_files(files):
    for f in files:
        if bglb.verbose:
            print("Removing : " + f)
        if bglb.dry_run:
            continue
        os.remove(f)


def make(target):
    '''
    make : add -j option automatically
    '''
    command = ['make', '-j', str(max((multiprocessing.cpu_count() - 1, 1)))]
    make_call(command, target=target, force_verbose=True)


def make_install(target, prefix=None):
    '''
    make install
    '''
    command = ['make', 'install']
    if prefix is not None:
        command.append('prefix='+prefix)
    make_call(command, target=target)


def download(xxx):
    '''
    download tar.gz from somewhere. xxx is name.
    url is given by repos above
    '''

    if os.path.exists(os.path.join(bglb.extdir, xxx)):
        print("Download " + xxx + " skipped. Use clean --all-exts if needed")
        return
    # Get the tarball for the latest release
    url = REPOS[xxx]["releases"][-1].tarball
    if url is None:
        raise RuntimeError(f"Could not find tarball URL for {xxx}")
    print("Downloading :", url)

    if bglb.use_unverifed_SSL:
        ssl._create_default_https_context = ssl._create_unverified_context

    ftpstream = request.urlopen(url)
    targz = tarfile.open(fileobj=ftpstream, mode="r|gz")
    targz.extractall(path=bglb.extdir)
    os.rename(os.path.join(bglb.extdir, targz.getnames()[0].split('/')[0]),
              os.path.join(bglb.extdir, xxx))


def gitclone(xxx, use_sha=False, branch='master'):
    cwd = os.getcwd()
    repo_xxx = os.path.join(bglb.extdir, xxx)

    if os.path.exists(repo_xxx):
        os.chdir(repo_xxx)
        if branch == 'master':
            branch = REPOS[xxx]["releases"][-1].defbranch
        command = ['git', 'checkout', branch]
        make_call(command)
        command = ['git', 'pull']
        make_call(command)
    else:
        repo = REPOS[xxx]["url"]
        if bglb.git_ssh:
            repo = repo.replace("https://github.com/", "git@github.com:")

        os.chdir(bglb.extdir)
        command = ['git', 'clone', repo, xxx]
        make_call(command)

    if not bglb.dry_run:
        if not os.path.exists(repo_xxx):
            print(repo_xxx + " does not exist. Check if git clone worked")
        os.chdir(repo_xxx)

        if use_sha:
            sha = REPOS[xxx]["releases"][-1].hash
            command = ['git', 'checkout',  sha]
        else:
            if branch == 'master':
                branch = REPOS[xxx]["releases"][-1].defbranch
            command = ['git', 'checkout', branch]
        make_call(command)
    os.chdir(cwd)


def cmake(*args, **kwargs):
    '''
    run cmake. must be called in the target directory
    '''
    command = ['cmake'] + list(args)
    for key, value in kwargs.items():
        command.append('-' + key + '=' + value)

    if bglb.osx_sysroot != '':
        command.append('-DCMAKE_OSX_SYSROOT=' + osx_sysroot)

    flag, stdout, stdrrr = make_call(command)
    return flag, stdout, stdrrr
