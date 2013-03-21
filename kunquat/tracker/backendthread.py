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

C_GENERATE = 'generate'
C_HALT = 'halt'

class BackendThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._q = Queue.Queue()
        self._backend = Backend()

    # Backend interface

    def set_audio_driver(self, driver):
        self._backend.set_audio_driver(driver)

    def set_frontend(self, frontend):
        self._backend.set_frontend(frontend)

    def generate(self, nframes):
        self._q.put(Command(C_GENERATE, nframes))

    # Threading interface

    def halt(self):
        self._q.put(Command(C_HALT, None))

    def run(self):
        cmd = self._q.get()
        while cmd.name != C_HALT:
            if cmd.name == C_GENERATE:
                self._backend.generate(cmd.arg)
            else:
                assert False
            cmd = self._q.get()


