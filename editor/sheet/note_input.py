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

from PyQt4 import QtCore


class NoteInput(object):

    def __init__(self):
        self.base_octave = 4
        self.keys = {
                QtCore.Qt.Key_Z: (0, 0),
                QtCore.Qt.Key_S: (1, 0),
                QtCore.Qt.Key_X: (2, 0),
                QtCore.Qt.Key_D: (3, 0),
                QtCore.Qt.Key_C: (4, 0),
                QtCore.Qt.Key_V: (5, 0),
                QtCore.Qt.Key_G: (6, 0),
                QtCore.Qt.Key_B: (7, 0),
                QtCore.Qt.Key_H: (8, 0),
                QtCore.Qt.Key_N: (9, 0),
                QtCore.Qt.Key_J: (10, 0),
                QtCore.Qt.Key_M: (11, 0),
                QtCore.Qt.Key_Q: (0, 1),
                QtCore.Qt.Key_2: (1, 1),
                QtCore.Qt.Key_W: (2, 1),
                QtCore.Qt.Key_3: (3, 1),
                QtCore.Qt.Key_E: (4, 1),
                QtCore.Qt.Key_R: (5, 1),
                QtCore.Qt.Key_5: (6, 1),
                QtCore.Qt.Key_T: (7, 1),
                QtCore.Qt.Key_6: (8, 1),
                QtCore.Qt.Key_Y: (9, 1),
                QtCore.Qt.Key_7: (10, 1),
                QtCore.Qt.Key_U: (11, 1),
                QtCore.Qt.Key_I: (0, 2),
                QtCore.Qt.Key_9: (1, 2),
                QtCore.Qt.Key_O: (2, 2),
                QtCore.Qt.Key_0: (3, 2),
                QtCore.Qt.Key_P: (4, 2),
            }

    def get_note(self, key):
        note, octave = self.keys[key]
        return note, octave + self.base_octave


