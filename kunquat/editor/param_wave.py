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
from itertools import count, izip
import math

from PyQt4 import QtGui, QtCore

from param_slider import KSlider
from waveform import Waveform


def normalise(x):
    return (x + 1) % 2 - 1


def sine(x):
    return math.sin(x * math.pi)


def triangle(x):
    if x < -0.5:
        return -x * 2 - 2
    elif x > 0.5:
        return -x * 2 + 2
    return x * 2


def pulse(x):
    if x < 0:
        return -1
    return 1


def saw(x):
    return x


def identity(x, amount):
    return x


def shift(x, amount):
    return x + amount


def stretch(x, amount):
    amount *= 2
    if x < 0:
        return -((-x)**(4**(amount)))
    return x**(4**(amount))


def stretch_asym(x, amount):
    amount *= 2
    x = (x + 1) / 2
    x = x**(4**(amount))
    return x * 2 - 1


def scale(x, amount):
    return x * 8**amount


def sine_shift(x, amount):
    return math.sin(x * 6**(amount + 0.5))


def mod_y(y):
    y += 1
    if y > 2 or y < 0:
        y -= 2 * math.floor(y / 2)
    return y - 1


def clip(y, amount):
    y *= 8**amount
    return mod_y(y)


def shift_y(y, amount):
    return mod_y(y + amount)


def stretch_y(y, amount):
    return stretch(y, amount * 2)


def stretch_y_asym(y, amount):
    return stretch_asym(y, amount * 2)


def quantise(y, amount):
    amount = 2**(-(amount - 1.3) * 4)
    y *= amount - 1
    y = math.floor(y) + 0.5
    return y / amount


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

        prewarp_options = [('None', identity),
                           ('Scale', scale),
                           ('Shift', shift),
                           ('Sine', sine_shift),
                           ('Stretch', stretch),
                           ('Stretch asym', stretch_asym),
                          ]
        self._prewarp_funcs = dict(prewarp_options)
        self._prewarp_count = 4
        self._prewarp_select = []
        for i in xrange(self._prewarp_count):
            self._prewarp_select.extend([WarpSelect([o[0] for o in
                                         prewarp_options], i)])
        self._prewarp_options = [identity] * self._prewarp_count
        self._prewarp_params = [0] * self._prewarp_count

        base_options = [('Sine', sine),
                        ('Triangle', triangle),
                        ('Pulse', pulse),
                        ('Saw', saw),
                       ]
        self._base_funcs = dict(base_options)
        self._base_select = BaseSelect([o[0] for o in base_options])

        postwarp_options = [('None', identity),
                            ('Clip', clip),
                            ('Quantise', quantise),
                            ('Shift', shift_y),
                            ('Sine', sine_shift),
                            ('Stretch', stretch_y),
                            ('Stretch asym', stretch_y_asym),
                           ]
        self._postwarp_funcs = dict(postwarp_options)
        self._postwarp_count = 1
        self._postwarp_select = []
        for i in xrange(self._postwarp_count):
            self._postwarp_select.extend([WarpSelect([o[0] for o in
                                          postwarp_options], i)])
        self._postwarp_options = [identity] * self._postwarp_count
        self._postwarp_params = [0] * self._postwarp_count

        self._waveform = Waveform()
        for ps in self._prewarp_select:
            layout.addWidget(ps)
        layout.addWidget(self._base_select)
        for ps in self._postwarp_select:
            layout.addWidget(ps)
        layout.addWidget(self._waveform, 1)
        self.set_constraints({
                                'length': length,
                                'range': val_range,
                             })
        self._lock_update = False
        for ps in self._prewarp_select:
            QtCore.QObject.connect(ps,
                                   QtCore.SIGNAL('funcChanged(int, QString)'),
                                   self._set_prewarp)
            QtCore.QObject.connect(ps,
                                   QtCore.SIGNAL('paramChanged(int, float)'),
                                   self._prewarp_param_changed)
            QtCore.QObject.connect(ps,
                                   QtCore.SIGNAL('paramFinished(int)'),
                                   self._prewarp_param_finished)
        QtCore.QObject.connect(self._base_select,
                               QtCore.SIGNAL('currentIndexChanged(QString)'),
                               self._set_base)
        for ps in self._postwarp_select:
            QtCore.QObject.connect(ps,
                                   QtCore.SIGNAL('funcChanged(int, QString)'),
                                   self._set_postwarp)
            QtCore.QObject.connect(ps,
                                   QtCore.SIGNAL('paramChanged(int, float)'),
                                   self._postwarp_param_changed)
            QtCore.QObject.connect(ps,
                                   QtCore.SIGNAL('paramFinished(int)'),
                                   self._postwarp_param_finished)
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

        try:
            self._base_option = str(aux_data['base_option'])
        except (KeyError, TypeError, ValueError):
            self._base_option = 'Custom'
        self._base_select.set_selection(self._base_option)

        try:
            preopts = []
            pw = aux_data['prewarps']
            for i in xrange(min(len(self._prewarp_options), len(pw))):
                name = str(pw[i][0])
                if name not in self._prewarp_funcs:
                    name = 'None'
                value = float(pw[i][1])
                if abs(value) > 1:
                    raise ValueError
                preopts.extend([(name, value)])
        except (KeyError, TypeError, ValueError):
            preopts = []
        preopts.extend([('None', 0)] *
                       (len(self._prewarp_options) - len(preopts)))
        prewarps = [(self._prewarp_funcs[n[0]], n[1]) for n in preopts]
        options, params = zip(*prewarps)
        self._prewarp_options = list(options)
        self._prewarp_params = list(params)
        for ps, prewarp in izip(self._prewarp_select, preopts):
            ps.set_state(*prewarp)
        for ps in self._prewarp_select:
            ps.setEnabled(self._base_option != 'Custom')

        try:
            postopts = []
            pw = aux_data['postwarps']
            for i in xrange(min(len(self._postwarp_options), len(pw))):
                name = str(pw[i][0])
                if name not in self._postwarp_funcs:
                    name = 'None'
                value = float(pw[i][1])
                if abs(value) > 1:
                    raise ValueError
                postopts.extend([(name, value)])
        except (KeyError, TypeError, ValueError):
            postopts = []
        postopts.extend([('None', 0)] *
                        (len(self._postwarp_options) - len(postopts)))
        postwarps = [(self._postwarp_funcs[n[0]], n[1]) for n in postopts]
        options, params = zip(*postwarps)
        self._postwarp_options = list(options)
        self._postwarp_params = list(params)
        for ps, postwarp in izip(self._postwarp_select, postopts):
            ps.set_state(*postwarp)

    def _set_prewarp(self, ident, prewarp):
        prewarp = str(prewarp)
        if prewarp in self._prewarp_funcs:
            func = self._prewarp_funcs[prewarp]
        else:
            func = identity
        self._prewarp_options[ident] = func
        self._set_wave()

    def _prewarp_param_changed(self, ident, value):
        self._prewarp_params[ident] = value
        self._set_wave(False)

    def _prewarp_param_finished(self, ident):
        self._project.flush(self._key)
        self._project.flush(self._aux_key)

    def _set_base(self, base):
        base = str(base)
        if base not in self._base_funcs:
            base = 'Custom'
        self._base_option = base
        for ps in self._prewarp_select:
            ps.setEnabled(self._base_option != 'Custom')
        self._set_wave()

    def _set_postwarp(self, ident, postwarp):
        postwarp = str(postwarp)
        if postwarp in self._postwarp_funcs:
            func = self._postwarp_funcs[postwarp]
        else:
            func = identity
        self._postwarp_options[ident] = func
        self._set_wave()

    def _postwarp_param_changed(self, ident, value):
        self._postwarp_params[ident] = value
        self._set_wave(False)

    def _postwarp_param_finished(self, ident):
        self._project.flush(self._key)
        self._project.flush(self._aux_key)

    def _set_wave(self, immediate=True):
        prewarps = []
        for func, value in izip(self._prewarp_options, self._prewarp_params):
            name = None
            for (n, f) in self._prewarp_funcs.iteritems():
                if f == func:
                    name = n
                    break
            assert name
            prewarps.extend([(name, value)])
        postwarps = []
        for func, value in izip(self._postwarp_options, self._postwarp_params):
            name = None
            for (n, f) in self._postwarp_funcs.iteritems():
                if f == func:
                    name = n
                    break
            assert name
            postwarps.extend([(name, value)])
        if immediate:
            self._project.start_group()
        self._project.set(self._aux_key,
                          {
                              'base_option': self._base_option,
                              'prewarps': prewarps,
                              'postwarps': postwarps,
                          },
                          immediate)
        waveform = [0] * self._length
        base_func = self._base_funcs[self._base_option]
        for i in xrange(self._length):
            value = i * 2 / self._length - 1
            for (f, p) in izip(self._prewarp_options, self._prewarp_params):
                value = normalise(f(value, p))
            value = base_func(value)
            for (f, p) in izip(self._postwarp_options, self._postwarp_params):
                value = f(value, p)
            waveform[i] = value
        self._project.set(self._key, waveform, immediate)
        if immediate:
            self._project.end_group()
        self._waveform.set_data(waveform)


class WarpSelect(QtGui.QWidget):

    funcChanged = QtCore.pyqtSignal(int, QtCore.QString, name='funcChanged')
    paramChanged = QtCore.pyqtSignal(int, float, name='paramChanged')
    paramFinished = QtCore.pyqtSignal(int, name='paramFinished')

    def __init__(self, prewarp_options, ident, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._id = ident
        self._factor = 1000
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._func_select = QtGui.QComboBox()
        for option in prewarp_options:
            self._func_select.addItem(option)
        self._param = KSlider(QtCore.Qt.Horizontal)
        self._param.setRange(-self._factor, self._factor)
        self._value_display = QtGui.QLabel()
        layout.addWidget(self._func_select, 1)
        layout.addWidget(self._param, 1)
        layout.addWidget(self._value_display)
        metrics = QtGui.QFontMetrics(QtGui.QFont())
        width = metrics.width('-0.000')
        self._value_display.setFixedWidth(width)
        QtCore.QObject.connect(self._func_select,
                               QtCore.SIGNAL('currentIndexChanged(QString)'),
                               self._func_changed)
        QtCore.QObject.connect(self._param,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self._param_changed)
        QtCore.QObject.connect(self._param,
                               QtCore.SIGNAL('editingFinished()'),
                               self._param_finished)

    def set_state(self, name, value):
        self.blockSignals(True)
        self._func_select.setCurrentIndex(self._func_select.findText(name))
        self._param.setValue(round(value * self._factor))
        self._value_display.setText(str(round(value, 2)))
        self.blockSignals(False)

    def _func_changed(self, name):
        self.emit(QtCore.SIGNAL('funcChanged(int, QString)'), self._id, name)

    def _param_changed(self, value):
        self._value_display.setText(str(value / self._factor))
        self.emit(QtCore.SIGNAL('paramChanged(int, float)'),
                                self._id, value / self._factor)

    def _param_finished(self):
        self.emit(QtCore.SIGNAL('paramFinished(int)'), self._id)


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
            if self._custom:
                assert self.itemText(0) == 'Custom'
                self.removeItem(0)
                self._custom = False
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


