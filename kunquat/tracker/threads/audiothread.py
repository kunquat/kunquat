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

HALT = None

class AudioThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._backend = None
        self._q = CommandQueue()
        self._audio = None

    # Driver interface

    def set_backend(self, backend):
        self._q.push('set_backend', backend)

    def set_frontend(self, frontend):
        self._q.push('set_frontend', frontend)

    def select_driver(self, name):
        self._q.push('select_driver', name)

    def put_audio(self, audio_data):
        self._q.push('put_audio', audio_data)

    def request_update(self):
        self._q.push('request_update')

    # Threading interface

    def set_handler(self, audio):
        self._audio = audio

    def halt(self):
        self._q.push(HALT)

    def run(self):
        command = self._q.get()
        while command.name != HALT:
            self._audio.__call__(command.name, *command.args)
            command = self._q.get()


