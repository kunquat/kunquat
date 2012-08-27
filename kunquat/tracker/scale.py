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

    knotes = [
    [(1,1), (3,1), None , (6,1), (8,1), (10,1), None , (1,2), (3,2)],
    [(0,1), (2,1), (4,1), (5,1), (7,1), (9,1), (11,1), (0,2), (2,2), (4,2)],
    [None , (1,0), (3,0), None , (6,0), (8,0), (10,0)],
    [(0,0), (2,0), (4,0), (5,0), (7,0), (9,0), (11,0)]
    ]

    buttons = {
    (0,  0): {'color': 'dark', 'name': u'C#', 'enabled': True },
    (0,  1): {'color': 'dark', 'name': u'D#', 'enabled': True },
    (0,  2): {'color': 'gray', 'name': u''  , 'enabled': False},
    (0,  3): {'color': 'dark', 'name': u'F#', 'enabled': True },
    (0,  4): {'color': 'dark', 'name': u'G#', 'enabled': True },
    (0,  5): {'color': 'dark', 'name': u'A#', 'enabled': True },
    (0,  6): {'color': 'gray', 'name': u''  , 'enabled': False},
    (0,  7): {'color': 'dark', 'name': u'C#', 'enabled': True },
    (0,  8): {'color': 'dark', 'name': u'D#', 'enabled': True },
    (0,  9): {'color': 'gray', 'name': u''  , 'enabled': False},
    (0, 10): {'color': 'gray', 'name': u'#F', 'enabled': True },

    (1,  0): {'color': 'light', 'name': u'C', 'enabled': True },
    (1,  1): {'color': 'light', 'name': u'D', 'enabled': True },
    (1,  2): {'color': 'light', 'name': u'E', 'enabled': True },
    (1,  3): {'color': 'light', 'name': u'F', 'enabled': True },
    (1,  4): {'color': 'light', 'name': u'G', 'enabled': True },
    (1,  5): {'color': 'light', 'name': u'A', 'enabled': True },
    (1,  6): {'color': 'light', 'name': u'B', 'enabled': True },
    (1,  7): {'color': 'light', 'name': u'C', 'enabled': True },
    (1,  8): {'color': 'light', 'name': u'D', 'enabled': True },
    (1,  9): {'color': 'light', 'name': u'E', 'enabled': True },
    (1, 10): {'color': 'light', 'name': u'F', 'enabled': True },

    (2,  0): {'color': 'gray', 'name': u''  , 'enabled': False},
    (2,  1): {'color': 'dark', 'name': u'C#', 'enabled': True },
    (2,  2): {'color': 'dark', 'name': u'D#', 'enabled': True },
    (2,  3): {'color': 'gray', 'name': u''  , 'enabled': False},
    (2,  4): {'color': 'dark', 'name': u'F#', 'enabled': True },
    (2,  5): {'color': 'dark', 'name': u'G#', 'enabled': True },
    (2,  6): {'color': 'dark', 'name': u'A#', 'enabled': True },
    (2,  7): {'color': 'gray', 'name': u''  , 'enabled': False},
    (2,  8): {'color': 'dark', 'name': u'C#', 'enabled': True },
    (2,  9): {'color': 'dark', 'name': u'D#', 'enabled': True },
    (2, 10): {'color': 'gray', 'name': u''  , 'enabled': False},

    (3,  0): {'color': 'light', 'name': u'C', 'enabled': True },
    (3,  1): {'color': 'light', 'name': u'D', 'enabled': True },
    (3,  2): {'color': 'light', 'name': u'E', 'enabled': True },
    (3,  3): {'color': 'light', 'name': u'F', 'enabled': True },
    (3,  4): {'color': 'light', 'name': u'G', 'enabled': True },
    (3,  5): {'color': 'light', 'name': u'A', 'enabled': True },
    (3,  6): {'color': 'light', 'name': u'B', 'enabled': True },
    (3,  7): {'color': 'light', 'name': u'C', 'enabled': True },
    (3,  8): {'color': 'light', 'name': u'D', 'enabled': True },
    (3,  9): {'color': 'light', 'name': u'E', 'enabled': True },
    (3, 10): {'color': 'light', 'name': u'F', 'enabled': True },
    }

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


