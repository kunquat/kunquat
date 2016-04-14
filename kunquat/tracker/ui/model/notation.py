# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from copy import deepcopy
import math


class Notation():

    def __init__(self, notation_id):
        self._controller = None
        self._store = None
        self._share = None
        self._ui_model = None

        is_shared, dict_id = notation_id
        self._is_shared = is_shared
        self._id = dict_id

    def set_controller(self, controller):
        self._controller = controller
        self._store = controller.get_store()
        self._share = controller.get_share()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def get_existence(self):
        if self._is_shared:
            return self._id in self._share.get_notations()
        else:
            return self._id in self._store.get('i_notations.json', [])

    def _get_raw_data(self):
        if self._is_shared:
            return self._share.get_notations()[self._id]
        else:
            return self._store['i_notations.json'][self._id]

    def _set_raw_data(self, raw_data):
        assert not self._is_shared
        notations = self._store['i_notations.json']
        notations[self._id] = raw_data
        self._store['i_notations.json'] = notations
        self._on_data_changed()

    def _on_data_changed(self):
        typewriter_manager = self._ui_model.get_typewriter_manager()
        typewriter_manager.notify_notation_changed((self._is_shared, self._id))

    def get_name(self):
        return self._get_raw_data()['name']

    def set_name(self, name):
        data = deepcopy(self._get_raw_data())
        data['name'] = name
        self._set_raw_data(data)

    def get_octave_count(self):
        return len(self._get_raw_data()['octave_names'])

    def add_octave(self):
        data = deepcopy(self._get_raw_data())
        data['octave_names'].append('{}'.format(self.get_octave_count()))
        self._set_raw_data(data)

    def remove_octave(self, octave_id):
        assert self.get_octave_count() > 1
        data = deepcopy(self._get_raw_data())
        del data['octave_names'][octave_id]
        self._set_raw_data(data)

    def get_octave_name(self, octave_id):
        return self._get_raw_data()['octave_names'][octave_id]

    def set_octave_name(self, octave_id, name):
        data = deepcopy(self._get_raw_data())
        data['octave_names'][octave_id] = name
        self._set_raw_data(data)

    def get_base_octave_id(self):
        return self._get_raw_data()['base_octave']

    def set_base_octave_id(self, octave_id):
        assert 0 <= octave_id < self.get_octave_count()
        data = deepcopy(self._get_raw_data())
        data['base_octave'] = octave_id
        self._set_raw_data(data)

    def get_notes(self):
        return (note for note in self._get_raw_data()['note_names'])

    def get_note(self, index):
        return self._get_raw_data()['note_names'][index]

    def set_note_cents(self, index, cents):
        data = deepcopy(self._get_raw_data())
        data['note_names'][index][0] = cents
        self._set_raw_data(data)

    def set_note_name(self, index, name):
        data = deepcopy(self._get_raw_data())
        data['note_names'][index][1] = name
        self._set_raw_data(data)

    def add_note(self):
        data = self._get_raw_data()
        data['note_names'].append((0, 'n{}'.format(len(data['note_names']))))
        self._set_raw_data(data)

    def remove_note(self, index):
        data = self._get_raw_data()
        del data['note_names'][index]
        self._set_raw_data(data)

    def get_key_count_in_octave(self, octave_id):
        data = self._get_raw_data()
        return len(data['keymap'][octave_id])

    def set_key_count_in_octave(self, octave_id, count):
        data = deepcopy(self._get_raw_data())
        keys = data['keymap'][octave_id]
        cur_count = len(keys)
        if count < cur_count:
            keys[:] = keys[:count]
        elif count > cur_count:
            add_count = count - cur_count
            keys.extend([None] * add_count)
        else:
            return
        self._set_raw_data(data)

    def get_key_cents(self, octave_id, key_index):
        data = self._get_raw_data()
        return data['keymap'][octave_id][key_index]

    def set_key_cents(self, octave_id, key_index, cents):
        data = deepcopy(self._get_raw_data())
        data['keymap'][octave_id][key_index] = cents
        self._set_raw_data(data)

    def get_note_name_and_offset(self, cents):
        nearest = self._get_nearest_note(cents)
        if not nearest:
            return None

        c, name = nearest
        return (name, cents - c)

    def get_full_name(self, cents):
        base, offset = self.get_note_name_and_offset(cents)
        offset_rnd = int(round(offset))
        if offset_rnd != 0:
            name = '{}{:+d}'.format(base, offset_rnd)
        else:
            name = base
        return name

    def get_keymap(self):
        # TODO: clean up the interface mess
        data = self._get_raw_data()
        keymap = {
            'name':        data['name'],
            'base_octave': data['base_octave'],
            'keymap':      data['keymap'],
        }
        return keymap

    def _get_nearest_note(self, cents):
        nearest = None
        nearest_dist = float('inf')

        notes = self._get_raw_data()['note_names']
        for note in notes:
            c, name = note
            dist = abs(c - cents)
            if dist < nearest_dist or (dist == nearest_dist and name < nearest[1]):
                nearest = note
                nearest_dist = dist

        return nearest

    def _get_template_data(self):
        data = self._get_raw_data()
        if 'template' in data:
            return data['template']
        return {
            'center_pitch': [0, 'cents'],
            'octave_ratio': [2, 1],
            'octaves'     : [0, 4, 9],
            'notes'       : [],
        }

    def _set_template_data(self, template_data):
        data = deepcopy(self._get_raw_data())
        data['template'] = template_data
        self._set_raw_data(data)

    def get_template(self):
        return Template(self._get_template_data, self._set_template_data)

    def apply_template_settings(self):
        template = self.get_template()

        # Get center pitch in cents
        center_pitch_value, center_pitch_units = template.get_center_pitch()
        if center_pitch_units == 'cents':
            center = center_pitch_value
        elif center_pitch_units == 'Hz':
            center = math.log(center_pitch_value / 440.0, 2) * 1200

        # Get octave width in cents
        octave_ratio = template.get_octave_ratio()
        if isinstance(octave_ratio, list):
            octave_width = math.log(octave_ratio[0] / float(octave_ratio[1]), 2) * 1200
        else:
            octave_width = octave_ratio

        note_count = template.get_note_count()

        # Get note offsets in cents
        note_offsets = []
        for i in range(note_count):
            ratio = template.get_note_ratio(i)
            if isinstance(ratio, list):
                offset = math.log(ratio[0] / float(ratio[1]), 2) * 1200
            else:
                offset = ratio
            note_offsets.append(offset)

        # Get base names of notes
        base_note_names = []
        for i in range(note_count):
            base_note_names.append(template.get_note_name(i))

        # Choose keyboard layout strategy, TODO: figure out a better solution
        if note_count == 12:
            holes_after = [4, 11]
        elif note_count % 2 == 1:
            holes_after = [note_count - 1]
        else:
            holes_after = []

        # Get octave range
        min_octave, center_octave, max_octave = template.get_octaves()
        center_octave_id = center_octave - min_octave

        # Generate new data
        octave_names = []
        notes = []
        keymap = []
        for octave_id in range(min_octave, max_octave + 1):
            octave_names.append(str(octave_id))

            rel_octave = octave_id - center_octave
            octave_offset = octave_width * rel_octave
            cur_octave_cents = []
            for note_offset, base_name in zip(note_offsets, base_note_names):
                cents = center + octave_offset + note_offset
                name = '{}{}'.format(base_name, octave_id)
                notes.append([cents, name])
                cur_octave_cents.append(cents)

            key_row = []
            for i, cents in enumerate(cur_octave_cents):
                key_row.append(cents)
                if i in holes_after:
                    key_row.append(None)
            keymap.append(key_row)

        # Set new data
        data = deepcopy(self._get_raw_data())
        data['octave_names'] = octave_names
        data['base_octave'] = center_octave_id
        data['note_names'] = notes
        data['keymap'] = keymap
        self._set_raw_data(data)


class Template():

    def __init__(self, get_data, set_data):
        self._get_data = get_data
        self._set_data = set_data

    def get_center_pitch(self):
        return self._get_data()['center_pitch']

    def set_center_pitch(self, value, units):
        data = deepcopy(self._get_data())
        data['center_pitch'] = [value, units]
        self._set_data(data)

    def get_octave_ratio(self):
        return self._get_data()['octave_ratio']

    def set_octave_ratio(self, ratio):
        data = deepcopy(self._get_data())
        data['octave_ratio'] = ratio
        self._set_data(data)

    def get_octaves(self):
        return self._get_data()['octaves']

    def set_octaves(self, lowest, center, highest):
        data = deepcopy(self._get_data())
        data['octaves'] = [lowest, center, highest]
        self._set_data(data)

    def get_note_count(self):
        return len(self._get_data()['notes'])

    def get_note_name(self, index):
        return self._get_data()['notes'][index][0]

    def add_note(self):
        data = deepcopy(self._get_data())
        data['notes'].append(['(n)', 0])
        self._set_data(data)

    def remove_note(self, index):
        data = deepcopy(self._get_data())
        del data['notes'][index]
        self._set_data(data)

    def set_note_name(self, index, name):
        data = deepcopy(self._get_data())
        data['notes'][index][0] = name
        self._set_data(data)

    def get_note_ratio(self, index):
        return self._get_data()['notes'][index][1]

    def set_note_ratio(self, index, ratio):
        data = deepcopy(self._get_data())
        data['notes'][index][1] = ratio
        self._set_data(data)


