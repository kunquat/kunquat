# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from Queue import Queue
from threading import Thread
from command import Command


class CommandQueue(Queue):
    """
    >>> q = CommandQueue()
    >>> command = Command('foo', 1, 2, 3)
    >>> q.put(command)
    >>> q.get() == command
    True
    """

    def __init__(self):
        Queue.__init__(self)

    def put(self, command):
        if isinstance(command, Command):
            Queue.put(self, command)
        else:
            raise TypeError(type(command))

