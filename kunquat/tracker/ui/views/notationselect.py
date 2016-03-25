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


class NotationSelect(QComboBox):

    def __init__(self):
        QComboBox.__init__(self)
        self._updater = None
        self._notation_manager = None
        self._typewriter_manager = None
        self._notation_catalog = dict()

        self.setSizeAdjustPolicy(QComboBox.AdjustToContents)

        QObject.connect(self, SIGNAL("currentIndexChanged(int)"), self._select_notation)

    def set_ui_model(self, ui_model):
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._notation_manager = ui_model.get_notation_manager()
        self._typewriter_manager = ui_model.get_typewriter_manager()

        self._update_notations()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_notation_list' in signals:
            self._update_notations()

    def _select_notation(self, catalog_index):
        notation_id = self._notation_catalog[catalog_index]
        self._notation_manager.set_selected_notation_id(notation_id)
        self._updater.signal_update(set(['signal_notation']))

    def _update_notation_texts(self):
        for i, notation_id in self._notation_catalog.items():
            notation = self._notation_manager.get_notation(notation_id)
            notation_name = notation.get_name() or '-'
            self.setItemText(i, notation_name)

    def _update_notations(self):
        notation_ids = self._notation_manager.get_all_notation_ids()
        self._notation_catalog = dict(enumerate(sorted(notation_ids)))
        selected_notation_id = self._notation_manager.get_selected_notation_id()
        old_block = self.blockSignals(True)
        self.clear()
        for i, notation_id in self._notation_catalog.items():
            self.addItem('')
            if selected_notation_id and notation_id == selected_notation_id:
                self.setCurrentIndex(i)
        self._update_notation_texts()
        self.blockSignals(old_block)


