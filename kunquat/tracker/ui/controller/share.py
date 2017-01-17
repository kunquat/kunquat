# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2017
#          Toni Ruottu, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import json
import glob
import os.path

from .kqtifile import KqtiFile


class Share():

    def __init__(self, path):
        self._path = path
        self._instruments_path = os.path.join(self._path, 'instruments')
        self._icons_path = os.path.join(self._path, 'icons')
        self._keymaps_path = os.path.join(self._path, 'keymaps')
        self._notations_path = os.path.join(self._path, 'notations')
        self._styles_path = os.path.join(self._path, 'styles')

        self._notations = {}
        self._read_notations()

    def _read_notations(self):
        notation_paths = glob.glob(os.path.join(self._notations_path, '*.json'))
        for path in notation_paths:
            base_name = os.path.basename(path)
            key = '.'.join(base_name.split('.')[:-1]) # strip the .json suffix
            with open(path, encoding='utf-8') as f:
                try:
                    unsafe_data = json.load(f)
                except json.JSONDecodeError:
                    continue
                notation = self._get_validated_notation(unsafe_data)
                if notation:
                    self._notations[key] = notation

    def _get_validated_notation(self, unsafe_data):
        notation = {}

        # Name of the notation
        name = unsafe_data.get('name', None)
        if not isinstance(name, str):
            return None
        notation['name'] = name

        # Octave names
        unsafe_octave_names = unsafe_data.get('octave_names', None)
        if not isinstance(unsafe_octave_names, list) or (len(unsafe_octave_names) == 0):
            return None
        octave_names = []
        for unsafe_name in unsafe_octave_names:
            if not isinstance(unsafe_name, str):
                return None
            octave_names.append(unsafe_name)
        notation['octave_names'] = octave_names

        # Base octave
        base_octave = unsafe_data.get('base_octave', None)
        if not isinstance(base_octave, int):
            return None
        if not 0 <= base_octave < len(octave_names):
            return None
        notation['base_octave'] = base_octave

        # Note names
        unsafe_note_names = unsafe_data.get('note_names', None)
        if not isinstance(unsafe_note_names, list) or (len(unsafe_note_names) == 0):
            return None
        note_names = []
        for unsafe_desc in unsafe_note_names:
            if (not isinstance(unsafe_desc, list)) or (len(unsafe_desc) != 2):
                return None
            unsafe_cents, unsafe_name = unsafe_desc
            if not isinstance(unsafe_cents, (int, float)):
                return None
            if not isinstance(unsafe_name, str):
                return None
            desc = [unsafe_cents, unsafe_name]
            note_names.append(desc)
        notation['note_names'] = note_names

        # Keymap
        unsafe_keymap = unsafe_data.get('keymap', None)
        if not isinstance(unsafe_keymap, list):
            return None
        keymap = []
        for unsafe_octave in unsafe_keymap:
            if not isinstance(unsafe_octave, list):
                return None
            octave = []
            for unsafe_entry in unsafe_octave:
                if not isinstance(unsafe_entry, (int, float, type(None))):
                    return None
                octave.append(unsafe_entry)
            keymap.append(octave)
        notation['keymap'] = keymap

        # Optional template
        if 'template' in unsafe_data:
            unsafe_template = unsafe_data['template']
            if not isinstance(unsafe_template, dict):
                return None
            template = {}

            # Centre pitch
            centre_pitch = unsafe_template.get('centre_pitch', None)
            if not isinstance(centre_pitch, list):
                return None
            if len(centre_pitch) != 2:
                return None
            centre, units = centre_pitch
            if not isinstance(centre, (int, float)):
                return None
            if units == 'cents':
                if not -9999 <= centre <= 9999:
                    return None
            elif units == 'Hz':
                if not 1 <= centre <= 20000:
                    return None
            else:
                return None
            template['centre_pitch'] = centre_pitch

            def _get_validated_ratio(parts):
                if len(parts) != 2:
                    return None
                if not all(isinstance(n, int) for n in parts):
                    return None
                if parts[1] <= 0:
                    return None
                return parts

            # Octave ratio
            octave_ratio = unsafe_template.get('octave_ratio', None)
            if isinstance(octave_ratio, list):
                octave_ratio = _get_validated_ratio(octave_ratio)
                if not octave_ratio:
                    return None
                if abs(octave_ratio[0]) <= octave_ratio[1]:
                    return None
            elif isinstance(octave_ratio, (int, float)):
                if not 0 < octave_ratio <= 9999:
                    return None
            else:
                return None
            template['octave_ratio'] = octave_ratio

            # Octave range
            octaves = unsafe_template.get('octaves', None)
            if not isinstance(octaves, list) or len(octaves) != 3:
                return None
            if not all(isinstance(o, int) for o in octaves):
                return None
            if not all(0 <= 0 < 16 for o in octaves):
                return None
            if not (octaves[0] <= octaves[1] <= octaves[2]):
                return None
            template['octaves'] = octaves

            # Notes
            notes = unsafe_template.get('notes', None)
            if not isinstance(notes, list):
                return None
            for note in notes:
                if not isinstance(note, list) or len(note) != 2:
                    return None
                name, ratio = note

                if not isinstance(name, str):
                    return None

                if isinstance(ratio, list):
                    ratio = _get_validated_ratio(ratio)
                    if not ratio:
                        return None
                elif isinstance(ratio, (int, float)):
                    if not -9999 <= ratio <= 9999:
                        return None
                else:
                    return None

            template['notes'] = notes

            notation['template'] = template

        return notation

    def get_keymaps(self):
        return self._keymaps

    def get_notations(self):
        return self._notations

    def get_default_instrument(self):
        path = os.path.join(self._instruments_path, 'example_ins.kqti.bz2')
        kqtifile = KqtiFile(path)
        return kqtifile

    def get_kunquat_logo_path(self):
        path = os.path.join(self._icons_path, 'kunquat.svg')
        return path

    def get_icon_path(self, icon_name):
        valid_names = [
                'add',
                'arrow_down_small',
                'arrow_up_small',
                'arrow_down_tiny',
                'arrow_left_tiny',
                'arrow_right_tiny',
                'arrow_up_tiny',
                'check_mark',
                'col_expand',
                'col_reset_width',
                'col_shrink',
                'convert_trigger',
                'copy',
                'cut',
                'delete',
                'delete_small',
                'edit',
                'error',
                'input_compact',
                'input_full',
                'new_pattern',
                'new_song',
                'paste',
                'play',
                'play_from_cursor',
                'play_pattern',
                'record',
                'redo',
                'remove',
                'remove_pattern',
                'remove_song',
                'replace',
                'rest',
                'reuse_pattern',
                'silence',
                'splitter_vertical',
                'undo',
                'warning',
                'zoom_in',
                'zoom_out',
                'zoom_reset',
            ]
        if not icon_name in valid_names:
            raise ValueError('invalid icon name {}'.format(icon_name))
        icon_filename = '{}.png'.format(icon_name)
        icon_path = os.path.join(self._icons_path, icon_filename)
        return icon_path

    def get_icons_dir(self):
        return self._icons_path

    def get_style_sheet(self, style_name):
        ss_path = os.path.join(self._styles_path, '{}.css'.format(style_name))
        try:
            with open(ss_path, encoding='utf-8') as f:
                ss = f.read(131072) # TODO: Add some basic sanity checks
        except FileNotFoundError:
            return ''
        return ss


