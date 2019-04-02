#!/usr/bin/env python

#
# InsertProjectName
# Copyright (c) 2019 The InsertProjectName Contributors
# InsertLicenseText
#

import os
import platform
import shutil
import subprocess
import sys

from setuptools import Command, find_packages
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

from cozmonaut import __version__


class CMakeExtension(Extension):
    def __init__(self, name, sources=[], cmake_lists_dir='.', **kw):
        Extension.__init__(self, name, sources=sources, **kw)

        # Absolute path to the directory holding the CMakeLists.txt file
        self.cmake_lists_dir = os.path.abspath(cmake_lists_dir)


class CMakeBuild(build_ext):
    def build_extensions(self):
        # Assert that we can actually call CMake
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError('Cannot find CMake executable')

        # Iterate all extensions to build
        for ext in self.extensions:
            # Ensure temp build directory exists
            if not os.path.exists(self.build_temp):
                os.makedirs(self.build_temp)

            # Ensure lib build directory exists
            if not os.path.exists(self.build_lib):
                os.makedirs(self.build_lib)

            # CMake configure and build
            # The library DLL will be put in the temp build directory
            subprocess.check_call(['cmake', ext.cmake_lists_dir], cwd=self.build_temp)
            subprocess.check_call(['cmake', '--build', '.'], cwd=self.build_temp)

            # The name of the DLL file
            if platform.system() == 'Linux':
                dll_name=f'lib{ext._full_name}.so'
            elif platform.system() == 'Windows':
                dll_name=f'{ext._full_name}.dll'
            else:
                raise NotImplementedError(f'Unsupported system: {platform.system()}')

            # Copy the DLL from the temp build directory to the lib build directory
            # This is where setuptools wants it to be before installation can proceed
            dll_path_temp=os.path.join(self.build_temp, dll_name)
            dll_path_lib=os.path.join(self.build_lib, ext._file_name)
            shutil.copy(dll_path_temp, dll_path_lib)


class CmdTest(Command):
    """Run our tests."""
    description = 'run tests'
    user_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        subprocess.call(['py.test', '--cov=cozmonaut', '--cov-report=term-missing'])


setup(
    name='cozmonaut',  # TODO: Need a better name?
    version=__version__,
    description='Our Cozmo program',  # TODO
    long_description='Our Cozmo program',  # TODO
    url='/we/need/a/github/repo',  # TODO
    author='OurTeamName',  # TODO
    author_email='OurTeamEmail',  # TODO
    license=None,  # TODO: Add our license name here
    classifiers=[
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.5',
    ],
    packages=find_packages(
        exclude=[
            'docs',
            'tests',
        ],
    ),
    ext_modules=[
        CMakeExtension('facelib'),
    ],
    install_requires=[
        'docopt',
        'numpy',
        'pillow',
    ],
    extras_require={
        'test': [
            'coverage',
            'pytest',
            'pytest-cov',
        ],
    },
    entry_points={
        'console_scripts': [
            'cozmonaut=cozmonaut.cli:main',
        ],
    },
    cmdclass={
        'build_ext': CMakeBuild,
        'test': CmdTest,
    },
)

# TODO: Package up the dlib datafiles as data and use them as resources
