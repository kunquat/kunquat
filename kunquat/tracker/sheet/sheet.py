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
from PyQt4.QtGui import QLabel, QFrame

import kunquat.tracker.kqt_limits as lim
from pattern_editor import PatternEditor
from comp_params import CompParams
from subsong_params import SubsongParams
from pattern_params import PatternParams
from trigger_params import TriggerParams
from subsongs import Subsongs


class Sheet(QtGui.QWidget):

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
        QtGui.QWidget.__init__(self)

        self.p = p
        self._project = project
        self._playback = playback
        self._section = Section(project, self)

        layout = QtGui.QVBoxLayout(self)
        layout.setMargin(0)
        layout.setSpacing(0)
        self._layout = layout

        self._pattern_editor = PatternEditor(project,
                                             playback,
                                             self._section,
                                             pattern_changed_slot,
                                             pattern_offset_changed_slot,
                                             octave_spin,
                                             instrument_spin,
                                             typewriter)

        autoinst = QtGui.QCheckBox('Autoinst')
        autoinst.setChecked(True)

        grid = QtGui.QCheckBox('Grid')
        grid.setChecked(True)

        snap_to_grid = QtGui.QCheckBox('Snap')
        snap_to_grid.setChecked(True)

        QtCore.QObject.connect(autoinst,
                               QtCore.SIGNAL('stateChanged(int)'),
                               self._pattern_editor._pattern.autoinst_changed)
        QtCore.QObject.connect(grid,
                               QtCore.SIGNAL('stateChanged(int)'),
                               self._pattern_editor._pattern.grid_changed)
        QtCore.QObject.connect(snap_to_grid,
                               QtCore.SIGNAL('stateChanged(int)'),
                               self._pattern_editor._pattern.snap_to_grid_changed)

        splitter = QtGui.QSplitter()
        topbar = QtGui.QWidget()
        topbar.setMaximumHeight(40)
        top_layout = QtGui.QHBoxLayout(topbar)
        top_layout.setMargin(5)
        top_layout.setSpacing(5)
        top_layout.addWidget(playbackbar)
        top_layout.addWidget(autoinst, 0)
        top_layout.addWidget(grid, 0)
        top_layout.addWidget(snap_to_grid, 0)

        self._layout.addWidget(topbar)
        self._layout.addWidget(splitter)


        self._comp_params = CompParams(project)
        self._subsong_params = SubsongParams(project)
        self._pattern_params = PatternParams(project, self)
        self._trigger_params = TriggerParams(project)

        tools = QtGui.QWidget(self)
        tool_layout = QtGui.QVBoxLayout(tools)
        tool_layout.addWidget(self._comp_params)
        tool_layout.addWidget(self._subsong_params)
        tool_layout.addWidget(self._pattern_params)
        #tool_layout.addWidget(self._trigger_params)

        tool_scroller = QtGui.QScrollArea()
        tool_scroller.setWidgetResizable(True)
        tool_scroller.setEnabled(True)
        tool_scroller.setWidget(tools)
        tool_scroller.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)

        edit_splitter = QtGui.QSplitter()
        edit_splitter.setOrientation(QtCore.Qt.Horizontal)
        edit_splitter.addWidget(self._pattern_editor)
        edit_splitter.addWidget(tool_scroller)
        edit_splitter.setStretchFactor(0, 0)
        edit_splitter.setStretchFactor(1, 1)
        edit_splitter.setSizes([480, 1])
        edit_area = QtGui.QFrame()
        sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Fixed)
        edit_area.setSizePolicy(sizePolicy)
        edit_area.setFrameStyle(QFrame.Panel | QFrame.Sunken)
        edit_area_layout = QtGui.QVBoxLayout(edit_area)
        edit_area_layout.setMargin(0)
        edit_area_layout.setSpacing(0)
        edit_area_layout.addWidget(edit_splitter)
        self._edit_area = edit_area

        self._subsongs = Subsongs(self.p, self._section)
        playorder = QtGui.QWidget(self)
        playout = QtGui.QVBoxLayout(playorder)
        playout.setMargin(0)
        playout.setSpacing(0)

        playout.addWidget(self._subsongs)

        QtCore.QObject.connect(self._subsongs,
                               QtCore.SIGNAL('compositionParams()'),
                               self.to_comp_params)
        QtCore.QObject.connect(self._subsongs,
                               QtCore.SIGNAL('subsongChanged(int)'),
                               subsong_changed_slot)
        splitter.addWidget(playorder)
        splitter.addWidget(self._edit_area)
        splitter.setStretchFactor(0, 0)
        splitter.setStretchFactor(1, 1)
        splitter.setSizes([180, 1])
        splitter.setMinimumHeight(200)

        self._section.connect(self.section_changed)
        self._section.connect(section_changed_slot)

    def init(self):
        self._subsongs.init()
        self._comp_params.init()
        self._subsong_params.init()
        self._pattern_params.init()
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
        self._pattern_params.section_changed(self._section.subsong, self._section.section)

    def to_comp_params(self, *args):
        pass

    def to_subsong_params(self, num):
        pass

    def length_changed(self):
        self._pattern_editor._pattern.length_changed()

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

    def set(self, track, system):
        assert track < lim.SONGS_MAX
        assert system < lim.SECTIONS_MAX
        pat = self._project._composition.get_pattern(track, system)
        if pat == None:
            return
        if self._subsong == track and self._section == system:
            pass
            #print('repeat signal for {0}:{1}'.format(subsong, section))
        self._subsong = track
        self._section = system
        QtCore.QObject.emit(self, QtCore.SIGNAL('sectionChanged(int, int)'),
                            track, system)

    @property
    def subsong(self):
        return self._subsong

    @property
    def section(self):
        return self._section


