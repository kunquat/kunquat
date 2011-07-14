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

from __future__ import division
from __future__ import print_function
from itertools import takewhile

from PyQt4 import QtGui, QtCore


class ParamCombo(QtGui.QComboBox):

    def __init__(self,
                 project,
                 label,
                 values,
                 default_text,
                 key,
                 dict_key=None,
                 parent=None):
        assert any(item[0] == default_text for item in values)
        QtGui.QComboBox.__init__(self, parent)
        self._project = project
        self._dict_key = dict_key
        self._items = []
        for text, value in values:
            self.addItem(text, value)
        self._default_text = default_text
        self._lock_update = False
        self.set_key(key)
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('currentIndexChanged(int)'),
                               self._index_changed)

    def addItem(self, text, value):
        if any(item[0] == text for item in self._items):
            index = len(takewhile(item[0] != text for item in self._items))
            assert index < len(self._items)
            item[index] = (text, value)
        else:
            self._items.extend([(text, value)])
            QtGui.QComboBox.addItem(self, text)

    def set_key(self, key):
        value = filter(lambda i: i[0] == self._default_text,
                       self._items)[0][1]
        if self._dict_key:
            d = self._project[key]
            if d and self._dict_key in d:
                value = self._project[key][self._dict_key]
        else:
            actual = self._project[key]
            if actual != None:
                value = actual
        matches = filter(lambda i: i[1] == value, self._items)
        self._lock_update = True
        if matches:
            self.setCurrentIndex(self.findText(matches[0][0]))
        else:
            self.setCurrentIndex(self.findText(self._default_text))
        self._lock_update = False
        self._key = key

    def sync(self):
        self.set_key(self._key)

    def _index_changed(self, index):
        if self._lock_update:
            return
        value = self._items[self.currentIndex()][1]
        if self._dict_key:
            d = self._project[self._key]
            if d == None:
                d = {}
            d[self._dict_key] = value
            self._project[self._key] = d
        else:
            self._project[self._key] = value


