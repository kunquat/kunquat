# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4 import QtGui, QtCore

import kunquat.tracker.kqt_limits as lim
from comp_params import CompParams
from pattern_editor import PatternEditor
from subsong_params import SubsongParams
from subsongs import Subsongs


class Sheet(QtGui.QSplitter):

    def __init__(self,
                 p,
                 project,
                 playback,
                 subsong_changed_slot,
                 section_changed_slot,
                 pattern_changed_slot,
                 pattern_offset_changed_slot,
                 octave_spin,
                 instrument_spin,
                 typewriter,
                 playbackbar,
                 parent=None):
        QtGui.QSplitter.__init__(self, parent)

        self.p = p
        self._project = project
        self._playback = playback
        self._section = Section(project, self)

        self._comp_params = CompParams(project)
        self._subsong_params = SubsongParams(project)
        self._pattern_editor = PatternEditor(project,
                                             playback,
                                             self._section,
                                             pattern_changed_slot,
                                             pattern_offset_changed_slot,
                                             octave_spin,
                                             instrument_spin,
                                             typewriter)

        tools = QtGui.QWidget(self)
        tool_layout = QtGui.QVBoxLayout(tools)
        tool_layout.addWidget(self._comp_params)
        tool_layout.addWidget(self._subsong_params)

        edit_area = QtGui.QSplitter()
        edit_area.setOrientation(QtCore.Qt.Horizontal)
        edit_area.addWidget(self._pattern_editor)
        edit_area.addWidget(tools)
        self._edit_area = edit_area

        self._subsongs = Subsongs(self.p, self._section)
        playorder = QtGui.QWidget(self)
        playout = QtGui.QVBoxLayout(playorder)
        playout.setMargin(0)
        playout.setSpacing(0)

        playout.addWidget(playbackbar)
        playout.addWidget(self._subsongs)

        QtCore.QObject.connect(self._subsongs,
                               QtCore.SIGNAL('compositionParams()'),
                               self.to_comp_params)
        QtCore.QObject.connect(self._subsongs,
                               QtCore.SIGNAL('subsongChanged(int)'),
                               subsong_changed_slot)
        self.addWidget(playorder)
        self.addWidget(self._edit_area)
        self.setStretchFactor(0, 0)
        self.setStretchFactor(1, 1)
        self.setSizes([180, 1])
        self.setMinimumHeight(200)

        self._section.connect(self.section_changed)
        self._section.connect(section_changed_slot)

    def init(self):
        self._subsongs.init()
        self._comp_params.init()
        self._subsong_params.init()
        self._pattern_editor.init()
        QtCore.QObject.connect(self._subsongs,
                               QtCore.SIGNAL('compositionParams()'),
                               self.to_comp_params)
        QtCore.QObject.connect(self._subsongs,
                               QtCore.SIGNAL('subsongParams(int)'),
                               self.to_subsong_params)
        QtCore.QObject.connect(self._subsongs,
                               QtCore.SIGNAL('subsongParams(int)'),
                               self._subsong_params.subsong_changed)

    def section_changed(self, *args):
        pass

    def to_comp_params(self, *args):
        pass

    def to_subsong_params(self, num):
        pass

    def sync(self):
        self._comp_params.sync()
        self._subsong_params.sync()
        self._pattern_editor.sync()


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
        assert subsong < lim.SONGS_MAX
        assert section < lim.SECTIONS_MAX
        pat = self._project._composition.get_pattern(subsong, section)
        if pat == None:
            return
        if self._subsong == subsong and self._section == section:
            pass
            #print('repeat signal for {0}:{1}'.format(subsong, section))
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


