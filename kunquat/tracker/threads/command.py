# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2016
#          Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import pickle

from threading import Thread


ARG_PICKLE = 'pickle'
ARG_THREAD = 'thread'


class Command():
    """
    Stores commands with protected values and thread objects.

    >>> thread_arg = Thread()
    >>> list_arg = [1, 2, 3]
    >>> command = Command('foo', list_arg, thread_arg)
    >>> list_arg[0] = 8
    >>> del list_arg[1]
    >>> list_arg
    [8, 3]
    >>> command.name
    'foo'
    >>> command.args[0]
    [1, 2, 3]
    >>> command.args[1] == thread_arg
    True
    """

    def __init__(self, name, *args):
        self._name = name
        self._frozen_args = [self._freeze_arg(i) for i in args]

    @property
    def name(self):
        return self._name

    @property
    def args(self):
        return [self._melt_arg(i) for i in self._frozen_args]

    def _freeze_arg(self, arg):
        if isinstance(arg, Thread):
            return (ARG_THREAD, arg)
        return (ARG_PICKLE, pickle.dumps(arg))

    def _melt_arg(self, frozen):
        (arg_type, value) = frozen
        if arg_type == ARG_THREAD:
            return value
        if arg_type == ARG_PICKLE:
            return pickle.loads(value)
        assert False


