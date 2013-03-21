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

from command import Command
from drivers.pulseaudio import Pulseaudio

C_HALT = 'halt'
C_PUT_AUDIO = 'put_audio'

class AudioThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._backend = None
        self._q = Queue.Queue()
        self._driver = Pulseaudio()

    # Driver interface

    def set_backend(self, backend):
        self._backend = backend
        self._driver.set_audio_generator(backend)

    def put_audio(self, audio):
        self._q.put(Command(C_PUT_AUDIO, audio))

    # Threading interface

    def halt(self):
        self._q.put(Command(C_HALT, None))

    def run(self):
        self._driver.start()
        cmd = self._q.get()
        while cmd.name != C_HALT:
            if cmd.name == C_PUT_AUDIO:
                self._driver.put_audio(cmd.arg)
            else:
                assert False
            cmd = self._q.get()


