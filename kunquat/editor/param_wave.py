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
from collections import defaultdict
import math

from PyQt4 import QtGui, QtCore

from waveform import Waveform


def sine(x):
    x = math.fmod(x, 1)
    return math.sin(x * 2 * math.pi)


def triangle(x):
    x = math.fmod(x, 1)
    if x < 0.25:
        return x * 4
    elif x > 0.75:
        return (x - 1) * 4
    return (0.5 - x) * 4


def pulse(x):
    x = math.fmod(x, 1)
    if x < 0.5:
        return 1
    return -1


def saw(x):
    x = math.fmod(x, 1)
    if x < 0.5:
        return x * 2
    return x * 2 - 2


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

        base_options = [('Sine', sine),
                        ('Triangle', triangle),
                        ('Pulse', pulse),
                        ('Saw', saw),
                       ]
        self._base_funcs = dict(base_options)
        self._base_select = BaseSelect([o[0] for o in base_options])
        self._waveform = Waveform()
        layout.addWidget(self._base_select)
        layout.addWidget(self._waveform)
        self.set_constraints({
                                'length': length,
                                'range': val_range,
                             })
        self._lock_update = False
        QtCore.QObject.connect(self._base_select,
                               QtCore.SIGNAL('currentIndexChanged(QString)'),
                               self._set_base)
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
        self._set_aux_key()
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

    def _set_aux_key(self):
        assert self._key
        components = self._key.split('/')
        last = components[-1]
        assert last.startswith('p_')
        components[-1] = 'i_' + last[2:last.index('.')] + '.json'
        self._aux_key = '/'.join(components)
        aux_data = self._project[self._aux_key]
        self._base = []
        try:
            self._base_option = str(aux_data['base_option'])
        except (KeyError, TypeError, ValueError):
            self._base_option = 'Custom'
        self._base_select.set_selection(self._base_option)
        try:
            self._stretch = float(aux_data['stretch'])
            if abs(self._stretch) > 1:
                raise ValueError
        except (KeyError, TypeError, ValueError):
            self._stretch = 0

    def _set_base(self, base):
        base = str(base)
        if base in self._base_funcs:
            func = self._base_funcs[base]
        else:
            func = lambda x: 0
        self._base = [func(x / self._length) for x in xrange(self._length)]
        self._base_option = base
        self._set_wave()

    def _set_wave(self):
        waveform = self._base
        self._project[self._aux_key] = {
                                           'base_option': self._base_option,
                                           'stretch': self._stretch,
                                       }
        self._project[self._key] = waveform
        self._waveform.set_data(waveform)


class BaseSelect(QtGui.QComboBox):

    def __init__(self, base_options, parent=None):
        QtGui.QComboBox.__init__(self, parent)
        self._custom = False
        self._base_waves = base_options
        for option in self._base_waves:
            self.addItem(option)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('currentIndexChanged(QString)'),
                               self._changed)

    def set_selection(self, text):
        self.blockSignals(True)
        if text in self._base_waves:
            self.setCurrentIndex(self.findText(text))
        else:
            if not self._custom:
                self.insertItem(0, 'Custom')
                self.setCurrentIndex(0)
                self._custom = True
        self.blockSignals(False)

    def _changed(self, text):
        assert text != 'Custom'
        if self._custom:
            assert self.itemText(0) == 'Custom'
            self.blockSignals(True)
            self.removeItem(0)
            self.blockSignals(False)
            self._custom = False


