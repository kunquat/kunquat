# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division
from __future__ import print_function
import fractions as fr
import math

from PyQt4.QtCore import Qt

class Scale(object):

    def __init__(self, data):
        self.knotes = data['knotes']
        self.buttons = data['buttons']
        self.center = data['ref_pitch']
        self.center_cents = math.log(self.center / 440, 2) * 1200
        self.notes = []
        self.name = u'-'
        if 'name' in data.keys():
            self.name = data['name']
        for note, tuning in data['notes']:
            _, cents = self.read_tuning(tuning)
            self.notes.append((note, cents))
        self.octave, self.octave_cents = self.read_tuning(
                                             data['octave_ratio'])
    def get_name(self):
        return self.name

    def note_name(self, note):
        name, _ = self.notes[note]
        return name

    def get_cents(self, note, octave):
        root = self.center_cents
        root += (octave - 5) * self.octave_cents
        return root + self.notes[note][1]

    def get_display_info(self, cents):
        octave_number = math.floor((cents - self.center_cents) / 1200) + 5
        root_offset = (cents - self.center_cents) % 1200
        note_name, note_offset = min(((name, root_offset - x)
                for name, x in self.notes), key=lambda x:
                min(abs(x[1]), self.octave_cents - abs(x[1])))
        if note_offset > self.octave_cents / 2:
            note_offset -= self.octave_cents
            octave_number += 1
        return note_name, int(octave_number), note_offset

    def read_tuning(self, pair):
        desc, value = pair
        if desc == '/':
            frac = fr.Fraction(*value)
            return frac, math.log(frac, 2) * 1200
        elif desc == 'f':
            return value, math.log(value, 2) * 1200
        elif desc == 'c':
            return None, value
        else:
            raise ValueError, "Invalid type description '{0}'".format(desc)


