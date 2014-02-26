# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import itertools
from bisect import bisect_left

from octavebuttonmodel import OctaveButtonModel
from typewriterbuttonmodel import TypewriterButtonModel


class TypewriterManager():

    _ROW_LENGTHS = [9, 10, 7, 7]

    def __init__(self):
        self._controller = None
        self._session = None
        self._share = None
        self._updater = None
        self._ui_model = None

        self._current_map = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._share = controller.get_share()
        self._updater = controller.get_updater()

        keymap_name = self._session.get_keymap_name()
        keymap_data = self._share.get_keymaps()[keymap_name]
        base_octave = keymap_data['base_octave']
        self.set_octave(base_octave)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def get_button_model(self, row, index):
        button_model = TypewriterButtonModel(row, index)
        button_model.set_controller(self._controller)
        button_model.set_ui_model(self._ui_model)
        return button_model

    def get_octave_button_model(self, octave_id):
        ob_model = OctaveButtonModel(octave_id)
        ob_model.set_controller(self._controller)
        ob_model.set_ui_model(self._ui_model)
        return ob_model

    def get_row_count(self):
        return len(self._ROW_LENGTHS)

    def get_button_count_at_row(self, row):
        return self._ROW_LENGTHS[row]

    def get_pad_factor_at_row(self, row):
        pads = [1, 0, 2, 1]
        return pads[row]

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
        upper_octaves = keymap[self.get_octave():]
        return upper_octaves

    def _current_lower_octaves(self, keymap):
        key_limit = 14
        lower_octave_candidates = keymap[:self.get_octave()]
        workspace = list(lower_octave_candidates)
        while sum([len(i) for i in workspace]) > 14:
            workspace.pop(0)
        fitting_lower_octaves = workspace
        padding = self._current_upper_octaves(keymap)
        lower_octaves = fitting_lower_octaves + padding
        return lower_octaves

    def _create_current_map(self, keymap):
        if self._current_map != None:
            return

        upper_octaves = self._current_upper_octaves(keymap)
        lower_octaves = self._current_lower_octaves(keymap)
        (row0, row1) = self._octaves_to_rows(upper_octaves)
        (row2, row3) = self._octaves_to_rows(lower_octaves)
        rows = [row0, row1, row2, row3]
        self._current_map = rows

    def get_button_pitch(self, coord):
        (row, column) = coord
        keymap_name = self._session.get_keymap_name()
        keymap_data = self._share.get_keymaps()[keymap_name]
        keymap = keymap_data['keymap']
        self._create_current_map(keymap)
        pitch_row = self._current_map[row]
        try:
            pitch = pitch_row[column]
        except IndexError:
            pitch = None
        return pitch

    def get_pitches_by_octave(self, octave_id):
        keymap_name = self._session.get_keymap_name()
        keymap_data = self._share.get_keymaps()[keymap_name]
        octaves = keymap_data['keymap']
        octave = octaves[octave_id]
        pitches = set(octave)
        return pitches

    def get_pitches(self):
        keymap_name = self._session.get_keymap_name()
        keymap_data = self._share.get_keymaps()[keymap_name]
        octaves = keymap_data['keymap']
        pitches = set()
        for pitch in itertools.chain(*octaves):
            if pitch != None:
                pitches.add(pitch)
        return pitches

    def get_closest_keymap_pitch(self, pitch):
        pitches = sorted(self.get_pitches())
        key_count = len(pitches)
        i = bisect_left(pitches, pitch)
        if i == key_count:
            return pitches[-1]
        elif i == 0:
            return pitches[0]
        else:
            a = pitches[i]
            b = pitches[i - 1]
            if abs(a - pitch) < abs(b - pitch):
                return a
            else:
                return b

    def get_octave_count(self):
        keymap_name = self._session.get_keymap_name()
        keymap_data = self._share.get_keymaps()[keymap_name]
        keymap = keymap_data['keymap']
        octave_count = len(keymap)
        return octave_count

    def get_octave(self):
        return self._session.get_octave_id()

    def set_octave(self, octave_id):
        self._session.set_octave_id(octave_id)
        self._current_map = None
        self._updater.signal_update(set(['signal_octave']))


