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
from commandqueue import CommandQueue
from kunquat.tracker.frontend.frontend import Frontend

HALT = None

class FrontendThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._q = CommandQueue()
        self._ui_launcher = None
        self._frontend = None

    # Mystery interface

    def set_ui_launcher(self, ui_launcher):
        self._ui_launcher = ui_launcher
        self._ui_launcher.set_queue_processor(self._process_queue)

    # Frontend interface

    def set_backend(self, backend):
        self._q.push('set_backend', backend)

    def set_audio_output(self, audio_output):
        self._q.push('set_audio_output', audio_output)

    def update_drivers(self, drivers):
        self._q.push('update_drivers', drivers)

    def select_driver_success(self, driver_class):
        self._q.push('select_driver_success', driver_class)

    def update_progress(self, position, last):
        self._q.push('update_progress', position, last)

    # Threading interface

    def set_handler(self, frontend):
        self._frontend = frontend

    def halt(self):
        self._q.push(HALT)

    def _process_queue(self):
        count_estimate = self._q.qsize()
        for _ in range(count_estimate):
            try:
                command = self._q.get_nowait()
                if command.name == HALT:
                    self._ui_launcher.halt_ui()
                else:
                    getattr(self._frontend, command.name)(*command.args)
            except Queue.Empty:
                break

    def run(self):
        self._ui_launcher.run_ui()


