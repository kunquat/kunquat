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


class CommandQueue(Queue):

    def __init__(self):
        Queue.__init__(self)

    def put(self, command):
        arg_type = type(command.arg)
        if not arg_type in [type(''), type(None), Thread]:
            raise TypeError
        Queue.put(self, command)

