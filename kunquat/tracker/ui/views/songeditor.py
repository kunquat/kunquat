# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
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


class SongEditor(QWidget):

    def __init__(self):
        super().__init__()

        self._name = NameEditor()
        self._tempo_editor = TempoEditor()

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

    def set_ui_model(self, ui_model):
        self._name.set_ui_model(ui_model)
        self._tempo_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._tempo_editor.unregister_updaters()
        self._name.unregister_updaters()


class NameEditor(QLineEdit):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('textEdited(const QString&)'), self._change_name)

        self._update_name()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set(['signal_song', 'signal_order_list'])
        if not signals.isdisjoint(update_signals):
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

        self._updater.signal_update(set(['signal_song', 'signal_order_list']))


class TempoEditor(QDoubleSpinBox):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self.setDecimals(1)
        self.setMinimum(1)
        self.setMaximum(999)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('valueChanged(double)'), self._change_tempo)

        self._update_tempo()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set(['signal_song', 'signal_order_list'])
        if not signals.isdisjoint(update_signals):
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

        self._updater.signal_update(set(['signal_song']))


