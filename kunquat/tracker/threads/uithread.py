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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from kunquat.tracker.ui.ui_launcher import create_ui_launcher

from command import Command
from commandqueue import CommandQueue
from qeventpump import QEventPump


HALT = None

class UiThread(threading.Thread):

    process_queue = pyqtSignal(name='process_queue')

    def __init__(self):
        threading.Thread.__init__(self, name="Ui")
        self._q = CommandQueue()
        self._ui_launcher = None
        self._controller = None
        self._pump = None

    # Mystery interface

    def set_ui_launcher(self, ui_launcher):
        self._controller = ui_launcher.get_controller()
        self._ui_launcher = ui_launcher
        self._ui_launcher.set_queue_processor(self._process_queue, self._q.block)

    # Ui Engine interface

    def set_audio_engine(self, audio_engine):
        self._ui_launcher.set_audio_engine(audio_engine)

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

    def halt(self):
        self._q.push(HALT)

    # _create_event_pump needs to be called after
    # the main Qt stuff has first been inialized
    # otherwise it will take over the main thread
    def _start_event_pump(self):
        self._pump = QEventPump()
        self._pump.set_blocker(self._q.block)
        self._pump.set_signaler(self._signal)
        QObject.connect(
                self._pump,
                SIGNAL('process_queue()'),
                self._process_queue)
        self._pump.start()

    def _process_queue(self):
        command = self._q.get()
        if command.name == HALT:
            self._ui_launcher.halt_ui()
        else:
            getattr(self._controller, command.name)(*command.args)

    def _signal(self):
        QObject.emit(self._pump, SIGNAL('process_queue()'))
        pass

    def run(self):
        self._ui_launcher.set_event_pump_starter(self._start_event_pump)
        self._ui_launcher.run_ui()

def create_ui_thread():
    ui_launcher = create_ui_launcher()
    ui_thread = UiThread()
    ui_thread.set_ui_launcher(ui_launcher)
    return ui_thread

