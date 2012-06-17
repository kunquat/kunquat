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


class ParamCheck(QtGui.QCheckBox):

    def __init__(self,
                 project,
                 label,
                 default_val,
                 key,
                 dict_key=None,
                 parent=None):
        assert default_val in (True, False)
        QtGui.QCheckBox.__init__(self, label, parent)
        self._project = project
        self._dict_key = dict_key
        self._default_val = default_val
        self._lock_update = False
        self.set_key(key)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('stateChanged(int)'),
                               self._value_changed)

    def set_key(self, key):
        value = self._default_val
        if self._dict_key:
            d = self._project[key]
            if d and self._dict_key in d:
                value = self._project[key][self._dict_key]
        else:
            actual = self._project[key]
            if actual != None:
                value = actual
        self._lock_update = True
        self.setCheckState(QtCore.Qt.Checked if value
                           else QtCore.Qt.Unchecked)
        self._lock_update = False
        self._key = key
        self._value = value

    def sync(self):
        self.set_key(self._key)

    def _value_changed(self, state):
        if self._lock_update:
            return
        value = True if state == QtCore.Qt.Checked else False
        if self._dict_key:
            d = self._project[self._key]
            if d == None:
                d = {}
            d[self._dict_key] = value
            self._project[self._key] = d
        else:
            self._project[self._key] = value
        self._value = value


