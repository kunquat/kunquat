# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class KeymapManager():

    def __init__(self):
        self._session = None
        self._updater = None
        self._keymaps = None
        self._ui_model = None

    def set_controller(self, controller):
        self._session = controller.get_session()
        self._updater = controller.get_updater()
        self._share = controller.get_share()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def get_keymap_ids(self):
        keymaps = self._share.get_keymaps()
        keymap_ids = keymaps.keys()
        return keymap_ids

    def get_selected_keymap_id(self):
        keymap_ids = self.get_keymap_ids()
        selected_id = self._session.get_selected_keymap_id()
        if len(keymap_ids) < 1:
            return None
        if not selected_id in keymap_ids:
            some_id = sorted(keymap_ids)[0]
            return some_id
        return selected_id

    def set_selected_keymap_id(self, keymap_id):
        self._session.set_selected_keymap_id(keymap_id)
        self._updater.signal_update()

    def get_keymap(self, keymap_id):
        keymaps = self._share.get_keymaps()
        keymap = keymaps[keymap_id]
        return keymap

    def get_selected_keymap(self):
        keymap_id = self.get_selected_keymap_id()
        keymap = self.get_keymap(keymap_id)
        return keymap
