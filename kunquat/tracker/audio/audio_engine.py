# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2013-2016
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

from kunquat.kunquat.kunquat import Kunquat
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


def gen_sine(rate):
    # we yield some silence here to comply with tests
    # this code is probably removed later anyway
    yield 0
    yield 0

    phase = 0
    while True:
        phase += 440 * 2 * math.pi / rate
        phase %= 2 * math.pi
        yield math.sin(phase) * 0.3


class AudioEngine():

    def __init__(self, chunk_size):
        self._audio_output = None
        self._rendering_engine = None
        self._push_time = None
        self._ui_engine = None
        self._nframes = chunk_size
        self._silence = ([0] * self._nframes, [0] * self._nframes)
        self._render_times = deque([], 20)
        self._output_times = deque([], 20)
        self._post_actions = deque()

        self._sine = gen_sine(48000)

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
        for ch in audio_data:
            max_level = max(abs(item) for item in ch)
            levels.append(max_level)
        assert len(levels) == 2
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
        elif event_type == 'Arow':
            row = event_value
            self._ui_engine.update_playback_cursor(row)
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
        start = time.time()
        #data_mono = list(islice(self._sine, nframes))
        #audio_data = (data_mono, data_mono)
        self._rendering_engine.play(nframes)
        audio_data = self._rendering_engine.get_audio()
        event_data = self._rendering_engine.receive_events()
        self._process_events(event_data, CONTEXT_MIX)
        self._process_post_actions()
        (l,r) = audio_data
        if len(l) < 1:
            # TODO: clarify intent here
            audio_data = self._silence
        end = time.time()
        self._render_times.append((nframes, start, end))
        self._audio_levels = self._get_audio_levels(audio_data)
        return audio_data

    def _generate_audio(self, nframes):
        audio_data = self._mix(nframes)
        self._push_amount = nframes
        self._audio_output.put_audio(audio_data)

    def _fire_event(self, channel, event, context):
        self._rendering_engine.fire_event(channel, event)
        event_data = self._rendering_engine.receive_events()
        self._process_events(event_data, context)

    def fire_event(self, channel, event):
        self._fire_event(channel, event, CONTEXT_FIRE)

    def tfire_event(self, channel, event):
        self._fire_event(channel, event, CONTEXT_TFIRE)

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
        for (key, value) in sorted(transaction.items()):
            self._rendering_engine.set_data(key, value)
        self._rendering_engine.validate()
        self._ui_engine.confirm_valid_data(transaction_id)

    def nanoseconds(self, nanos):
        self._rendering_engine.nanoseconds = nanos

    def reset_and_pause(self):
        self._rendering_engine.nanoseconds = 0
        pause_event = ('cpause', None)
        self.tfire_event(0, pause_event)

    def sync_call_post_action(self, action_name, args):
        self._post_actions.append((action_name, args))

    def _average_time(self, times):
        total = sum(end - start for _, start, end in times)
        frames = sum(nframes for nframes, _, _ in times)
        if total == 0:
            return 0
        else:
            return frames / total

    def acknowledge_audio(self):
        end = time.time()
        start = self._push_time
        nframes = self._push_amount
        self._output_times.append((nframes, start, end))
        self._output_fps = math.floor((nframes / (end - start)))
        output_avg = int(self._average_time(self._output_times))
        render_avg = int(self._average_time(self._render_times))
        if render_avg == 0:
            ratio = 0
        else:
            ratio = float(output_avg) / float(render_avg)
            ratio = min(max(0.0, ratio), 1.0)
        if self._ui_engine:
            self._ui_engine.update_output_speed(output_avg)
            self._ui_engine.update_render_speed(render_avg)
            self._ui_engine.update_render_load(ratio)
            self._ui_engine.update_audio_levels(self._audio_levels)

    def produce_sound(self):
        self._push_time = time.time()
        self._generate_audio(self._nframes)

    def close_device(self):
        pass


def create_audio_engine():
    latency = cmdline.get_audio_latency() * 0.001
    rendering_engine = Kunquat()
    audio_rate = rendering_engine.audio_rate
    chunk_size = max(1, int(latency * audio_rate * 0.5))
    audio_engine = AudioEngine(chunk_size)
    audio_output = Pushaudio(latency)
    audio_output.set_audio_source(audio_engine)
    audio_output.start()
    audio_engine.set_rendering_engine(rendering_engine)
    audio_engine.set_audio_output(audio_output)
    return audio_engine


