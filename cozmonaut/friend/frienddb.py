#
# InsertProjectName
# Copyright (c) 2019 The InsertProjectName Contributors
# InsertLicenseText
#
from typing import List

from cozmonaut.friend import Friend


class FriendDB:
    """
    A database for all of the Cozmos' friends.

    Eventually, this needs to be backed by the filesystem or a real database.
    All the data need not be held in memory all the time, so these functions can
    actually be backed by queries.

    We might even consider making FriendDB abstract and then implementing it for
    different types of data stores (e.g. have a real data store and a mock data
    store for testing).
    """

    def __init__(self):
        self.store = {}

    def add(self, friend: Friend):
        """Add a friend to the database."""
        self.store[friend.fid] = friend

    def remove(self, fid: int):
        """Remove a friend from the database."""
        del self.store[fid]

    def get(self, fid: int) -> Friend:
        """Get a friend from the database."""
        return self.store[fid]

    def list(self) -> List[int]:
        """List the IDs of all known friends."""
        return list(self.store.keys())
