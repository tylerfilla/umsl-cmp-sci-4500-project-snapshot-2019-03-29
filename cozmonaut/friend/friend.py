#
# InsertProjectName
# Copyright (c) 2019 The InsertProjectName Contributors
# InsertLicenseText
#

from datetime import datetime

import PIL


class Friend:
    """
    A friend of Cozmo.

    This class gathers together all the info we need about a single friend that
    Cozmo has made throughout his life.
    """

    def __init__(self, fid: int, name: str = "", photo: PIL.Image.Image = None, when_first_seen: datetime = None,
                 when_last_seen: datetime = None):
        self._fid = fid
        self._name = name
        self._photo = photo
        self._when_first_seen = when_first_seen
        self._when_last_seen = when_last_seen

    @property
    def fid(self) -> int:
        """The unique friend ID."""
        return self._fid

    @fid.setter
    def fid(self, value: int):
        """The unique friend ID."""
        self._fid = value

    @property
    def name(self) -> str:
        """The friend's name."""
        return self._name

    @name.setter
    def name(self, value: str):
        """The friend's name."""
        self._name = value

    @property
    def photo(self) -> PIL.Image.Image:
        """The friend's photo."""
        return self._photo

    @photo.setter
    def photo(self, value: PIL.Image.Image):
        """The friend's photo."""
        self._photo = value

    @property
    def when_first_seen(self) -> datetime:
        """The date and time when the friend was first seen."""
        return self._when_first_seen

    @when_first_seen.setter
    def when_first_seen(self, value: datetime):
        """The date and time when the friend was first seen."""
        self._when_first_seen = value

    @property
    def when_last_seen(self) -> datetime:
        """The date and time when the friend was last seen."""
        return self._when_last_seen

    @when_last_seen.setter
    def when_last_seen(self, value: datetime):
        """The date and time when the friend was last seen."""
        self._when_last_seen = value
