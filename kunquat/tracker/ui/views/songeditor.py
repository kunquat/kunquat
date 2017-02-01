# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.kunquat.limits import *
from .headerline import HeaderLine
from .updatingview import UpdatingView


class SongEditor(QWidget, UpdatingView):

    def __init__(self):
        super().__init__()
        self._name = NameEditor()
        self._tempo_editor = TempoEditor()

        self.add_updating_child(self._name, self._tempo_editor)

        gl = QGridLayout()
        gl.addWidget(QLabel('Name:'), 0, 0)
        gl.addWidget(self._name, 0, 1)
        gl.addWidget(QLabel('Initial tempo:'), 1, 0)
        gl.addWidget(self._tempo_editor, 1, 1)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Song'))
        v.addLayout(gl)
        v.addStretch(1)
        self.setLayout(v)


class NameEditor(QLineEdit, UpdatingView):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.register_action('signal_song', self._update_name)
        self.register_action('signal_order_list', self._update_name)

        QObject.connect(self, SIGNAL('textEdited(const QString&)'), self._change_name)

        self._update_name()

    def _set_name(self, name):
        old_block = self.blockSignals(True)
        if name != self.text():
            self.setText(name)
        self.blockSignals(old_block)

    def _update_name(self):
        album = self._ui_model.get_module().get_album()
        track_num = album.get_selected_track_num()
        if track_num < 0:
            self._set_name('')
            self.setEnabled(False)
            return
        song = album.get_song_by_track(track_num)
        self.setEnabled(True)
        self._set_name(song.get_name())

    def _change_name(self, text):
        album = self._ui_model.get_module().get_album()
        track_num = album.get_selected_track_num()
        if track_num < 0:
            return
        song = album.get_song_by_track(track_num)
        song.set_name(text)

        self._updater.signal_update('signal_song', 'signal_order_list')


class TempoEditor(QDoubleSpinBox, UpdatingView):

    def __init__(self):
        super().__init__()
        self.setDecimals(1)
        self.setMinimum(1)
        self.setMaximum(999)

    def _on_setup(self):
        self.register_action('signal_song', self._update_tempo)
        self.register_action('signal_order_list', self._update_tempo)

        QObject.connect(self, SIGNAL('valueChanged(double)'), self._change_tempo)

        self._update_tempo()

    def _set_tempo(self, tempo):
        old_block = self.blockSignals(True)
        if tempo != self.value():
            self.setValue(tempo)
        self.blockSignals(old_block)

    def _update_tempo(self):
        album = self._ui_model.get_module().get_album()
        track_num = album.get_selected_track_num()
        if track_num < 0:
            self._set_tempo(120)
            self.setEnabled(False)
            return
        song = album.get_song_by_track(track_num)
        self.setEnabled(True)
        self._set_tempo(song.get_initial_tempo())

    def _change_tempo(self, value):
        album = self._ui_model.get_module().get_album()
        track_num = album.get_selected_track_num()
        if track_num < 0:
            return
        song = album.get_song_by_track(track_num)
        song.set_initial_tempo(value)

        self._updater.signal_update('signal_song')


