# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from itertools import repeat
from multiprocessing import Lock, Process, Queue, get_start_method
from queue import Empty

import kunquat.tracker.cmdline as cmdline
from kunquat.tracker.ui.uilauncher import create_ui_launcher


HALT = None

def portable_qsize(queue):
    FALLBACK_QSIZE = 2 ** 14
    try:
        return queue.qsize()
    except NotImplementedError:
        return FALLBACK_QSIZE


class CommandQueue():

    _STATE_NORMAL = 'normal'
    _STATE_FLUSHING = 'flushing'
    _STATE_FINISHED = 'finished'

    def __init__(self):
        self._in = Queue()
        self._out = Queue()
        self._state = self._STATE_NORMAL
        self._state_lock = Lock()
        self._terminating_commands = (
                'notify_kunquat_exception', 'notify_libkunquat_error')

    def update(self):
        in_count = portable_qsize(self._in)

        with self._state_lock:
            if self._state == self._STATE_FLUSHING:
                get_counter = repeat(True)
            else:
                get_counter = range(in_count)

        for _ in get_counter:
            try:
                command_data = self._in.get_nowait()
            except Empty:
                return

            command, _ = command_data
            if command in self._terminating_commands:
                # Make sure we won't block the UI before the terminating command is sent
                with self._state_lock:
                    self._state = self._STATE_FLUSHING
            self._out.put(command_data)

    def put(self, command, *args):
        with self._state_lock:
            is_state_normal = (self._state == self._STATE_NORMAL)
        if is_state_normal:
            self._in.put((command, args))

    def get(self):
        command_data = self._out.get_nowait()
        command, _ = command_data
        if command in self._terminating_commands:
            with self._state_lock:
                self._state = self._STATE_FINISHED
        return command_data

    def get_command_count(self):
        return portable_qsize(self._out)


class UiProcess(Process):

    def __init__(self):
        super().__init__(name='Kunquat Tracker UI')
        self._ui_launcher = None
        self._controller = None
        self._audio_engine = None
        self._q = CommandQueue()

    # UI engine access interface

    def set_audio_engine(self, audio_engine):
        self._audio_engine = audio_engine

    def notify_kunquat_exception(self, exception):
        self._q.put('notify_kunquat_exception', exception)

    def notify_libkunquat_error(self, info):
        self._q.put('notify_libkunquat_error', info)

    def update_drivers(self, drivers):
        self._q.put('update_drivers', drivers)

    def update_selected_driver(self, driver_class):
        self._q.put('update_selected_driver', driver_class)

    def update_output_speed(self, fps):
        self._q.put('update_output_speed', fps)

    def update_render_speed(self, fps):
        self._q.put('update_render_speed', fps)

    def update_render_load(self, ratio):
        self._q.put('update_render_load', ratio)

    def update_audio_levels(self, levels):
        self._q.put('update_audio_levels', levels)

    def update_selected_control(self, channel_number, control_number):
        self._q.put('update_selected_control', channel_number, control_number)

    def update_active_note(self, channel_number, event_type, pitch):
        self._q.put('update_active_note', channel_number, event_type, pitch)

    def update_active_var_name(self, channel_number, var_name):
        self._q.put('update_active_var_name', channel_number, var_name)

    def update_active_var_value(self, channel_number, var_value):
        self._q.put('update_active_var_value', channel_number, var_value)

    def update_ch_expression(self, channel_number, expr_name):
        self._q.put('update_ch_expression', channel_number, expr_name)

    def update_pending_playback_cursor_track(self, track):
        self._q.put('update_pending_playback_cursor_track', track)

    def update_pending_playback_cursor_system(self, system):
        self._q.put('update_pending_playback_cursor_system', system)

    def update_playback_pattern(self, piref):
        self._q.put('update_playback_pattern', piref)

    def update_playback_cursor(self, row):
        self._q.put('update_playback_cursor', row)

    def update_active_voice_count(self, voice_count):
        self._q.put('update_active_voice_count', voice_count)

    def update_active_vgroup_count(self, vgroup_count):
        self._q.put('update_active_vgroup_count', vgroup_count)

    def update_event_log_with(self, channel_number, event_type, event_value, context):
        self._q.put('update_event_log_with', channel_number, event_type, event_value, context)

    def update_import_progress(self, progress):
        self._q.put('update_import_progress', progress)

    def add_imported_entry(self, key, value):
        self._q.put('add_imported_entry', key, value)

    def notify_import_error(self, path, error):
        self._q.put('notify_import_error', path, error)

    def notify_import_finished(self):
        self._q.put('notify_import_finished')

    def update_transaction_progress(self, transaction_id, progress):
        self._q.put('update_transaction_progress', transaction_id, progress)

    def confirm_valid_data(self, transaction_id):
        self._q.put('confirm_valid_data', transaction_id)

    def call_post_action(self, action_name, args):
        self._q.put('call_post_action', action_name, args)

    # Process interface

    def halt(self):
        self._q.put(HALT)

    def _process_queue(self):
        self._q.update()
        cmd_count = self._q.get_command_count()
        for _ in range(cmd_count):
            try:
                command, args = self._q.get()
            except Empty:
                return

            if command == HALT:
                self._ui_launcher.halt_ui()
                return
            else:
                getattr(self._controller, command)(*args)

    def run(self):
        if get_start_method() != 'fork':
            # We haven't received the parsed arguments from the main process
            cmdline.parse_arguments()

        # Create the UI inside the correct process
        self._ui_launcher = create_ui_launcher()
        self._ui_launcher.set_audio_engine(self._audio_engine)
        self._controller = self._ui_launcher.get_controller()

        self._ui_launcher.set_event_queue_processor(self._process_queue)
        self._ui_launcher.run_ui()


def create_ui_process():
    ui_process = UiProcess()
    return ui_process


