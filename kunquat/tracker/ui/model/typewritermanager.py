# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2014-2017
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

from .octavebuttonmodel import OctaveButtonModel
from .typewriterbuttonmodel import TypewriterButtonModel


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
        self._upper_key_ids = None
        self._lower_key_ids = None
        self._pitch_key_info = None
        self._pitch_key_info_version = None

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
            active_coords.extend((ri, ci) for ci in range(row_length)
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

    def _get_key_pitches(self, keymap, key_id_lists):
        pitch_lists = []
        for key_id_list in key_id_lists:
            pitches = [keymap[o][i] for (o, i) in key_id_list]
            pitch_lists.append(pitches)
        return pitch_lists

    def _get_key_id_map(self, key_id_lists):
        key_id_map = []
        for key_id_list in key_id_lists:
            key_id_map.extend(key_id_list)
        return key_id_map

    def _current_upper_octaves(self, keymap):
        lower_key_limit = sum(self._ROW_LENGTHS[2:4])
        upper_key_limit = sum(self._ROW_LENGTHS[0:2])

        cur_octave_index = self.get_octave()
        if len(keymap[cur_octave_index]) > upper_key_limit:
            upper_octaves = [[(cur_octave_index, i)
                    for i in range(lower_key_limit, len(keymap[cur_octave_index]))]]
        else:
            upper_octaves = []
            for octave_index in range(cur_octave_index, len(keymap)):
                octave = [(octave_index, i) for i in range(len(keymap[octave_index]))]
                upper_octaves.append(octave)

        return upper_octaves

    def _current_lower_octaves(self, keymap):
        lower_key_limit = sum(self._ROW_LENGTHS[2:4])
        upper_key_limit = sum(self._ROW_LENGTHS[0:2])

        cur_octave_index = self.get_octave()
        if len(keymap[cur_octave_index]) > upper_key_limit:
            # Use lower part of the keyboard for very large octaves
            fitting_lower_octaves = [[(cur_octave_index, i)
                    for i in range(lower_key_limit)]]
        else:
            # Find lower octaves that fit inside the lower part of the keyboard
            lower_octave_candidates = []
            for octave_index, pitches in enumerate(keymap[:cur_octave_index]):
                octave = [(octave_index, i) for i in range(len(pitches))]
                lower_octave_candidates.append(octave)
            workspace = list(lower_octave_candidates) # copy
            while sum([len(i) for i in workspace]) > lower_key_limit:
                workspace.pop(0)
            fitting_lower_octaves = workspace

        return fitting_lower_octaves

    def _create_current_map(self, keymap):
        keymap_id = self._keymap_manager.get_selected_keymap_id()
        if self._current_map_version == keymap_id:
            return
        self._current_map_version = keymap_id
        upper_key_id_lists = self._current_upper_octaves(keymap)
        lower_key_id_lists = self._current_lower_octaves(keymap)
        self._upper_key_ids = self._get_key_id_map(upper_key_id_lists)
        self._lower_key_ids = self._get_key_id_map(lower_key_id_lists)
        upper_octaves = self._get_key_pitches(keymap, upper_key_id_lists)
        lower_octaves = self._get_key_pitches(keymap, lower_key_id_lists)
        (row0, row1) = self._octaves_to_rows(upper_octaves)
        (row2, row3) = self._octaves_to_rows(lower_octaves)
        rows = [row0, row1, row2, row3]
        self._current_map = rows

    def _get_button_param(self, coords, get_pitch):
        (row, column) = coords
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

    def get_button_pitch(self, coords):
        return self._get_button_param(coords, get_pitch=True)

    def get_button_hit(self, coords):
        return self._get_button_param(coords, get_pitch=False)

    def get_key_id(self, coords):
        (row, column) = coords
        keymap_data = self._keymap_manager.get_selected_keymap()
        keymap = keymap_data['keymap']
        self._create_current_map(keymap)

        key_index = column * 2
        if row % 2 == 0:
            key_index += 1

        key_ids = self._upper_key_ids if row in (0, 1) else self._lower_key_ids
        if key_index >= len(key_ids):
            return None
        return key_ids[key_index]

    def get_pitches_by_octave(self, octave_id):
        keymap_data = self._keymap_manager.get_selected_keymap()
        octaves = keymap_data['keymap']
        octave = octaves[octave_id]
        pitches = set(octave)
        return pitches

    def _get_pitch_key_info(self):
        keymap_data = self._keymap_manager.get_selected_keymap()
        octaves = keymap_data['keymap']
        pitches = set()
        for octave_id, keymap_pitches in enumerate(octaves):
            for key_index, pitch in enumerate(keymap_pitches):
                if pitch != None:
                    pitches.add((pitch, (octave_id, key_index)))
        return pitches

    def get_nearest_key_id(self, pitch):
        keymap_id = self._keymap_manager.get_selected_keymap_id()
        if self._pitch_key_info_version != keymap_id:
            self._pitch_key_info_version = keymap_id
            self._pitch_key_info = sorted(self._get_pitch_key_info(), key=lambda x: x[0])
        key_count = len(self._pitch_key_info)
        i = bisect_left(self._pitch_key_info, (pitch, (float('-inf'),)))
        if i == key_count:
            _, key_id = self._pitch_key_info[-1]
            return key_id
        elif i == 0:
            _, key_id = self._pitch_key_info[0]
            return key_id
        else:
            pitch_a, key_id_a = self._pitch_key_info[i]
            pitch_b, key_id_b = self._pitch_key_info[i - 1]
            if abs(pitch_a - pitch) < abs(pitch_b - pitch):
                return key_id_a
            else:
                return key_id_b

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
        self._updater.signal_update('signal_octave')

    def notify_notation_changed(self, notation_id):
        if self._current_map_version == notation_id:
            self._current_map_version = None
        if self._pitch_key_info_version == notation_id:
            self._pitch_key_info_version = None


