"""
InsertProjectName
Copyright (c) 2019 The InsertProjectName Contributors
InsertLicenseText
"""

from docopt import docopt

from . import __version__

# Command-line documentation
# docopt will parse this into the CLI options
DOCUMENTATION = """cozmonaut

Usage:
  cozmonaut --delete-friend <id>
  cozmonaut --list-friends
  cozmonaut --run <num>
  cozmonaut (-h | --help)
  cozmonaut --version

Options:
  --delete-friend <id>              Delete friend by their ID.
  --list-friends                    List all friends made.
  --run <num>                       Run program with <num> Cozmos.
  -h --help                         Show this information.
  --version                         Show version information.
"""


def main():
    # Parse command-line options
    options = docopt(DOCUMENTATION, version='cozmonaut ' + __version__)

    # If --delete-friend was supplied
    if options['--delete-friend']:
        print('Deleting friend ' + options['--delete-friend'])
        print('Not implemented!')

    # If --list-friends was supplied
    if options['--list-friends']:
        print('Listing friends...')
        print('Not implemented!')

    # If --run was supplied
    if options['--run']:
        print('Running program with ' + options['--run'] + ' Cozmos')
        print('Not implemented!')
