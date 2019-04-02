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

from setuptools import Command, find_packages
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

from cozmonaut import __version__


class CMakeExtension(Extension):
    """An extension module built with CMake."""

    def __init__(self, name, cmake_lists_dir='.', **kw):
        Extension.__init__(self, name, sources=[], **kw)

        # Absolute path to the directory holding the CMakeLists.txt file
        self.cmake_lists_dir = os.path.abspath(cmake_lists_dir)


# noinspection PyPep8Naming
class build_ext_cmake(build_ext):
    """A setuptools command to build CMakeExtension objects."""

    def build_extensions(self):
        # Assert that we can actually call CMake
        try:
            subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError('Unable to execute CMake')

        # Iterate all extensions to build
        for ext in self.extensions:
            # Ensure temp build directory exists
            if not os.path.exists(self.build_temp):
                os.makedirs(self.build_temp)

            # Ensure lib build directory exists
            if not os.path.exists(self.build_lib):
                os.makedirs(self.build_lib)

            # The full name of the extension
            # noinspection PyProtectedMember
            ext_name = ext._full_name

            # The filename setuptools wants for the extension DLL
            # noinspection PyProtectedMember
            ext_file = ext._file_name

            # CMake configure and build
            # The library DLL will be put in the temp build directory
            subprocess.check_call(['cmake', ext.cmake_lists_dir], cwd=self.build_temp)
            subprocess.check_call(['cmake', '--build', '.'], cwd=self.build_temp)

            # The name of the DLL file
            if platform.system() == 'Linux':
                dll_name = f'lib{ext_name}.so'
            elif platform.system() == 'Windows':
                dll_name = f'{ext_name}.dll'
            else:
                raise NotImplementedError(f'Unsupported system: {platform.system()}')

            # Copy the DLL from the temp build directory to the lib build directory
            # This is where setuptools wants it to be before installation can proceed
            dll_path_temp = os.path.join(self.build_temp, dll_name)
            dll_path_lib = os.path.join(self.build_lib, ext_file)
            shutil.copy(dll_path_temp, dll_path_lib)


class CmdTest(Command):
    """Run our tests."""

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        subprocess.call(['py.test', '--cov=cozmonaut', '--cov-report=term-missing'])


setup(
    name='cozmonaut',
    version=__version__,
    description='CS4500 project by team Cozmonauts',
    long_description='Our Cozmo program',  # TODO
    url='/we/need/a/github/repo',  # TODO
    author='Cozmonauts',  # TODO
    license=None,  # TODO: Add our license name here
    classifiers=[
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.7',
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
        'pyaudio',
        'SpeechRecognition',
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
        'build_ext': build_ext_cmake,
        'test': CmdTest,
    },
)

# TODO: Package up the dlib datafiles as data and use them as resources
