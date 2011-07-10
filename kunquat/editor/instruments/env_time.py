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

from PyQt4 import QtCore, QtGui

import kunquat.editor.kqt_limits as lim
from kunquat.editor.envelope import Envelope
from kunquat.editor.param_check import ParamCheck
from kunquat.editor.param_slider import ParamSlider


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
                                         (-16, 16),
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
        if support_loop:
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
                                         0,
                                         envelope,
                                         'envelope')
            self._loop_end = LoopBound(project,
                                       'Loop end:',
                                       self._key_base.format(self._cur_inst),
                                       1,
                                       envelope,
                                       'envelope')
            loop_layout.addWidget(self._loop_enabled)
            loop_layout.addSpacing(10)
            loop_layout.addWidget(self._loop_start)
            loop_layout.addSpacing(10)
            loop_layout.addWidget(self._loop_end)
            layout.addLayout(loop_layout)
            self._widgets.extend([self._loop_enabled,
                                  self._loop_start,
                                  self._loop_end])

    def inst_changed(self, num):
        self._cur_inst = num
        for widget in self._widgets:
            widget.set_key(self._key_base.format(self._cur_inst))

    def sync(self):
        for widget in self._widgets:
            widget.sync()


class LoopBound(QtGui.QWidget):

    def __init__(self,
                 project,
                 label,
                 key,
                 mark_index,
                 envelope,
                 dict_key=None,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._mark_index = mark_index
        self._envelope = envelope
        self._dict_key = dict_key
        self._lock_update = False
        QtCore.QObject.connect(self._envelope,
                               QtCore.SIGNAL('nodeCountChanged(int)'),
                               self._node_count_changed)

        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        lab = QtGui.QLabel(label)

        self._spin = QtGui.QSpinBox()
        self._spin.setMinimum(0)
        self._spin.setMaximum(self._envelope.node_count() - 1)
        QtCore.QObject.connect(self._spin,
                               QtCore.SIGNAL('valueChanged(int)'),
                               self._value_changed)
        QtCore.QObject.connect(self._spin,
                               QtCore.SIGNAL('editingFinished()'),
                               self._finished)

        layout.addWidget(lab)
        layout.addWidget(self._spin)

        self.set_key(key)

    def set_key(self, key):
        value = 0
        dvalue = {}
        if self._dict_key:
            d = self._project[key]
            if d and self._dict_key in d:
                dvalue = self._project[key][self._dict_key]
        else:
            actual = self._project[key]
            if actual != None:
                dvalue = actual
        if 'marks' in dvalue:
            marks = value['marks']
            if len(marks) > self._mark_index:
                value = marks[self._mark_index]
        self._lock_update = True
        self._spin.setValue(int(value))
        self._lock_update = False
        self._key = key

    def sync(self):
        self.set_key(self._key)

    def _node_count_changed(self, count):
        self._spin.setMaximum(count - 1)

    def _value_changed(self, value):
        if self._lock_update:
            return
        self._envelope.set_mark(self._mark_index, value)
        """
        dvalue = {}
        if self._dict_key:
            d = self._project[self._key]
            if not d:
                d = {}
            if self._dict_key in d:
                dvalue = d[self._dict_key]
            marks = dvalue.get('marks', [])
            marks.extend([-1] * (self._mark_index - len(marks) + 1))
            marks[self._mark_index] = value
            dvalue['marks'] = marks
            d[self._dict_key] = dvalue
            self._project.set(self._key, d, immediate=False)
        else:
            if self._project[self._key]:
                dvalue = self._project[self._key]
            marks = dvalue.get('marks', [])
            marks.extend([-1] * (self._mark_index - len(marks) + 1))
            marks[self._mark_index] = value
            dvalue['marks'] = marks
            self._project.set(self._key, dvalue, immediate=False)
        """

    def _finished(self):
        self._project.flush(self._key)


