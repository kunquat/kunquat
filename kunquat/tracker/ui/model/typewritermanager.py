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

class TypewriterManager():

    def __init__(self):
        self._controller = None
        self._session = None
        self._octave = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        keymap_data = self._session.get_keymap_data()
        self._octave = keymap_data['base_octave']

    def get_button_pitch(self, coord):
        keymap_data = self._session.get_keymap_data()
        keymap = keymap_data['keymap']
        (x, y) = coord
        pitch = keymap[self._octave][y]
        return pitch
