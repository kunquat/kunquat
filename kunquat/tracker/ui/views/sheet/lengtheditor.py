# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import kunquat.tracker.ui.model.tstamp as tstamp


class LengthEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._spinbox = QDoubleSpinBox()
        self._spinbox.setMinimum(0)
        self._spinbox.setMaximum(1024)
        self._spinbox.setDecimals(3)

        h = QHBoxLayout()
        h.setContentsMargins(5, 0, 5, 0)
        h.setSpacing(5)
        h.addWidget(QLabel('Length'))
        h.addWidget(self._spinbox)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_value()

        QObject.connect(
                self._spinbox, SIGNAL('valueChanged(double)'), self._value_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

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

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_module',
            'signal_pattern_length',
            'signal_selection',
            'signal_order_list'])
        if not signals.isdisjoint(update_signals):
            self._update_value()

    def _value_changed(self, new_value):
        pattern = self._get_pattern()
        if not pattern:
            return

        length = tstamp.Tstamp(new_value)
        pattern.set_length(length)
        self._updater.signal_update(set(['signal_pattern_length']))


