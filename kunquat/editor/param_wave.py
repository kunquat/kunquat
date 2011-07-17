# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division, print_function

from PyQt4 import QtGui, QtCore

from waveform import Waveform


class ParamWave(QtGui.QWidget):

    def __init__(self,
                 project,
                 val_range,
                 length,
                 key,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)

        self._waveform = Waveform()
        layout.addWidget(self._waveform)
        self.set_constraints({
                                'length': length,
                                'range': val_range,
                             })
        self._lock_update = False
        self.set_key(key)

    def set_constraints(self, constraints):
        try:
            self._length = int(constraints['length'])
            if self._length < 1 or self._length > 2**20:
                raise ValueError
        except (KeyError, TypeError, ValueError):
            self._length = 1

    def set_key(self, key):
        if not key:
            return
        self._key = key
        data = self._project[key]
        if not data or not isinstance(data, list):
            data = [0] * self._length
        else:
            if len(data) < self._length:
                data.extend([0] * (self._length - len(data)))
            elif len(data) > self._length:
                del data[self._length:]
        self._waveform.set_data(data)

    def sync(self):
        self.set_key(self._key)


