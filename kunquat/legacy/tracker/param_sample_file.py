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


class ParamSampleFile(QtGui.QWidget):

    def __init__(self, project, key, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._key = key
        layout = QtGui.QHBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._load_button = QtGui.QPushButton('Load')
        layout.addWidget(self._load_button)
        self._filters = {
                            'wv': 'WavPack',
                        }

    def init(self):
        QtCore.QObject.connect(self._load_button, QtCore.SIGNAL('clicked()'),
                               self._load)
        self.set_key(self._key)

    def set_key(self, key):
        self._key = key

    def sync(self):
        self.set_key(self._key)

    def _load(self):
        suffix = self._key.split('.')[-1]
        fname = QtGui.QFileDialog.getOpenFileName(
                caption='Load {0} sample'.format(self._filters[suffix]),
                filter='{0} samples (*.{1})'.format(self._filters[suffix],
                                                    suffix))
        if fname:
            with open(fname, 'rb') as f:
                self._project[self._key] = f.read()


