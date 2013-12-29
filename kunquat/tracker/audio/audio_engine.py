# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013
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

from drivers.pushaudio import Pushaudio

EVENT_SELECT_CONTROL = '.i'
EVENT_NOTE_ON = 'n+'
EVENT_NOTE_OFF = 'n-'

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

    def __init__(self):
        self._audio_output = None
        self._rendering_engine = None
        self._push_time = None
        self._ui_engine = None
        self._nframes = 2048
        self._silence = ([0] * self._nframes, [0] * self._nframes)
        self._render_times = deque([], 20)
        self._output_times = deque([], 20)

        self._sine = gen_sine(48000)

    def set_ui_engine(self, ui_engine):
        self._ui_engine = ui_engine

    def set_rendering_engine(self, rendering_engine):
        self._rendering_engine = rendering_engine
        self._rendering_engine.nanoseconds = 0
        self._rendering_engine.fire_event(0, ('Ipause', None))

    def set_audio_output(self, audio_output):
        self._audio_output = audio_output

    def _get_audio_levels(self, audio_data):
        levels = []
        for ch in audio_data:
            max_level = max(abs(item) for item in ch)
            levels.append(max_level)
        assert len(levels) == 2
        return tuple(levels)

    def _process_event(self, channel_number, event_type, event_value):
        if event_type == EVENT_SELECT_CONTROL:
            control_number = event_value
            self._ui_engine.update_selected_control(channel_number, control_number)
        elif event_type == EVENT_NOTE_OFF:
            pass
            self._ui_engine.update_active_note(channel_number, None)
        elif event_type == EVENT_NOTE_ON:
            pitch = event_value
            self._ui_engine.update_active_note(channel_number, pitch)

    def _process_events(self, event_data):
        for channel_number, event in event_data:
            event_type, event_value = tuple(event)
            self._process_event(channel_number, event_type, event_value)

    def _mix(self, nframes):
        start = time.time()
        #data_mono = list(islice(self._sine, nframes))
        #audio_data = (data_mono, data_mono)
        self._rendering_engine.play(nframes)
        audio_data = self._rendering_engine.get_audio()
        event_data = self._rendering_engine.receive_events()
        self._process_events(event_data)
        (l,r) = audio_data
        if len(l) < 1:
            audio_data = self._silence
        end = time.time()
        self._render_times.append((nframes, start, end))
        self._render_fps = math.floor((nframes / (end - start)))
        self._audio_levels = self._get_audio_levels(audio_data)
        return audio_data

    def _generate_audio(self, nframes):
        audio_data = self._mix(nframes)
        self._push_amount = nframes
        self._audio_output.put_audio(audio_data)

    def fire_event(self, channel, event):
        self._rendering_engine.fire_event(channel, event)
        event_data = self._rendering_engine.receive_events()
        self._process_events(event_data)

    def set_data(self, transaction):
        #TODO: Remove sorting once it works without
        for (key, value) in sorted(transaction.items()):
            self._rendering_engine.set_data(key, value)
        self._rendering_engine.validate()

    def nanoseconds(self, nanos):
        self._rendering_engine.nanoseconds = nanos

    def _average_time(self, times):
        total = sum(end - start for _, start, end in times)
        frames = sum(nframes for nframes, _, _ in times)
        return frames / total

    def acknowledge_audio(self):
        start = self._push_time
        end = time.time()
        nframes = self._push_amount
        self._output_times.append((nframes, start, end))
        self._output_fps = math.floor((nframes / (end - start)))
        output_avg = int(self._average_time(self._output_times))
        render_avg = int(self._average_time(self._render_times))
        ratio = float(output_avg) / float(render_avg)
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
    rendering_engine = Kunquat()
    audio_engine = AudioEngine()
    audio_output = Pushaudio()
    audio_output.set_audio_source(audio_engine)
    audio_output.start()
    audio_engine.set_rendering_engine(rendering_engine)
    audio_engine.set_audio_output(audio_output)
    return audio_engine

