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
import json

from PyQt4 import QtGui, QtCore


class ParamLine(QtGui.QWidget):

    def __init__(self,
                 project,
                 label,
                 default_val,
                 key,
                 dict_key=None,
                 parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._default_val = default_val
        self._dict_key = dict_key

        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.addWidget(QtGui.QLabel(label))

        self._edit = FixedLineEdit()
        layout.addWidget(self._edit)
        self._key = key

    def init(self):
        QtCore.QObject.connect(self._edit,
                               QtCore.SIGNAL('textEdited(QString)'),
                               self._value_changed)
        QtCore.QObject.connect(self._edit,
                               QtCore.SIGNAL('editingFinished()'),
                               self._finished)
        self.set_key(self._key)

    def set_key(self, key):
        self._key = key
        if not key:
            return
        value = self._default_val
        if self._dict_key:
            d = self._project[key]
            if d and self._dict_key in d:
                value = self._project[key][self._dict_key]
        else:
            actual = self._project[key]
            if actual != None:
                value = actual
        if isinstance(self._default_val, list):
            value = ', '.join(value)
        self._edit.setText(value)

    def sync(self):
        self.set_key(self._key)

    def _value_changed(self, value):
        pass

    def _finished(self):
        value = unicode(self._edit.text())
        if isinstance(self._default_val, list):
            value = [s.strip() for s in value.split(',')]
        if self._dict_key:
            d = self._project[self._key]
            if d == None:
                d = {}
            d[self._dict_key] = value
            self._project[self._key] = d
        else:
            self._project[self._key] = value
        #self._project.flush(self._key)


class FixedLineEdit(QtGui.QLineEdit):

    def __init__(self, parent=None):
        QtGui.QLineEdit.__init__(self, parent)

    def keyPressEvent(self, ev):
        if ev.type() == QtCore.QEvent.KeyPress:
            if ev.modifiers() == QtCore.Qt.ControlModifier and \
                    ev.key() in (QtCore.Qt.Key_Z, QtCore.Qt.Key_Y):
                ev.ignore()
                return
        QtGui.QLineEdit.keyPressEvent(self, ev)


