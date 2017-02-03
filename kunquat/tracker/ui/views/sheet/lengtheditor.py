# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2017
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

import kunquat.tracker.ui.model.tstamp as tstamp
from kunquat.tracker.ui.views.updater import Updater


class LengthEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._is_latest_committed = True

        self._spinbox = QDoubleSpinBox()
        self._spinbox.setMinimum(0)
        self._spinbox.setMaximum(1024)
        self._spinbox.setDecimals(3)

        h = QHBoxLayout()
        h.setContentsMargins(5, 0, 5, 0)
        h.setSpacing(5)
        h.addWidget(QLabel('Pattern length'))
        h.addWidget(self._spinbox)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_module', self._update_value)
        self.register_action('signal_pattern_length', self._update_value)
        self.register_action('signal_selection', self._update_value)
        self.register_action('signal_order_list', self._update_value)
        self.register_action('signal_undo', self._update_value)
        self.register_action('signal_redo', self._update_value)

        self._update_value()

        QObject.connect(
                self._spinbox, SIGNAL('valueChanged(double)'), self._change_length)
        QObject.connect(
                self._spinbox, SIGNAL('editingFinished()'), self._change_length_final)

    def _get_pattern(self):
        module = self._ui_model.get_module()
        album = module.get_album()
        if not album.get_existence():
            return None

        selection = self._ui_model.get_selection()
        location = selection.get_location()
        song = album.get_song_by_track(location.get_track())
        pinst = song.get_pattern_instance(location.get_system())
        pattern = pinst.get_pattern()
        return pattern

    def _update_value(self):
        pattern = self._get_pattern()
        if not pattern:
            self.setEnabled(False)
            old_block = self._spinbox.blockSignals(True)
            self._spinbox.setValue(0)
            self._spinbox.blockSignals(old_block)
            return

        length = pattern.get_length()
        length_val = float(length)

        self.setEnabled(True)
        old_block = self._spinbox.blockSignals(True)
        if length_val != self._spinbox.value():
            self._spinbox.setValue(length_val)
        self._spinbox.blockSignals(old_block)

    def _change_value(self, new_value, is_final):
        pattern = self._get_pattern()
        if not pattern:
            return

        sheet_manager = self._ui_model.get_sheet_manager()

        length = tstamp.Tstamp(new_value)
        if length == pattern.get_length():
            if is_final and not self._is_latest_committed:
                sheet_manager.set_pattern_length(pattern, length, is_final)
                self._is_latest_committed = True
            return

        sheet_manager.set_pattern_length(pattern, length, is_final)
        self._updater.signal_update('signal_pattern_length')

    def _change_length(self, new_value):
        self._is_latest_committed = False
        self._change_value(new_value, is_final=False)

    def _change_length_final(self):
        new_value = self._spinbox.value()
        self._change_value(new_value, is_final=True)


