# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2013-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import time
import math
from collections import deque
from itertools import islice

from kunquat.kunquat.kunquat import Kunquat, KunquatError, KunquatFormatError
from kunquat.kunquat.file import KqtFile, KQTFILE_KEEP_ALL_DATA
import kunquat.tracker.cmdline as cmdline
import kunquat.tracker.ui.model.tstamp as tstamp

from .drivers.pushaudio import Pushaudio


EVENT_SELECT_CONTROL = '.a'
EVENT_NOTE_ON = 'n+'
EVENT_HIT = 'h'
EVENT_NOTE_OFF = 'n-'

CONTEXT_MIX = 'mix'
CONTEXT_FIRE = 'fire'
CONTEXT_TFIRE = 'tfire'


class AudioEngine():

    def __init__(self, chunk_size):
        self._audio_output = None
        self._rendering_engine = None
        self._cycle_time = None
        self._ui_engine = None
        self._nframes = chunk_size
        self._silence = [0] * (self._nframes * 2)
        self._render_speed = 0
        self._post_actions = deque()
        self._send_voice_info = False

        self._is_handle_ready = True

    def set_ui_engine(self, ui_engine):
        self._ui_engine = ui_engine

    def set_rendering_engine(self, rendering_engine):
        self._rendering_engine = rendering_engine
        self._rendering_engine.nanoseconds = 0
        self._rendering_engine.fire_event(0, ('cpause', None))

    def set_audio_output(self, audio_output):
        self._audio_output = audio_output

    def _get_audio_levels(self, audio_data):
        levels = []

        lslice = islice(audio_data, 0, None, 2)
        rslice = islice(audio_data, 1, None, 2)
        for ch in (lslice, rslice):
            max_level = max(abs(item) for item in ch)
            levels.append(max_level)

        return tuple(levels)

    def _process_event(self, channel_number, event_type, event_value, context):
        self._ui_engine.update_event_log_with(
                channel_number, event_type, event_value, context)

        if event_type == EVENT_SELECT_CONTROL:
            control_number = event_value
            self._ui_engine.update_selected_control(channel_number, control_number)
        elif event_type == EVENT_NOTE_OFF:
            self._ui_engine.update_active_note(channel_number, EVENT_NOTE_OFF, None)
        elif event_type in (EVENT_NOTE_ON, EVENT_HIT):
            pitch = event_value
            self._ui_engine.update_active_note(channel_number, event_type, pitch)
        elif event_type == 'Atrack':
            track = event_value
            self._ui_engine.update_pending_playback_cursor_track(track)
        elif event_type == 'Asystem':
            system = event_value
            self._ui_engine.update_pending_playback_cursor_system(system)
        elif event_type == 'Apattern':
            piref = event_value
            self._ui_engine.update_playback_pattern(piref)
        elif event_type == 'Arow':
            row = event_value
            self._ui_engine.update_playback_cursor(row)
        elif event_type == 'Avoices':
            voice_count = event_value
            self._ui_engine.update_active_voice_count(voice_count)
        elif event_type == 'Avgroups':
            vgroup_count = event_value
            self._ui_engine.update_active_vgroup_count(vgroup_count)
        elif event_type == 'c.evn':
            var_name = event_value
            self._ui_engine.update_active_var_name(channel_number, var_name)
        elif event_type == 'c.ev':
            var_type = tstamp.Tstamp if type(event_value) == list else type(event_value)
            var_value = var_type(event_value)
            self._ui_engine.update_active_var_value(channel_number, var_value)
        elif event_type == '.xc':
            expr_name = event_value
            self._ui_engine.update_ch_expression(channel_number, expr_name)

    def _process_events(self, event_data, context):
        for channel_number, event in event_data:
            event_type, event_value = tuple(event)
            self._process_event(channel_number, event_type, event_value, context)

    def _process_post_actions(self):
        while self._post_actions:
            action_name, args = self._post_actions.popleft()
            self._ui_engine.call_post_action(action_name, args)

    def _mix(self, nframes):
        success = False

        # Try rendering until we get more audio data
        if self._is_handle_ready:
            attempt_count = 16
            for _ in range(attempt_count):
                self._rendering_engine.play(nframes)
                audio_data = self._rendering_engine.get_audio()
                event_data = self._rendering_engine.receive_events()
                self._process_events(event_data, CONTEXT_MIX)
                self._process_post_actions()
                frame_count = len(audio_data) // 2
                if frame_count > 0:
                    success = True
                    self._push_amount = frame_count
                    break

        if not success:
            # We are not getting more audio, possibly due to heavy event spamming
            audio_data = self._silence
            self._push_amount = 0

        self._audio_levels = self._get_audio_levels(audio_data)
        return audio_data

    def _generate_audio(self, nframes):
        start = time.perf_counter()
        audio_data = self._mix(nframes)
        end = time.perf_counter()

        elapsed = end - start
        if elapsed > 0:
            self._render_speed = self._push_amount / elapsed

        self._audio_output.put_audio(audio_data)

    def _fire_event(self, channel, event, context):
        if not self._is_handle_ready:
            return
        self._rendering_engine.fire_event(channel, event)
        event_data = self._rendering_engine.receive_events()
        self._process_events(event_data, context)

    def fire_event(self, channel, event):
        self._fire_event(channel, event, CONTEXT_FIRE)

    def tfire_event(self, channel, event):
        self._fire_event(channel, event, CONTEXT_TFIRE)

    def request_voice_info(self):
        self._send_voice_info = True

    def set_channel_mute(self, channel, mute):
        self._rendering_engine.set_channel_mute(channel, mute)

    def load_module(self, path):
        self._is_handle_ready = False

        try:
            f = KqtFile(self._rendering_engine, KQTFILE_KEEP_ALL_DATA)
            for _ in f.load_steps(path):
                progress = f.get_loading_progress()
                self._ui_engine.update_import_progress(progress * 0.5)

            entries = f.get_kept_entries()
            entry_count = f.get_kept_entry_count()

        except KunquatError as e:
            self._ui_engine.notify_import_error(path, e)
            return

        try:
            self._rendering_engine.validate()
        except KunquatFormatError as e:
            self._ui_engine.notify_import_error(path, e)
            return

        for i, (key, value) in enumerate(entries):
            self._ui_engine.add_imported_entry(key, value)
            self._ui_engine.update_import_progress(0.5 + ((i + 1) / entry_count))

        self._ui_engine.notify_import_finished()

        self._is_handle_ready = True

        del f

    def set_data(self, transaction_id, transaction):
        # This method should never be called directly. Feeding data to the
        # audio engine needs to be done through the store to keep module state
        # synchronised.

        if transaction == None:
            # We received a token instead of actual data
            self._ui_engine.confirm_valid_data(transaction_id)
            return

        #TODO: Remove sorting once it works without
        assert type(transaction) == dict
        step_count = len(transaction) + 1
        for i, (key, value) in enumerate(sorted(transaction.items())):
            self._rendering_engine.set_data(key, value)
            self._ui_engine.update_transaction_progress(transaction_id, i / step_count)
        self._rendering_engine.validate()
        self._ui_engine.confirm_valid_data(transaction_id)
        self._ui_engine.update_transaction_progress(transaction_id, 1)

    def nanoseconds(self, nanos):
        self._rendering_engine.nanoseconds = nanos

    def reset_and_pause(self, track_num):
        self._rendering_engine.track = track_num
        self._rendering_engine.nanoseconds = 0
        pause_event = ('cpause', None)
        self.tfire_event(0, pause_event)

    def sync_call_post_action(self, action_name, args):
        self._post_actions.append((action_name, args))

    def acknowledge_audio(self):
        start = self._cycle_time
        end = time.perf_counter()
        self._cycle_time = end

        if start == None:
            return

        if self._ui_engine:
            output_fps = self._rendering_engine.audio_rate
            self._ui_engine.update_output_speed(output_fps)

            if self._render_speed > 0:
                ratio = (output_fps / self._render_speed)
                self._ui_engine.update_render_speed(self._render_speed)
                self._ui_engine.update_render_load(ratio)

            self._ui_engine.update_audio_levels(self._audio_levels)
            if self._send_voice_info:
                self.tfire_event(0, ('qvoices', None))
                self._send_voice_info = False

    def produce_sound(self):
        self._generate_audio(self._nframes)

    def close_device(self):
        pass


def create_audio_engine():
    latency = cmdline.get_audio_latency() * 0.001
    rendering_engine = Kunquat(audio_rate=cmdline.get_audio_rate())
    rendering_engine.set_loader_thread_count(cmdline.get_default_thread_count())
    rendering_engine.set_player_thread_count(cmdline.get_thread_count())
    audio_rate = rendering_engine.audio_rate
    chunk_size = max(1, int(latency * audio_rate * 0.5))
    audio_engine = AudioEngine(chunk_size)
    audio_output = Pushaudio(audio_rate, latency)
    audio_output.set_audio_source(audio_engine)
    audio_output.start()
    audio_engine.set_rendering_engine(rendering_engine)
    audio_engine.set_audio_output(audio_output)
    return audio_engine


