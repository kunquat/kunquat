# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010-2011
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
                 project,
                 playback,
                 subsong_changed_slot,
                 section_changed_slot,
                 pattern_changed_slot,
                 pattern_offset_changed_slot,
                 octave_spin,
                 instrument_spin,
                 playbackbar,
                 parent=None):
        QtGui.QSplitter.__init__(self, parent)

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
                                             instrument_spin)
        self._edit_area = QtGui.QStackedWidget()
        self._edit_area.setFrameShape(QtGui.QFrame.StyledPanel)
        self._edit_area.addWidget(self._comp_params)
        self._edit_area.addWidget(self._subsong_params)
        self._edit_area.addWidget(self._pattern_editor)

        self._subsongs = Subsongs(project, self._section)
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

        self._section.connect(self.section_changed)
        QtCore.QObject.connect(self._subsongs,
                               QtCore.SIGNAL('compositionParams()'),
                               self.to_comp_params)
        QtCore.QObject.connect(self._subsongs,
                               QtCore.SIGNAL('subsongParams(int)'),
                               self.to_subsong_params)
        QtCore.QObject.connect(self._subsongs,
                               QtCore.SIGNAL('subsongParams(int)'),
                               self._subsong_params.subsong_changed)
        self._section.connect(section_changed_slot)

    def section_changed(self, *args):
        self._edit_area.setCurrentWidget(self._pattern_editor)

    def to_comp_params(self, *args):
        self._edit_area.setCurrentWidget(self._comp_params)

    def to_subsong_params(self, num):
        self._edit_area.setCurrentWidget(self._subsong_params)

    def sync(self):
        self._subsongs.sync()
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
        assert subsong < lim.SUBSONGS_MAX
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


