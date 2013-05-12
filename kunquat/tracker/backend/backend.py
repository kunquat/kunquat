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
            for i, entry in zip(range(member_count), members):
                if self._frontend:
                    self._frontend.update_progress(i, member_count)
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
            tfile.close()
            self._kunquat.validate()

    def commit_data(self):
        self._kunquat.validate()

    def _generate_audio(self, nframes):
        #data_mono = list(islice(self._sine, nframes))
        #audio_data = (data_mono, data_mono)
        audio_data = self._kunquat.mix(nframes)
        self._audio_output.put_audio(audio_data)

    def update_selected_driver(self, name):
        self._audio_output.put_audio(([],[]))

    def acknowledge_audio(self):
        nframes = 2048
        self._generate_audio(nframes)
