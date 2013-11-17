# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
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


class InstrumentSelect(QComboBox):

    def __init__(self):
        QComboBox.__init__(self)
        self._ui_manager = None
        self._module = None
        self._slot_catalog = dict()
        QObject.connect(self, SIGNAL("currentIndexChanged(int)"), self._select_instrument)

    def set_ui_model(self, ui_model):
        updater = ui_model.get_updater()
        updater.register_updater(self.perform_updates)
        self._ui_manager = ui_model.get_ui_manager()
        self._module = ui_model.get_module()

    def _select_instrument(self, catalog_index):
        slot_id = self._slot_catalog[catalog_index]
        self._ui_manager.set_selected_slot_id(slot_id)

    def update_slot_texts(self):
        for i, slot_id in self._slot_catalog.items():
            parts = slot_id.split('_')
            second = parts[1]
            slot_number = int(second)
            slot = self._module.get_slot(slot_id)
            instrument = slot.get_instrument()
            instrument_name = instrument.get_name() or '-'
            play = '' if len(slot.get_active_notes()) < 1 else u'*'
            text = 'instrument %s: %s %s' % (slot_number, instrument_name, play)
            self.setItemText(i, text)

    def update_slots(self):
        slot_ids = self._module.get_slot_ids()
        self._slot_catalog = dict(enumerate(sorted(slot_ids)))
        selected_slot_id = self._ui_manager.get_selected_slot_id()
        old_block = self.blockSignals(True)
        self.clear()
        for i, slot_id in self._slot_catalog.items():
            self.addItem('')
            if selected_slot_id and slot_id == selected_slot_id:
                self.setCurrentIndex(i)
        self.update_slot_texts()
        self.blockSignals(old_block)

    def perform_updates(self, signals):
        if 'signal_slots' in signals:
            self.update_slots()
        self.update_slot_texts()

