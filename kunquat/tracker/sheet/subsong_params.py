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
from kunquat.tracker.param_slider import ParamSlider


class SubsongParams(QtGui.QGroupBox):

    def __init__(self, project):
        QtGui.QWidget.__init__(self, 'song')
        self._project = project
        self._layout = QtGui.QVBoxLayout(self)
        self._subsong = 0

        key = 'song_00/p_song.json'
        self._title = QtGui.QLabel()
        self._layout.addWidget(self._title)
        self._tempo = ParamSlider(project,
                                  'Initial tempo:',
                                  (6, 360),
                                  120,
                                  key,
                                  'tempo',
                                  unit='BPM')
        self._layout.addWidget(self._tempo)
        self._global_volume = ParamSlider(project,
                                          'Initial global volume:',
                                          (-96, 8),
                                          0,
                                          key,
                                          'global_vol',
                                          decimals=1,
                                          unit='dB')
        self._layout.addWidget(self._global_volume)
        self._button_delete_song = QtGui.QPushButton('delete song')
        self._layout.addWidget(self._button_delete_song)
        self._button_new_song = QtGui.QPushButton('new song')
        self._layout.addWidget(self._button_new_song)



    def init(self):
        self._tempo.init()
        self._global_volume.init()

    def subsong_changed(self, subsong):
        self._subsong = subsong
        song_id = 'song_{0:02d}'.format(subsong)
        key = '%s/p_song.json' % song_id
        song = self.-project._composition.get_song(song_id)
        song_name = song.get_name()
        self._title.setText(song_name)
        self._tempo.set_key(key)
        self._global_volume.set_key(key)

    def sync(self):
        self._tempo.sync()
        self._global_volume.sync()


