#
# InsertProjectName
# Copyright (c) 2019 The InsertProjectName Contributors
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
    options = docopt(__doc__, version='cozmonaut ' + __version__)

    if options['--delete-friend']:
        task = TaskDeleteFriend(int(options['--delete-friend']))
        task.run()

    if options['--list-friends']:
        task = TaskListFriends()
        task.run()

    if options['--run']:
        task = TaskRun(int(options['--run']))
        task.run()
