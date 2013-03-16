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


class AudioThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._cp = None
        self._q = Queue.Queue()
        self._driver = Pulseaudio()

    def set_command_processor(self, cp):
        self._cp = cp

    def process_audio(self, cmd):
        self._q.put(cmd)

    def halt(self):
        self.process_audio(Command('halt', None))

    def _get_audio(self, nframes):
        self._cp(Command('generate', nframes))
        cmd = self._q.get()
        if cmd.name == 'halt':
            return None
        return cmd.arg

    def run(self):
        self._driver.set_audio_generator(self._get_audio)
        self._driver.start()


