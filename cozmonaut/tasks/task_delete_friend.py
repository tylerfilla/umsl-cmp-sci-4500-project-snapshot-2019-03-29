#
# Cozmonaut
# Copyright (c) 2019 The Cozmonaut Contributors
#
# InsertLicenseText
#


class TaskDeleteFriend:
    """
    A task to delete a friend.
    """

    def __init__(self, friend_id):
        self.friend_id = friend_id

    def run(self):
        print(f'Delete friend {self.friend_id}... NOT IMPLEMENTED')
