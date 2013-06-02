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
        threading.Thread.__init__(self, name="Frontend")
        self._q = CommandQueue()
        self._ui_launcher = None
        self._frontend = None

    # Mystery interface

    def set_ui_launcher(self, ui_launcher):
        self._ui_launcher = ui_launcher
        self._ui_launcher.set_queue_processor(self._process_queue, self._q.block)

    # Frontend interface

    def set_backend(self, backend):
        self._frontend.set_backend(backend)

    def set_audio_output(self, audio_output):
        self._frontend.set_audio_output(audio_output)

    def update_drivers(self, drivers):
        self._q.push('update_drivers', drivers)

    def update_selected_driver(self, driver_class):
        self._q.push('update_selected_driver', driver_class)

    def update_import_progress(self, position, steps):
        self._q.push('update_import_progress', position, steps)

    def update_output_speed(self, fps):
        self._q.push('update_output_speed', fps)

    def update_render_speed(self, fps):
        self._q.push('update_render_speed', fps)

    def update_render_load(self, ratio):
        self._q.push('update_render_load', ratio)

    def update_audio_levels(self, levels):
        self._q.push('update_audio_levels', levels)

    def update_selected_instrument(self, channel_number, instrument_number):
        self._q.push('update_selected_instrument', channel_number, instrument_number)

    def update_instrument_existence(self, instrument_number, existence):
        self._q.push('update_instrument_existence', instrument_number, existence)

    def update_instrument_name(self, instrument_number, name):
        self._q.push('update_instrument_name', instrument_number, name)

    def update_active_note(self, channel_number, pitch):
        self._q.push('update_active_note', channel_number, pitch)

    # Threading interface

    def set_handler(self, frontend):
        self._frontend = frontend

    def halt(self):
        self._q.push(HALT)

    def _process_queue(self):
        command = self._q.get()
        if command.name == HALT:
            self._ui_launcher.halt_ui()
        else:
            getattr(self._frontend, command.name)(*command.args)

    def run(self):
        self._ui_launcher.run_ui()


