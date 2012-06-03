# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

"""A library for accessing Kunquat music data.

This module provides interfaces for inspecting and modifying Kunquat
compositions and rendering them to digital audio.

Classes:
EHandle

"""

from __future__ import print_function
import json
import kunquat
from kunquat.storage.store import Store

__all__ = ['EHandle']


class EHandle(kunquat.MHandle):

    def __init__(self, path, mixing_rate):
        """Create a new EHandle.

        Arguments:
        path -- The path to the Kunquat composition project directory.
                Normally, this directory contains the subdirectories
                "committed" and "workspace", although new project
                directories may be empty.

        Optional arguments:
        mixing_rate -- Mixing rate in frames per second.  Typical
                       values include 44100 ("CD quality") and 48000
                       (the default).

        """
        super(EHandle, self).__init__(mixing_rate)
        self._store = Store(path)

    def __getitem__(self, key):
        """Get data from the handle based on a key.

        Arguments:
        key -- The key of the data in the composition.  A key consists
               of one or more textual elements separated by forward
               slashes ('/').  The last element is the only one that
               is allowed and required to contain a period.  Examples:
               'p_composition.json'
               'pat_000/col_00/p_events.json'
               'ins_01/kqtiXX/p_instrument.json'
               The 'XX' in the last example should be written
               literally.  It is expanded to the file format version
               number behind the scenes.

        Return value:
        The data associated with the key if found, otherwise None.
        The function converts JSON data to Python objects.

        Exceptions:
        KunquatArgumentError -- The key is not valid.
        KunquatResourceError -- Retrieving the data failed.  This can
                                usually occur only with subclasses of
                                RHandle.

        """
        try:
            value = self._store[key]
        except KeyError:
            value = ''
        (_, suffix) = key.split('.')
        if suffix.startswith('.json'):
            return json.loads(value) if value else None
        return value if value else None

    def __setitem__(self, key, value):
        """Set data in the handle.

        Arguments:
        key   -- The key of the data in the composition.  This refers
                 to the same data as the key argument of __getitem__
                 and the same formatting rules apply.
        value -- The data to be set.  For JSON keys, this should be a
                 Python object -- it is automatically converted to a
                 JSON string.

        Exceptions:
        KunquatArgumentError -- The key is not valid.
        KunquatFormatError   -- The data is not valid.  Only the data
                                that audibly affects mixing is
                                validated.
        KunquatResourceError -- File system access failed.

        """
        self.set_data(key, value)

        (_, suffix) = key.split('.')
        if suffix.startswith('.json'):
            value = json.dumps(value) if value else ''
        elif value == None:
            value = ''
        self._store[key] = value

    def commit(self):
        """Commit the changes made in the handle.

        Exceptions:
        KunquatFormatError   -- The project contains invalid data.
                                This usually means that the workspace
                                was modified manually or there's a bug
                                in Kunquat.
        KunquatResourceError -- File system access failed.

        If any exception is raised during the commit, it is usually a
        good idea to discard the handle and create a new one for the
        project.  This will initiate a recovery procedure that will
        restore a valid composition state.  Changes made after the last
        successful commit are possibly lost in this case, though.

        """
        self._store.commit()

    def flush(self):
        """

        """
        self._store.flush()


