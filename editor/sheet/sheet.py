#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4 import QtGui, QtCore

from pattern import Pattern


class Sheet(QtGui.QSplitter):

    def __init__(self, parent=None):
        QtGui.QSplitter.__init__(self, parent)

        subsong_editor = QtGui.QLabel('[subsong editor]')

        self.addWidget(subsong_editor)
        self.addWidget(self.create_pattern_editor())
        self.setStretchFactor(0, 0)
        self.setStretchFactor(1, 1)

    def create_pattern_editor(self):
        pattern_editor = QtGui.QWidget()
        layout = QtGui.QVBoxLayout(pattern_editor)
        layout.setMargin(0)
        layout.setSpacing(0)

        top_control = QtGui.QWidget()
        top_layout = QtGui.QHBoxLayout(top_control)

        name = QtGui.QLabel('[pattern num/name]')
        
        length = QtGui.QLabel('[length]')

        top_layout.addWidget(name)
        top_layout.addWidget(length)

        pattern = Pattern(None)

        layout.addWidget(top_control, 0)
        layout.addWidget(pattern, 1)
        return pattern_editor


