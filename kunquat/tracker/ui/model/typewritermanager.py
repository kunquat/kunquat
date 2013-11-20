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

import itertools


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

    def _octaves_to_rows(self, octaves):
        notes = list(itertools.chain(*octaves))
        upper = []
        lower = []
        for (i, note) in enumerate(notes):
            if i % 2 == 0:
                lower.append(note)
            else:
                upper.append(note)
        rows = [upper, lower]
        return rows

    def _current_upper_octaves(self, keymap):
        upper_octaves = keymap[self._octave:]
        return upper_octaves

    def _current_lower_octaves(self, keymap):
        key_limit = 14
        lower_octave_candidates = keymap[:self._octave]
        workspace = list(lower_octave_candidates)
        while sum([len(i) for i in workspace]) > 14:
            workspace.pop(0)
        fitting_lower_octaves = workspace
        padding = self._current_upper_octaves(keymap)
        lower_octaves = fitting_lower_octaves + padding
        return lower_octaves

    def _create_current_map(self, keymap):
        upper_octaves = self._current_upper_octaves(keymap)
        lower_octaves = self._current_lower_octaves(keymap)
        (row0, row1) = self._octaves_to_rows(upper_octaves)
        (row2, row3) = self._octaves_to_rows(lower_octaves)
        rows = [row0, row1, row2, row3]
        return rows

    def get_button_pitch(self, coord):
        (row, column) = coord
        keymap_data = self._session.get_keymap_data()
        keymap = keymap_data['keymap']
        current_map = self._create_current_map(keymap)
        pitch_row = current_map[row]
        try:
            pitch = pitch_row[column]
        except IndexError:
            pitch = None
        return pitch
