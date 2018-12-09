# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2014
#          Tomi Jylhä-Ollila, Finland 2016-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .kqtcombobox import KqtComboBox
from .updater import Updater


class NotationSelect(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._notation_mgr = None
        self._typewriter_mgr = None
        self._notation_catalogue = {}

        self._notations = KqtComboBox()
        self._notations.setSizeAdjustPolicy(QComboBox.AdjustToContents)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(QLabel('Notation:'))
        h.addWidget(self._notations)
        self.setLayout(h)

    def _on_setup(self):
        self._notation_mgr = self._ui_model.get_notation_manager()
        self._typewriter_mgr = self._ui_model.get_typewriter_manager()

        self.register_action('signal_module', self._update_notations)
        self.register_action('signal_notation', self._update_notations)
        self.register_action('signal_notation_list', self._update_notations)
        self.register_action('signal_select_keymap', self._update_enabled)

        self._notations.currentIndexChanged.connect(self._select_notation)

        self._update_enabled()
        self._update_notations()

    def _select_notation(self, catalogue_index):
        old_octave_id = self._typewriter_mgr.get_octave()

        notation_id = self._notation_catalogue[catalogue_index]
        self._notation_mgr.set_selected_notation_id(notation_id)

        signals = ['signal_notation']

        notation = self._notation_mgr.get_selected_notation()
        new_octave_id = min(old_octave_id, notation.get_octave_count() - 1)
        if new_octave_id != old_octave_id:
            self._typewriter_mgr.set_octave(new_octave_id)
            signals.append('signal_octave')

        self._updater.signal_update(*signals)

    def _update_enabled(self):
        keymap_mgr = self._ui_model.get_keymap_manager()
        self.setEnabled(not keymap_mgr.is_hit_keymap_active())

    def _update_notation_texts(self):
        for i, notation_id in self._notation_catalogue.items():
            notation = self._notation_mgr.get_notation(notation_id)
            notation_name = notation.get_name() or '-'
            self._notations.setItemText(i, notation_name)

    def _update_notations(self):
        notation_ids = sorted(self._notation_mgr.get_all_notation_ids())
        self._notation_catalogue = dict(enumerate(notation_ids))

        selected_notation_id = self._notation_mgr.get_selected_notation_id()
        try:
            selected_index = notation_ids.index(selected_notation_id)
        except ValueError:
            selected_index = -1

        old_block = self._notations.blockSignals(True)
        self._notations.set_items('' for _ in notation_ids)
        self._notations.setCurrentIndex(selected_index)
        self._update_notation_texts()
        self._notations.blockSignals(old_block)


