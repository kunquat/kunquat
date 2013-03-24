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
from kunquat.tracker.frontend.frontend import Frontend
from kunquat.tracker.qt.ui import Ui

C_HALT = 'halt'
C_UPDATE_DRIVERS = 'update_drivers'

class FrontendThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        self._q = Queue.Queue()
        self._frontend = Frontend()
        self._ui = None

    # Frontend interface

    def set_backend(self, backend):
        self._frontend.set_backend(backend)

    def set_audio_output(self, audio_output):
        self._frontend.set_audio_output(audio_output)

    def update_drivers(self, drivers):
        self._q.put(Command(C_UPDATE_DRIVERS, drivers))

    # Threading interface

    def halt(self):
        self._q.put(Command(C_HALT, None))

    def _process_queue(self):
        count_estimate = self._q.qsize()
        for _ in range(count_estimate):
            try:
                command = self._q.get_nowait()
                if command.name == C_HALT:
                    self._ui.halt()
                elif command.name == C_UPDATE_DRIVERS:
                    self._frontend.update_drivers(command.arg)
                else:
                    self._frontend.process_command(command)
            except Queue.Empty:
                break

    def run(self):
        self._ui = Ui()
        self._ui.set_frontend(self._frontend)
        self._ui.set_queue_processor(self._process_queue)
        self._ui.start_driver_randomizer()
        self._ui.run()


