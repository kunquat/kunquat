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

import threading

from commandqueue import CommandQueue

C_COMMIT_DATA = 'commit_data'
C_GENERATE = 'generate'
C_HALT = 'halt'
C_SET_AUDIO_OUTPUT = 'set_audio_output'
C_SET_FRONTEND = 'set_frontend'
C_SET_DATA = 'set_data'

class BackendThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._q = CommandQueue()
        self._backend = None

    # Backend interface

    def set_audio_output(self, audio_output):
        self._q.push(C_SET_AUDIO_OUTPUT, audio_output)

    def set_frontend(self, frontend):
        self._q.push(C_SET_FRONTEND, frontend)

    def set_backend(self, backend):
        self._backend = backend

    def set_data(self, key, value):
        self._q.push(C_SET_DATA, key, value)

    def commit_data(self):
        self._q.push(C_COMMIT_DATA, None)

    def generate_audio(self, nframes):
        self._q.push(C_GENERATE, nframes)

    # Threading interface

    def halt(self):
        self._q.push(C_HALT, None)

    def run(self):
        cmd = self._q.get()
        while cmd.name != C_HALT:
            self._backend.__call__(cmd.name, *cmd.args)
            cmd = self._q.get()


