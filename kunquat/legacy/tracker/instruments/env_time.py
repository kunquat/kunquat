# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011-2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division, print_function

from PyQt4 import QtCore, QtGui

import kunquat.tracker.kqt_limits as lim
from kunquat.tracker.envelope import Envelope
from kunquat.tracker.param_check import ParamCheck
from kunquat.tracker.param_slider import ParamSlider


class EnvTime(QtGui.QWidget):

    def __init__(self,
                 project,
                 key_base,
                 name,
                 envelope,
                 support_loop=False,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._cur_inst = 0
        self._project = project
        self._key_base = key_base
        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)

        top_layout = QtGui.QHBoxLayout()
        top_layout.setMargin(0)
        top_layout.setSpacing(0)
        label = QtGui.QLabel(name + ':')
        self._enabled = ParamCheck(project,
                                   'Enabled',
                                   False,
                                   self._key_base.format(self._cur_inst),
                                   'enabled')
        self._scale_amount = ParamSlider(project,
                                         'Scale amount:',
                                         (-3, 3),
                                         0,
                                         self._key_base.format(self._cur_inst),
                                         'scale_amount',
                                         decimals=2)
        self._scale_center = ParamSlider(project,
                                         'Scale center:',
                                         (-4800, 4800),
                                         0,
                                         self._key_base.format(self._cur_inst),
                                         'scale_center')
        top_layout.addSpacing(10)
        top_layout.addWidget(label)
        top_layout.addSpacing(10)
        top_layout.addWidget(self._enabled)
        top_layout.addWidget(self._scale_amount)
        top_layout.addWidget(self._scale_center)
        self._env = envelope
        layout.addLayout(top_layout)
        layout.addWidget(self._env, 1)

        self._widgets = [self._enabled,
                         self._scale_amount,
                         self._scale_center,
                         self._env]
        self._support_loop = support_loop
        if self._support_loop:
            loop_layout = QtGui.QHBoxLayout()
            loop_layout.setMargin(0)
            loop_layout.setSpacing(0)
            self._loop_enabled = ParamCheck(project,
                                            'Loop',
                                            False,
                                            self._key_base.format(self._cur_inst),
                                            'loop')
            self._loop_start = LoopBound(project,
                                         'Loop start:',
                                         self._key_base.format(self._cur_inst),
                                         #0,
                                         #envelope,
                                         'envelope')
            self._loop_end = LoopBound(project,
                                       'Loop end:',
                                       self._key_base.format(self._cur_inst),
                                       #1,
                                       #envelope,
                                       'envelope')
            loop_layout.addWidget(self._loop_enabled)
            loop_layout.addSpacing(10)
            loop_layout.addWidget(self._loop_start)
            loop_layout.addSpacing(10)
            loop_layout.addWidget(self._loop_end)
            layout.addLayout(loop_layout)
            self._widgets.extend([self._loop_enabled])

    def init(self):
        for widget in self._widgets:
            widget.init()
        if self._support_loop:
            self._loop_start.init()
            self._loop_end.init()

            QtCore.QObject.connect(self._loop_enabled,
                                   QtCore.SIGNAL('stateChanged(int)'),
                                   self._set_loop_display)

            self._loop_start.set_upper_bound(0)
            self._loop_end.set_lower_bound(0)

            QtCore.QObject.connect(self._loop_start,
                                   QtCore.SIGNAL('valueChanged(int)'),
                                   self._loop_start_changed)
            QtCore.QObject.connect(self._loop_end,
                                   QtCore.SIGNAL('valueChanged(int)'),
                                   self._loop_end_changed)
            QtCore.QObject.connect(self._loop_start,
                                   QtCore.SIGNAL('valueFinished()'),
                                   self._loop_point_finished)
            QtCore.QObject.connect(self._loop_end,
                                   QtCore.SIGNAL('valueFinished()'),
                                   self._loop_point_finished)

            QtCore.QObject.connect(self._env,
                                   QtCore.SIGNAL('nodesChanged()'),
                                   self._fix_loop)

    def _set_loop_display(self, state):
        checked = state == QtCore.Qt.Checked
        self._env.set_mark_display(0, checked)
        self._env.set_mark_display(1, checked)

    def inst_changed(self, num):
        self._cur_inst = num
        key = self._key_base.format(self._cur_inst)
        self._fix_loop()
        for widget in self._widgets:
            widget.set_key(key)

    def sync(self):
        self.inst_changed(self._cur_inst)

    def _fix_loop(self):
        if not self._support_loop:
            return
        key = self._key_base.format(self._cur_inst)
        d = self._project[key]
        try:
            length = len(d['envelope']['nodes'])
            length = max(length, 2)
        except (KeyError, TypeError):
            length = 2
        try:
            marks = d['envelope']['marks']
            if len(marks) != 2:
                marks = [0, 0]
        except (KeyError, TypeError):
            marks = [0, 0]
        self._loop_end.set_upper_bound(length - 1)
        self._loop_end.set_lower_bound(marks[0])
        self._loop_end.set_value(marks[1])
        self._loop_start.set_upper_bound(marks[1])
        self._loop_start.set_value(marks[0])

    def _loop_start_changed(self, value):
        assert self._support_loop
        self._loop_end.set_lower_bound(value)
        self._env.set_mark(0, value)

    def _loop_end_changed(self, value):
        assert self._support_loop
        self._loop_start.set_upper_bound(value)
        self._env.set_mark(1, value)

    def _loop_point_finished(self):
        assert self._support_loop
        self._env.finished()


class LoopBound(QtGui.QWidget):

    valueChanged = QtCore.pyqtSignal(int, name='valueChanged')
    valueFinished = QtCore.pyqtSignal(name='valueFinished')

    def __init__(self,
                 project,
                 label,
                 key,
                 dict_key=None,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._dict_key = dict_key

        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        lab = QtGui.QLabel(label)

        self._lower_bound = 0
        self._upper_bound = float('inf')
        self._spin = QtGui.QSpinBox()
        self._spin.setMinimum(0)
        self._spin.setMaximum(99999) # XXX: ugly

        layout.addWidget(lab)
        layout.addWidget(self._spin)

    def init(self):
        QtCore.QObject.connect(self._spin,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self._value_changed)
        QtCore.QObject.connect(self._spin,
                               QtCore.SIGNAL('editingFinished()'),
                               self._finished)

    def set_lower_bound(self, bound):
        self._lower_bound = bound
        self._spin.blockSignals(True)
        self._spin.setMinimum(bound)
        self._spin.blockSignals(False)

    def set_upper_bound(self, bound):
        self._upper_bound = bound
        self._spin.blockSignals(True)
        self._spin.setMaximum(bound)
        self._spin.blockSignals(False)

    def set_value(self, value):
        self._spin.blockSignals(True)
        self._spin.setValue(value)
        self._spin.blockSignals(False)

    def _value_changed(self, value):
        QtCore.QObject.emit(self, QtCore.SIGNAL('valueChanged(int)'), value)

    def _finished(self):
        QtCore.QObject.emit(self, QtCore.SIGNAL('valueFinished()'))


