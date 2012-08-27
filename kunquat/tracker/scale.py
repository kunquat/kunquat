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

    keys = [
    [Qt.Key_2,Qt.Key_3,Qt.Key_4,Qt.Key_5,Qt.Key_6,Qt.Key_7,Qt.Key_8,Qt.Key_9,Qt.Key_0],
    [Qt.Key_Q,Qt.Key_W,Qt.Key_E,Qt.Key_R,Qt.Key_T,Qt.Key_Y,Qt.Key_U,Qt.Key_I,Qt.Key_O,Qt.Key_P],
    [Qt.Key_A,Qt.Key_S,Qt.Key_D,Qt.Key_F,Qt.Key_G,Qt.Key_H,Qt.Key_J],
    [Qt.Key_Z,Qt.Key_X,Qt.Key_C,Qt.Key_V,Qt.Key_B,Qt.Key_N,Qt.Key_M]
    ]

    knotes = [
    [(1,1), (3,1), None , (6,1), (8,1), (10,1), None , (1,2), (3,2)],
    [(0,1), (2,1), (4,1), (5,1), (7,1), (9,1), (11,1), (0,2), (2,2), (4,2)],
    [None , (1,0), (3,0), None , (6,0), (8,0), (10,0)],
    [(0,0), (2,0), (4,0), (5,0), (7,0), (9,0), (11,0)]
    ]


    def __init__(self, data):
        self.center = data['ref_pitch']
        self.center_cents = math.log(self.center / 440, 2) * 1200
        self.notes = []
        for note, tuning in data['notes']:
            _, cents = self.read_tuning(tuning)
            self.notes.append((note, cents))
        self.octave, self.octave_cents = self.read_tuning(
                                             data['octave_ratio'])

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


