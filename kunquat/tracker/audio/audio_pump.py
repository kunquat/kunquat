# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from Queue import Queue
import threading

HALT = None

class AudioPump(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._audio_source = None
        self._buffer = Queue()
        self._write_method = None

    def __del__(self):
        if self.is_alive():
            self.halt()
            self.join()

    def set_audio_source(self, audio_source):
        self._audio_source = audio_source

    def set_write_method(self, write_method):
        self._write_method = write_method

    def put_audio(self, audio_data):
        self._buffer.put(audio_data)

    def _pump(self):
        nframes = 2048
        self._audio_source.generate_audio(nframes)
        audio_data = self._buffer.get()
        self._write_method(audio_data)

    def halt(self):
        self._buffer.put(HALT)

    def run(self):
        audio_data = self._buffer.get()
        while audio_data != HALT:
            self._pump()
            audio_data = self._buffer.get()


