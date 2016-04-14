# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from queue import Queue
from threading import Thread

from .command import Command


class CommandQueue():
    """
    >>> q = CommandQueue()
    >>> command = Command('foo', 1, 2, 3)
    >>> q.put(command)
    >>> q.block()
    >>> q.get() == command
    True
    """

    def __init__(self):
        self._in = Queue()
        self._out = Queue()

    def block(self):
        foo = self._in.get()
        self._out.put(foo)

    def put(self, command):
        if isinstance(command, Command):
            self._in.put(command)
        else:
            raise TypeError(type(command))

    def push(self, name, *args):
        command = Command(name, *args)
        self.put(command)

    def get(self):
        return self._out.get()


