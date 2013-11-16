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

import time
import threading

from kunquat.tracker.audio.audio_engine import create_audio_engine

from command import Command
from commandqueue import CommandQueue

HALT = None

class AudioThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self, name="Audio")
        self._ui_engine = None
        self._engine = None
        self._q = CommandQueue()

    def _close_device(self):
        if self._engine:
            self._engine.close_device()
            time.sleep(0.1)

    def set_ui_engine(self, ui_engine):
        self._ui_engine = ui_engine

    def set_handler(self, engine):
        self._engine = engine

    # Libkunquat interface

    def set_data(self, key, value):
        self._q.push('set_data', key, value)

    # Thread interface

    def halt(self):
        self._q.push(HALT)

    def run(self):
        self._q.block()
        command = self._q.get()
        while command.name != HALT:
            getattr(self._audio, command.name)(*command.args)
            self._q.block()
            command = self._q.get()
        self._close_device()
    
def create_audio_thread():
    audio_engine = create_audio_engine()
    audio_thread = AudioThread()
    audio_thread.set_handler(audio_engine)
    return audio_thread

