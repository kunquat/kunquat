# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2014
#          Tomi Jylhä-Ollila, Finland 2016
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


class NotationSelect(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._notation_manager = None
        self._typewriter_manager = None
        self._notation_catalog = dict()

        self._notations = QComboBox()
        self._notations.setSizeAdjustPolicy(QComboBox.AdjustToContents)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(QLabel('Notation:'))
        h.addWidget(self._notations)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._notation_manager = ui_model.get_notation_manager()
        self._typewriter_manager = ui_model.get_typewriter_manager()

        QObject.connect(
                self._notations,
                SIGNAL("currentIndexChanged(int)"),
                self._select_notation)

        self._update_enabled()
        self._update_notations()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set(['signal_module', 'signal_notation_list'])
        if not signals.isdisjoint(update_signals):
            self._update_notations()

        if 'signal_select_keymap' in signals:
            self._update_enabled()

    def _select_notation(self, catalog_index):
        old_octave_id = self._typewriter_manager.get_octave()

        notation_id = self._notation_catalog[catalog_index]
        self._notation_manager.set_selected_notation_id(notation_id)

        signals = set(['signal_notation'])

        notation = self._notation_manager.get_selected_notation()
        new_octave_id = min(old_octave_id, notation.get_octave_count() - 1)
        if new_octave_id != old_octave_id:
            self._typewriter_manager.set_octave(new_octave_id)
            signals.add('signal_octave')

        self._updater.signal_update(signals)

    def _update_enabled(self):
        keymap_manager = self._ui_model.get_keymap_manager()
        self.setEnabled(not keymap_manager.is_hit_keymap_active())

    def _update_notation_texts(self):
        for i, notation_id in self._notation_catalog.items():
            notation = self._notation_manager.get_notation(notation_id)
            notation_name = notation.get_name() or '-'
            self._notations.setItemText(i, notation_name)

    def _update_notations(self):
        notation_ids = self._notation_manager.get_all_notation_ids()
        self._notation_catalog = dict(enumerate(sorted(notation_ids)))
        selected_notation_id = self._notation_manager.get_selected_notation_id()
        old_block = self._notations.blockSignals(True)
        self._notations.clear()
        for i, notation_id in self._notation_catalog.items():
            self._notations.addItem('')
            if selected_notation_id and notation_id == selected_notation_id:
                self._notations.setCurrentIndex(i)
        self._update_notation_texts()
        self._notations.blockSignals(old_block)


