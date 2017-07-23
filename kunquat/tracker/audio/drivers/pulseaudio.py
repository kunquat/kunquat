# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2016
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
import time
import queue

from kunquat.extras.pulseaudio_async import Async

def split_list_at(lst, i):
    """
    >>> split_list_at([1, 2, 3], 1)
    ([1], [2,]]] 3])
    """
    first = lst[:i]
    second = lst[i:]
    return (first, second)

def split_audio_at(audio, i):
    """
    >>> split_audio_at(([1, 2, 3], [4, 5, 6]), 1)
    (([1], [4]), ([2, 3], [5, 6]))
    """
    (left, right) = audio
    (left1, left2) = split_list_at(left, i)
    (right1, right2) = split_list_at(right, i)
    audio1 = (left1, right1)
    audio2 = (left2, right2)
    return (audio1, audio2)

def join_audio(audio1, audio2):
    """
    >>> join_audio(([1], [4]), ([2, 3], [5, 6]))
    ([1, 2, 3], [4, 5, 6])
    """
    (left1, right1) = audio1
    (left2, right2) = audio2
    left = left1 + left2
    right = right1 + right2
    audio = (left, right)
    return audio

def audio_len(audio):
    """
    >>> audio_len(([1, 2, 3], [4, 5, 6]))
    3
    """
    (left, right) = audio
    assert len(left) == len(right)
    return len(left)

class Pulseaudio():

    def __init__(self):
        self._started = False
        self._audio_source = None
        self._pa = Async(
                'Kunquat Tracker',
                'Editor output',
                self._pa_callback)
        self._buffer = queue.Queue()
        self._acks = queue.Queue()
        self._workspace = ([],[])
        self._pa.init()

    def set_audio_source(self, audio_source):
        self._audio_source = audio_source

    def put_audio(self, audio):
        assert self._started
        self._buffer.put(audio)
        self._acks.get()
        self._audio_source.acknowledge_audio()

    def _add_audio_to_workspace(self, audio):
        self._workspace = join_audio(self._workspace, audio)

    def _flush_queue(self):
        items = self._buffer.qsize()
        for _ in range(items):
            fresh_audio = self._buffer.get()
            self._acks.put('ack')
            self._add_audio_to_workspace(fresh_audio)

    def _update_workspace(self, nframes):
        self._flush_queue()
        missing = nframes - audio_len(self._workspace)
        while missing > 0:
            try:
                fresh_audio = self._buffer.get(True, 1.0)
                self._acks.put('ack')
            except queue.Empty:
                fresh_audio = ([0.0] * missing, [0.0] * missing)
            self._add_audio_to_workspace(fresh_audio)
            missing = nframes - audio_len(self._workspace)

    def _read_workspace(self, nframes):
        frames = audio_len(self._workspace)
        if frames > nframes:
            (audio_data, remainder) = split_audio_at(self._workspace, nframes)
        elif frames == nframes:
            audio_data = self._workspace
            remainder = ([],[])
        else:
            assert False
        self._workspace = remainder
        return audio_data

    def _pa_callback(self, nframes):
        self._update_workspace(nframes)
        audio_data = self._read_workspace(nframes)
        return audio_data

    def start(self):
        self._started = True
        self._pa.play()

    def stop(self):
        self._pa.stop()

    def close(self):
        self.stop()
        self._pa.deinit()

    @classmethod
    def get_id(cls):
        return 'pulseaudio'

    #'PulseAudio asynchronic pull driver'


