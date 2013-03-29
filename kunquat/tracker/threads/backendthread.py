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

HALT = None

class BackendThread(threading.Thread):

    def __init__(self, backend):
        threading.Thread.__init__(self)
        self._q = CommandQueue()
        self._backend = backend

    # Backend interface

    def set_audio_output(self, audio_output):
        self._q.push('set_audio_output', audio_output)

    def set_frontend(self, frontend):
        self._q.push('set_frontend', frontend)

    def set_data(self, key, value):
        self._q.push('set_data', key, value)

    def commit_data(self):
        self._q.push('commit_data')

    def generate_audio(self, nframes):
        self._q.push('generate_audio', nframes)

    # Threading interface

    def halt(self):
        self._q.push(HALT)

    def run(self):
        cmd = self._q.get()
        while cmd.name != HALT:
            self._backend.__call__(cmd.name, *cmd.args)
            cmd = self._q.get()


