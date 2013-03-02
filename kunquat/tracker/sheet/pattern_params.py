# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2012
#          Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4 import QtGui, QtCore
import kunquat.tracker.timestamp as ts
from kunquat.tracker.timestamp_spin import TimestampSpin

class PatternParams(QtGui.QGroupBox):

    def __init__(self, project, sheet):
        QtGui.QWidget.__init__(self, 'pattern')
        
        self._sheet = sheet

        self._project = project
        self._layout = QtGui.QVBoxLayout(self)
        self._title = QtGui.QLabel()
        self._layout.addWidget(self._title)
        self._length = TimestampSpin(project,
                                     'Length:',
                                     (ts.Timestamp(0), ts.Timestamp(1024, 0)),
                                     ts.Timestamp(16),
                                     'pat_000/p_pattern.json',
                                     'length',
                                     2)
        self._layout.addWidget(self._length)

        self._button_delete_pattern = QtGui.QPushButton('delete pattern')
        self._button_delete_pattern.clicked.connect(self.delete_current)
        self._layout.addWidget(self._button_delete_pattern)
        self._button_reuse_pattern = QtGui.QPushButton('reuse pattern')
        self._layout.addWidget(self._button_reuse_pattern)
        self._button_new_pattern = QtGui.QPushButton('new pattern')
        self._layout.addWidget(self._button_new_pattern)


    def init(self):
        self._length.init()
        QtCore.QObject.connect(self._length,
                               QtCore.SIGNAL('tsChanged(int, int)'),
                               self._sheet.length_changed)
    def delete_current(self):
        global_system = (self._track, self._system)
        self._project._composition.delete_from_system(global_system)

    def section_changed(self, track, system):
        self._track = track
        self._system = system
        pattern_num = self._project._composition.get_pattern(track, system)
        if pattern_num != None:
            song = self._project._composition.get_song_by_track(track)
            pattern_instance = song.get_pattern_instance(system)
            pattern_instance_name = pattern_instance.get_name(song)
            self._title.setText(pattern_instance_name)
            key = 'pat_{0:03d}/p_pattern.json'.format(pattern_num[0])
            self._length.set_key(key)

    def sync(self):
        self._length.sync()

