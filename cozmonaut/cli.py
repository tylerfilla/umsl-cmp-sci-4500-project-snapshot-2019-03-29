#
# Cozmonaut
# Copyright (c) 2019 The Cozmonaut Contributors
#
# InsertLicenseText
#

"""cozmonaut

Usage:
  cozmonaut --delete-friend <id> | --list-friends
  cozmonaut --run <num>
  cozmonaut (-h | --help)
  cozmonaut --version

Options:
  --delete-friend <id>              Delete friend by their ID.
  --list-friends                    List all friends made.
  --run <num>                       Run program with num Cozmos.
  -h --help                         Show this information.
  --version                         Show version information.

"""

from docopt import docopt

from cozmonaut.tasks import TaskDeleteFriend, TaskListFriends, TaskRun
from . import __version__


def main():
    """The main function of the command-line interface."""

    # Parse cmdline arguments
    # This interprets the docstring above for the arguments
    options = docopt(__doc__, version='cozmonaut ' + __version__)

    if options['--delete-friend']:
        # The friend ID parameter
        param_id = int(options['--delete-friend'])

        task = TaskDeleteFriend(param_id)
        task.run()

    if options['--list-friends']:
        task = TaskListFriends()
        task.run()

    if options['--run']:
        # The Cozmo number parameter
        # TODO: Will this even be needed?
        param_num = int(options['--run'])

        task = TaskRun(param_num)
        task.run()
