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

import kqt_limits as lim
from pattern import Pattern
from subsongs import Subsongs


class Sheet(QtGui.QSplitter):

    def __init__(self, project, playback, parent=None):
        QtGui.QSplitter.__init__(self, parent)

        self.project = project
        self._playback = playback
        self.section = Section(project, self)

        subsong_editor = QtGui.QLabel('[subsong editor]')

        self.addWidget(self.create_subsong_editor(project))
        self.addWidget(self.create_pattern_editor(project))
        self.setStretchFactor(0, 0)
        self.setStretchFactor(1, 1)
        self.setSizes([180, 1])

    def create_pattern_editor(self, project):
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

        pattern = Pattern(project, self.section, self._playback)

        layout.addWidget(top_control, 0)
        layout.addWidget(pattern, 1)
        return pattern_editor

    def create_subsong_editor(self, project):
        subsong_editor = Subsongs(project, self.section)
        return subsong_editor


class Section(QtCore.QObject):

    section_changed = QtCore.pyqtSignal(int, int, name='sectionChanged')

    def __init__(self, project, parent=None):
        QtCore.QObject.__init__(self, parent)
        self._project = project
        self._subsong = 0
        self._section = 0

    def connect(self, func):
        self.section_changed.connect(func)

    def set(self, subsong, section):
        assert subsong < lim.SUBSONGS_MAX
        assert section < lim.SECTIONS_MAX
        pat = self._project.get_pattern(subsong, section)
        if pat == None:
            return
        if self._subsong == subsong and self._section == section:
            print('repeat signal for {0}:{1}'.format(subsong, section))
        self._subsong = subsong
        self._section = section
        QtCore.QObject.emit(self, QtCore.SIGNAL('sectionChanged(int, int)'),
                            subsong, section)

    @property
    def subsong(self):
        return self._subsong

    @property
    def section(self):
        return self._section


