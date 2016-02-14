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


class KeymapSelect(QComboBox):

    def __init__(self):
        QComboBox.__init__(self)
        self._updater = None
        self._keymap_manager = None
        self._typewriter_manager = None
        self._keymap_catalog = dict()
        QObject.connect(self, SIGNAL("currentIndexChanged(int)"), self._select_keymap)

    def set_ui_model(self, ui_model):
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._keymap_manager = ui_model.get_keymap_manager()
        self._typewriter_manager = ui_model.get_typewriter_manager()
        self._update_keymaps()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        self._update_keymap_texts()

    def _select_keymap(self, catalog_index):
        keymap_id = self._keymap_catalog[catalog_index]
        self._keymap_manager.set_selected_keymap_id(keymap_id)
        keymap_data = self._keymap_manager.get_selected_keymap()
        if keymap_data.get('is_hit_keymap', False):
            base_octave = 0
        else:
            base_octave = keymap_data['base_octave']
        self._typewriter_manager.set_octave(base_octave)
        self._updater.signal_update(set(['signal_select_keymap']))

    def _update_keymap_texts(self):
        for i, keymap_id in self._keymap_catalog.items():
            keymap = self._keymap_manager.get_keymap(keymap_id)
            keymap_name = keymap['name'] or '-'
            text = u'{}'.format(keymap_name)
            self.setItemText(i, text)

    def _update_keymaps(self):
        keymap_ids = self._keymap_manager.get_keymap_ids()
        self._keymap_catalog = dict(enumerate(sorted(keymap_ids)))
        selected_keymap_id = self._keymap_manager.get_selected_keymap_id()
        old_block = self.blockSignals(True)
        self.clear()
        for i, keymap_id in self._keymap_catalog.items():
            self.addItem('')
            if selected_keymap_id and keymap_id == selected_keymap_id:
                self.setCurrentIndex(i)
        self._update_keymap_texts()
        self.blockSignals(old_block)


