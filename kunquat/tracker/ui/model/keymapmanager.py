# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2014
#          Tomi Jylhä-Ollila, Finland 2016-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class HitKeymapID:
    pass


class KeyboardAction():

    NOTE = 'note'
    OCTAVE_DOWN = 'octave_down'
    OCTAVE_UP = 'octave_up'
    PLAY = 'play'
    REST = 'rest'
    SILENCE = 'silence'

    def __init__(self, action_type):
        super().__init__()
        self.action_type = action_type

    def __eq__(self, other):
        return (self.action_type == other.action_type)

    def __hash__(self):
        return hash(self.action_type)


class KeyboardNoteAction(KeyboardAction):

    def __init__(self, row, index):
        super().__init__(KeyboardAction.NOTE)
        self.row = row
        self.index = index

    def _get_fields(self):
        return (self.action_type, self.row, self.index)

    def __eq__(self, other):
        if not isinstance(other, KeyboardNoteAction):
            return False
        return (self._get_fields() == other._get_fields())

    def __hash__(self):
        return hash(self._get_fields())


_hit_keymap = {
    'is_hit_keymap': True,
    'name': 'Hits',
    'keymap': [
        [0, 7, 1, 8, 2, 9, 3, 10, 4, 11, 5, 12, 6, 13,
            14, 23, 15, 24, 16, 25, 17, 26, 18, 27, 19, 28, 20, 29, 21, 30, 22, 31],
        [32, 39, 33, 40, 34, 41, 35, 42, 36, 43, 37, 44, 38, 45,
            46, 55, 47, 56, 48, 57, 49, 58, 50, 59, 51, 60, 52, 61, 53, 62, 54, 63],
        [64, 71, 65, 72, 66, 73, 67, 74, 68, 75, 69, 76, 70, 77,
            78, 87, 79, 88, 80, 89, 81, 90, 82, 91, 83, 92, 84, 93, 85, 94, 86, 95],
        [96, 103, 97, 104, 98, 105, 99, 106, 100, 107, 101, 108, 102, 109,
            110, 119, 111, 120, 112, 121, 113, 122, 114, 123, 115, 124, 116, 125,
            117, 126, 118, 127],
    ],
}


class KeymapManager():

    def __init__(self):
        self._session = None
        self._updater = None
        self._ui_model = None

    def set_controller(self, controller):
        self._session = controller.get_session()
        self._updater = controller.get_updater()
        self._share = controller.get_share()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def _are_keymap_actions_valid(self, actions):
        ka = KeyboardAction

        req_single_action_types = (
                ka.OCTAVE_DOWN, ka.OCTAVE_UP, ka.PLAY, ka.REST, ka.SILENCE)
        for action_type in req_single_action_types:
            if ka(action_type) not in actions:
                return False

        note_actions = set(a for a in actions if a.action_type == ka.NOTE)
        if len(note_actions) != 33:
            return False

        return True

    def get_keyboard_row_sizes(self):
        # The number of buttons provided for configuration on each row
        # On a QWERTY layout, the leftmost buttons are: 1, Q, A, Z
        return (11, 11, 11, 10)

    def get_typewriter_row_sizes(self):
        return (9, 10, 7, 7)

    def get_typewriter_row_offsets(self):
        return (1, 0, 1, 0)

    def _is_row_layout_valid(self, locs):
        row_sizes = self.get_keyboard_row_sizes()

        used_locs = set()
        for loc in locs:
            if loc in used_locs:
                return False
            used_locs.add(loc)

            row, index = loc
            if not (0 <= row < len(row_sizes)):
                return False
            if not (0 <= index < row_sizes[row]):
                return False

        return True

    def set_key_actions(self, actions):
        assert self._is_row_layout_valid(actions.keys())
        assert self._are_keymap_actions_valid(actions.values())
        self._session.keyboard_key_actions = actions

        action_locations = { act: loc for (loc, act) in actions.items() }
        self._session.keyboard_action_locations = action_locations

    def set_key_names(self, names):
        assert self._is_row_layout_valid(names.keys())
        self._session.keyboard_key_names = names

    def set_scancode_locations(self, codes_to_locs):
        assert self._is_row_layout_valid(codes_to_locs.values())
        self._session.keyboard_scancode_locations = codes_to_locs

    def set_key_id_locations(self, ids_to_locs):
        assert self._is_row_layout_valid(ids_to_locs.values())
        self._session.keyboard_id_locations = ids_to_locs

    def get_scancode_location(self, code):
        return self._session.keyboard_scancode_locations.get(code, None)

    def get_key_id_location(self, key_id):
        return self._session.keyboard_id_locations.get(key_id, None)

    def get_key_action(self, location):
        return self._session.keyboard_key_actions.get(location, None)

    def get_key_name(self, location):
        return self._session.keyboard_key_names.get(location, None)

    def get_action_location(self, action):
        if not isinstance(action, KeyboardAction):
            assert action != KeyboardAction.NOTE
            action = KeyboardAction(action)
        return self._session.keyboard_action_locations.get(action, None)

    def _get_keymap_ids(self):
        notation_mgr = self._ui_model.get_notation_manager()
        keymap_ids = notation_mgr.get_notation_ids()
        keymap_ids.append(HitKeymapID)
        return keymap_ids

    def _get_some_keymap_id(self):
        keymap_ids = self._get_keymap_ids()
        if len(keymap_ids) < 2:
            return HitKeymapID
        some_id = sorted(keymap_ids)[1]
        return some_id

    def get_selected_keymap_id(self):
        #keymap_ids = self.get_keymap_ids()
        selected_id = self._session.get_selected_notation_id() or self._get_some_keymap_id()
        return selected_id

    def is_hit_keymap_active(self):
        return self._session.is_hit_keymap_active()

    def get_selected_keymap(self):
        if self.is_hit_keymap_active():
            return _hit_keymap
        notation_mgr = self._ui_model.get_notation_manager()
        notation = notation_mgr.get_selected_notation()
        return notation.get_keymap()

    def set_hit_keymap_active(self, active):
        self._session.set_hit_keymap_active(active)

        keymap_data = self.get_selected_keymap()
        if keymap_data.get('is_hit_keymap', False):
            base_octave = 0
        else:
            base_octave = keymap_data['base_octave']

        typewriter_mgr = self._ui_model.get_typewriter_manager()
        typewriter_mgr.set_octave(base_octave)


