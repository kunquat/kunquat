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


def is_immutable(value):
    """
    >>> is_immutable('hello')
    True
    >>> is_immutable(None)
    True
    >>> is_immutable(14)
    True
    >>> is_immutable(long(20))
    True
    >>> is_immutable([1, 2, 3])
    False
    >>> is_immutable((1, 2))
    True
    >>> is_immutable((1, [2, 3]))
    False
    """
    if type(value) == tuple:
        for v in value:
            if not is_immutable(v):
                return False
        return True
    immutable_types = [str, int, long, type(None)]
    result = (type(value) in immutable_types)
    return result


class CommandQueue(Queue):

    def __init__(self):
        Queue.__init__(self)

    def put(self, command):
        arg = command.arg
        if is_immutable(arg) or type(arg) == Thread:
            Queue.put(self, command)
        else:
            raise TypeError

