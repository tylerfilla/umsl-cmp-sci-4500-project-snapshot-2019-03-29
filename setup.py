#!/usr/bin/env python

#
# InsertProjectName
# Copyright (c) 2019 The InsertProjectName Contributors
# InsertLicenseText
#

#
# NOTICE
#
# The CMake invocation code is from https://github.com/m-pilia/disptools/blob/master/setup.py
# Copyright 2018 Martino Pilia <martino.pilia@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

import os
import platform
import subprocess
import sys
from pprint import pprint

from setuptools import Command, find_packages
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

from cozmonaut import __version__

# Parse command line flags
options = {k: 'OFF' for k in ['--opt', '--debug', '--cuda']}
for flag in options.keys():
    if flag in sys.argv:
        options[flag] = 'ON'
        sys.argv.remove(flag)

# Command line flags forwarded to CMake
cmake_cmd_args = []
for f in sys.argv:
    if f.startswith('-D'):
        cmake_cmd_args.append(f)
        sys.argv.remove(f)


class CMakeExtension(Extension):
    def __init__(self, name, cmake_lists_dir='.', sources=[], **kwa):
        Extension.__init__(self, name, sources=sources, **kwa)
        self.cmake_lists_dir = os.path.abspath(cmake_lists_dir)


class CMakeBuild(build_ext):
    def build_extensions(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError('Cannot find CMake executable')

        for ext in self.extensions:

            extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
            cfg = 'Debug' if options['--debug'] == 'ON' else 'Release'

            cmake_args = [
                '-DDISPTOOLS_DEBUG=%s' % ('ON' if cfg == 'Debug' else 'OFF'),
                '-DDISPTOOLS_OPT=%s' % options['--opt'],
                '-DDISPTOOLS_VERBOSE=ON',
                '-DDISPTOOLS_LOW_ORDER_PD=OFF',
                '-DDISPTOOLS_DOUBLE=OFF',
                '-DDISPTOOLS_CUDA_SUPPORT=%s' % options['--cuda'],
                '-DDISPTOOLS_CUDA_ERROR_CHECK=ON',
                '-DDISPTOOLS_CUDA_ERROR_CHECK_SYNC=ON',
                '-DDISPTOOLS_PYTHON_SUPPORT=ON',
                '-DDISPTOOLS_PYTHON_C_MODULE_NAME=%s' % 'facelib',
                '-DCMAKE_BUILD_TYPE=%s' % cfg,
                '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir),
                '-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), self.build_temp),
                '-DPYTHON_EXECUTABLE={}'.format(sys.executable),
            ]

            if platform.system() == 'Windows':
                plat = ('x64' if platform.architecture()[0] == '64bit' else 'Win32')
                cmake_args += [
                    '-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE',
                    '-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), extdir),
                ]
                if self.compiler.compiler_type == 'msvc':
                    cmake_args += [
                        '-DCMAKE_GENERATOR_PLATFORM=%s' % plat,
                    ]
                else:
                    cmake_args += [
                        '-G', 'MinGW Makefiles',
                    ]

            cmake_args += cmake_cmd_args

            pprint(cmake_args)

            if not os.path.exists(self.build_temp):
                os.makedirs(self.build_temp)

            # Config and build the extension
            subprocess.check_call(['cmake', ext.cmake_lists_dir] + cmake_args,
                                  cwd=self.build_temp)
            subprocess.check_call(['cmake', '--build', '.', '--config', cfg], cwd=self.build_temp)


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
