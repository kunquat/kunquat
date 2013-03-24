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
import Queue
import threading

from command import Command
from commandqueue import CommandQueue
from kunquat.tracker.frontend.frontend import Frontend

C_HALT = 'halt'
C_UPDATE_DRIVERS = 'update_drivers'

class FrontendThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._q = CommandQueue()
        self._frontend = Frontend()
        self._ui_launcher = None

    # Frontend interface

    def set_backend(self, backend):
        self._frontend.set_backend(backend)

    def set_audio_output(self, audio_output):
        self._frontend.set_audio_output(audio_output)

    def update_drivers(self, drivers):
        arg = json.dumps(drivers)
        self._q.put(Command(C_UPDATE_DRIVERS, arg))

    # Threading interface

    def set_ui_launcher(self, ui_launcher):
        self._ui_launcher = ui_launcher
        self._ui_launcher.set_frontend(self._frontend)
        self._ui_launcher.set_queue_processor(self._process_queue)

    def halt(self):
        self._q.put(Command(C_HALT, None))

    def _process_queue(self):
        count_estimate = self._q.qsize()
        for _ in range(count_estimate):
            try:
                command = self._q.get_nowait()
                if command.name == C_HALT:
                    self._ui_launcher.halt_ui()
                elif command.name == C_UPDATE_DRIVERS:
                    drivers = json.loads(command.arg)
                    self._frontend.update_drivers(drivers)
                else:
                    self._frontend.process_command(command)
            except Queue.Empty:
                break

    def run(self):
        self._ui_launcher.run_ui()


