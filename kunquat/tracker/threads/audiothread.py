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

import json
import threading

from command import Command
from commandqueue import CommandQueue
from kunquat.tracker.audio.audio import Audio

C_HALT = 'halt'
C_PUT_AUDIO = 'put_audio'


class AudioThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._backend = None
        self._q = CommandQueue()
        self._audio = Audio()

    # Driver interface

    def set_backend(self, backend):
        self._audio.set_backend(backend)

    def set_frontend(self, frontend):
        self._audio.set_frontend(frontend)

    def select_driver(self, name):
        self._audio.select_driver(name)

    def put_audio(self, audio):
        arg = json.dumps(audio)
        self._q.put(Command(C_PUT_AUDIO, arg))

    # Threading interface

    def halt(self):
        self._q.put(Command(C_HALT, None))

    def run(self):
        self._audio.request_update()
        cmd = self._q.get()
        while cmd.name != C_HALT:
            if cmd.name == C_PUT_AUDIO:
                audio = tuple(json.loads(cmd.arg))
                self._audio.put_audio(audio)
            else:
                assert False
            cmd = self._q.get()


