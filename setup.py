#!/usr/bin/env python

"""
InsertProjectName
Copyright (c) 2019 The InsertProjectName Contributors
InsertLicenseText
"""

import subprocess

from setuptools import Command, find_packages, setup

from cozmonaut import __version__


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
        'Programming Language :: Python :: 3.5'
    ],
    packages=find_packages(
        exclude=['docs', 'tests'],
    ),
    install_requires=[],
    extras_require={
        'test': ['coverage', 'pytest', 'pytest-cov'],
    },
    entry_points={
        'console_scripts': [
            'cozmonaut=cozmonaut.cli:main',
        ],
    },
    cmdclass={'test': CmdTest},
)
