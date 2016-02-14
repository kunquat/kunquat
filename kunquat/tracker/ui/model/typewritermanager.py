# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import random
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
        self._keymap_manager = None

        # Cached data
        self._current_map = None
        self._current_map_version = None
        self._pitches = None
        self._pitches_version = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._share = controller.get_share()
        self._updater = controller.get_updater()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._keymap_manager = ui_model.get_keymap_manager()

    def get_button_model(self, row, index):
        button_model = TypewriterButtonModel(row, index)
        button_model.set_controller(self._controller)
        button_model.set_ui_model(self._ui_model)
        return button_model

    def get_random_button_model(self):
        active_coords = []
        for ri, row_length in enumerate(self._ROW_LENGTHS):
            active_coords.extend((ri, ci) for ci in xrange(row_length)
                    if self.get_button_pitch((ri, ci)) != None)
        if not active_coords:
            return None
        row, index = random.choice(active_coords)
        return self.get_button_model(row, index)

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
        lower_key_limit = sum(self._ROW_LENGTHS[2:4])
        upper_key_limit = sum(self._ROW_LENGTHS[0:2])

        cur_octave_index = self.get_octave()
        if len(keymap[cur_octave_index]) > upper_key_limit:
            upper_octaves = [keymap[cur_octave_index][lower_key_limit:]]
        else:
            upper_octaves = keymap[cur_octave_index:]

        return upper_octaves

    def _current_lower_octaves(self, keymap):
        lower_key_limit = sum(self._ROW_LENGTHS[2:4])
        upper_key_limit = sum(self._ROW_LENGTHS[0:2])

        cur_octave_index = self.get_octave()
        if len(keymap[cur_octave_index]) > upper_key_limit:
            # Use lower part of the keyboard for very large octaves
            fitting_lower_octaves = [keymap[cur_octave_index][:lower_key_limit]]
        else:
            # Find lower octaves that fit inside the lower part of the keyboard
            lower_octave_candidates = keymap[:cur_octave_index]
            workspace = list(lower_octave_candidates) # copy
            while sum([len(i) for i in workspace]) > lower_key_limit:
                workspace.pop(0)
            fitting_lower_octaves = workspace

        grey_key = None
        padding_octave = lower_key_limit * [grey_key]
        padding_octaves = [padding_octave]
        lower_octaves = fitting_lower_octaves + padding_octaves
        return lower_octaves

    def _create_current_map(self, keymap):
        keymap_id = self._keymap_manager.get_selected_keymap_id()
        if self._current_map_version == keymap_id:
            return
        self._current_map_version = keymap_id
        upper_octaves = self._current_upper_octaves(keymap)
        lower_octaves = self._current_lower_octaves(keymap)
        (row0, row1) = self._octaves_to_rows(upper_octaves)
        (row2, row3) = self._octaves_to_rows(lower_octaves)
        rows = [row0, row1, row2, row3]
        self._current_map = rows

    def _get_button_param(self, coord, get_pitch):
        (row, column) = coord
        keymap_data = self._keymap_manager.get_selected_keymap()
        keymap = keymap_data['keymap']
        if keymap_data.get('is_hit_keymap', False) == get_pitch:
            return None
        self._create_current_map(keymap)
        param_row = self._current_map[row]
        try:
            param = param_row[column]
        except IndexError:
            param = None
        return param

    def get_button_pitch(self, coord):
        return self._get_button_param(coord, get_pitch=True)

    def get_button_hit(self, coord):
        return self._get_button_param(coord, get_pitch=False)

    def get_pitches_by_octave(self, octave_id):
        keymap_data = self._keymap_manager.get_selected_keymap()
        octaves = keymap_data['keymap']
        octave = octaves[octave_id]
        pitches = set(octave)
        return pitches

    def _get_pitches(self):
        keymap_data = self._keymap_manager.get_selected_keymap()
        octaves = keymap_data['keymap']
        pitches = set()
        for pitch in itertools.chain(*octaves):
            if pitch != None:
                pitches.add(pitch)
        return pitches

    def get_closest_keymap_pitch(self, pitch):
        keymap_id = self._keymap_manager.get_selected_keymap_id()
        if self._pitches_version != keymap_id:
            self._pitches_version = keymap_id
            self._pitches = sorted(self._get_pitches())
        key_count = len(self._pitches)
        i = bisect_left(self._pitches, pitch)
        if i == key_count:
            return self._pitches[-1]
        elif i == 0:
            return self._pitches[0]
        else:
            a = self._pitches[i]
            b = self._pitches[i - 1]
            if abs(a - pitch) < abs(b - pitch):
                return a
            else:
                return b

    def get_octave_count(self):
        keymap_data = self._keymap_manager.get_selected_keymap()
        keymap = keymap_data['keymap']
        octave_count = len(keymap)
        return octave_count

    def get_octave(self):
        selected = self._session.get_octave_id()
        if selected == None:
            keymap_data = self._keymap_manager.get_selected_keymap()
            if keymap_data.get('is_hit_keymap', False):
                base_octave = 0
            else:
                base_octave = keymap_data['base_octave']
            return base_octave
        return selected

    def set_octave(self, octave_id):
        self._session.set_octave_id(octave_id)
        self._current_map_version = None
        self._updater.signal_update(set(['signal_octave']))


