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

import sys
import json
import time
from collections import deque
from itertools import islice
import tarfile
import math
from kunquat import Kunquat


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


def remove_prefix(path, prefix):
    preparts = prefix.split('/')
    keyparts = path.split('/')
    for pp in preparts:
        kp = keyparts.pop(0)
        if pp != kp:
             return None
    return '/'.join(keyparts)


class Backend():

    def __init__(self):
        self._audio_output = None
        self._frontend = None
        self._kunquat = Kunquat()
        self._render_times = deque([], 20)
        self._output_times = deque([], 20)
        self._push_time = None
        self._push_amount = None
        self._nframes = 2048
        self._silence = ([0] * self._nframes, [0] * self._nframes)

        self._sine = gen_sine(48000)

    def set_audio_output(self, audio_output):
        self._audio_output = audio_output

    def set_frontend(self, frontend):
        self._frontend = frontend

    def set_data(self, key, value):
        self._kunquat.set_data(key, value)

    def load_module(self):
        if len(sys.argv) > 1 and sys.argv[1][-4:] in ['.kqt', '.bz2']:
            path = sys.argv[1]
            prefix = 'kqtc00'
            tfile = tarfile.open(path, format=tarfile.USTAR_FORMAT)
            members = tfile.getmembers()
            member_count = len(members)
            if self._frontend:
                self._frontend.update_progress(0, member_count)
            for i, entry in zip(range(member_count), members):
                tarpath = entry.name
                key = remove_prefix(tarpath, prefix)
                assert (key != None) #TODO broken file exception
                if entry.isfile():
                    value = tfile.extractfile(entry).read()
                    if key.endswith('.json'):
                        decoded = json.loads(value)
                    elif key.endswith('.jsone'):
                        decoded = json.loads(value)
                    elif key.endswith('.jsonf'):
                        decoded = json.loads(value)
                    elif key.endswith('.jsoni'):
                        decoded = json.loads(value)
                    elif key.endswith('.jsonln'):
                        decoded = json.loads(value)
                    elif key.endswith('.jsonsh'):
                        decoded = json.loads(value)
                    elif key.endswith('.jsonsm'):
                        decoded = json.loads(value)
                    else:
                        decoded = value
                    self._kunquat.set_data(key, decoded)
                if self._frontend:
                    self._frontend.update_progress(i + 1, member_count)
            tfile.close()
            self._kunquat.validate()

    def commit_data(self):
        self._kunquat.validate()

    def _mix(self, nframes):
        start = time.time()
        #data_mono = list(islice(self._sine, nframes))
        #audio_data = (data_mono, data_mono)
        audio_data = self._kunquat.mix(nframes)
        (l,r) = audio_data
        if len(l) < 1:
            audio_data = self._silence
        end = time.time()
        self._render_times.append((nframes, start, end))
        self._render_fps = math.floor((nframes / (end - start)))
        return audio_data

    def _generate_audio(self, nframes):
        audio_data = self._mix(nframes)
        self._push_amount = nframes
        self._audio_output.put_audio(audio_data)

    def _next_audio(self):
        self._push_time = time.time()
        self._generate_audio(self._nframes)

    def update_selected_driver(self, name):
        self._next_audio()

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
        alert = '!' if render_avg < output_avg else ''
        print 'output: %s fps\trender: %s fps\t%s' % (output_avg, render_avg, alert)
        self._next_audio()

