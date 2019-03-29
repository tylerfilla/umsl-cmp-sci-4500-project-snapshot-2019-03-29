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
  cozmonaut [--delete-friend <num>] [--list-friends]
  cozmonaut (-h | --help)
  cozmonaut --version

Options:
  --delete-friend <num>             Delete friend by their number.
  --list-friends                    List all friends made.
  -h --help                         Show this information.
  --version                         Show version information.

Help:
  If you supply both --delete-friend and --list-friends, the former will be
  processed first, and the deleted friend will not be listed by the latter.
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
