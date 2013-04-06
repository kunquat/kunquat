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

from kunquat.extras.pulseaudio import Simple

HALT = None

class AudioPump(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._audio_source = None
        self._buffer = Queue()
        self._pa = Simple('Kunquat Tracker',
                          'Editor output')

    def set_audio_source(self, audio_source):
        self._audio_source = audio_source

    def put_audio(self, audio_data):
        self._buffer.put(audio_data)

    def _pump(self):
        nframes = 2048
        self._audio_source.generate_audio(nframes)
        audio_data = self._buffer.get()
        (left, right) = audio_data
        if len(left) > 0:
            self._pa.write(left, right)

    def halt(self):
        self._buffer.put(HALT)

    def run(self):
        audio_data = self._buffer.get()
        while audio_data != HALT:
            self._pump()
            audio_data = self._buffer.get()
        del self._pa


class Pushaudio():

    def __init__(self):
        self._audio_pump = AudioPump()

    def __del__(self):
        if self._audio_pump.is_alive():
            self.stop()

    def set_audio_source(self, audio_source):
        self._audio_pump.set_audio_source(audio_source)

    def put_audio(self, audio_data):
        self._audio_pump.put_audio(audio_data)

    def start(self):
        self._audio_pump.start()

    def stop(self):
        self._audio_pump.halt()
        self._audio_pump.join()

    def close(self):
        self.stop()


