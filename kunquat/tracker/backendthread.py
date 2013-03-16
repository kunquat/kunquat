# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013
#          Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import Queue
import threading

from backend.backend import Backend
from command import Command


class BackendThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._q = Queue.Queue()
        self._backend = Backend()

    def set_event_processor(self, ep):
        self._backend.set_event_processor(ep)

    def queue_command(self, cmd):
        self._q.put(cmd)

    def halt(self):
        self.queue_command(Command('halt', None))

    def run(self):
        cmd = self._q.get()
        while cmd.name != 'halt':
            self._backend.process_command(cmd)
            cmd = self._q.get()


