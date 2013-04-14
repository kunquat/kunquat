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

from __future__ import print_function

from PyQt4 import QtCore, QtGui


class TypeSelect(QtGui.QComboBox):

    typeChanged = QtCore.pyqtSignal(int, str, name='typeChanged')

    type_to_index = {
                None: 0,
                'bool': 1,
                'int': 2,
                'float': 3,
                #'timestamp': 4,
            }

    index_to_type = dict((y, x) for (x, y) in
                         type_to_index.iteritems())

    def __init__(self, index, parent=None):
        QtGui.QComboBox.__init__(self, parent)
        self._index = index
        for item in ('None', 'Boolean', 'Integer', 'Floating'):
            self.addItem(item)

    def init(self):
        QtCore.QObject.connect(self,
                               QtCore.SIGNAL('currentIndexChanged(int)'),
                               self._type_changed)

    @property
    def index(self):
        return self._index

    @index.setter
    def index(self, value):
        self._index = value

    @property
    def type_format(self):
        return TypeSelect.index_to_type[self.currentIndex()]

    @type_format.setter
    def type_format(self, type_name):
        self.setCurrentIndex(TypeSelect.type_to_index[type_name])

    def _type_changed(self, index):
        QtCore.QObject.emit(self,
                            QtCore.SIGNAL('typeChanged(int, QString*)'),
                            self.index,
                            TypeSelect.index_to_type[index])


