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

import threading

from kunquat.tracker.ui.ui_engine import create_ui_engine

from command import Command
from commandqueue import CommandQueue


HALT = None

class UiThread(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self, name="Ui")
        self._q = CommandQueue()
        self._handler = None

    # Mystery interface

    def set_ui_engine(self, handler):
        self._handler = handler
        self._handler.set_queue_processor(self._process_queue, self._q.block)

    # Frontend interface

    def set_audio_engine(self, audio_engine):
        self._handler.set_audio_engine(audio_engine)

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
            self._handler.halt_ui()
        else:
            getattr(self._frontend, command.name)(*command.args)

    def run(self):
        self._handler.run_ui()

def create_ui_thread():
    ui_engine = create_ui_engine()
    ui_thread = UiThread()
    ui_thread.set_ui_engine(ui_engine)
    return ui_thread

