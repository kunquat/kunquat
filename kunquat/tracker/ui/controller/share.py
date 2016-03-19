# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2016
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
from types import NoneType

from kqtifile import KqtiFile


class Share():

    def __init__(self, path):
        self._path = path
        self._instruments_path = os.path.join(self._path, 'instruments')
        self._icons_path = os.path.join(self._path, 'icons')
        self._keymaps_path = os.path.join(self._path, 'keymaps')
        self._notations_path = os.path.join(self._path, 'notations')

        self._notations = {}
        self._read_notations()

    def _read_notations(self):
        notation_paths = glob.glob(os.path.join(self._notations_path, '*.json'))
        for path in notation_paths:
            base_name = os.path.basename(path)
            key = '.'.join(base_name.split('.')[:-1]) # strip the .json suffix
            with open(path) as f:
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
        name = unsafe_data.get(u'name', None)
        if not isinstance(name, unicode):
            return None
        notation[u'name'] = name

        # Octave names
        unsafe_octave_names = unsafe_data.get(u'octave_names', None)
        if not isinstance(unsafe_octave_names, list) or (len(unsafe_octave_names) == 0):
            return None
        octave_names = []
        for unsafe_name in unsafe_octave_names:
            if not isinstance(unsafe_name, unicode):
                return None
            octave_names.append(unsafe_name)
        notation[u'octave_names'] = octave_names

        # Base octave
        base_octave = unsafe_data.get(u'base_octave', None)
        if not isinstance(base_octave, int):
            return None
        if not 0 <= base_octave < len(octave_names):
            return None
        notation[u'base_octave'] = base_octave

        # Note names
        unsafe_note_names = unsafe_data.get(u'note_names', None)
        if not isinstance(unsafe_note_names, list) or (len(unsafe_note_names) == 0):
            return None
        note_names = []
        for unsafe_desc in unsafe_note_names:
            if (not isinstance(unsafe_desc, list)) or (len(unsafe_desc) != 2):
                return None
            unsafe_cents, unsafe_name = unsafe_desc
            if not isinstance(unsafe_cents, (int, float)):
                return None
            if not isinstance(unsafe_name, unicode):
                return None
            desc = [unsafe_cents, unsafe_name]
            note_names.append(desc)
        notation[u'note_names'] = note_names

        # Keymap
        unsafe_keymap = unsafe_data.get(u'keymap', None)
        if not isinstance(unsafe_keymap, list):
            return None
        keymap = []
        for unsafe_octave in unsafe_keymap:
            if not isinstance(unsafe_octave, list):
                return None
            octave = []
            for unsafe_entry in unsafe_octave:
                if not isinstance(unsafe_entry, (int, float, NoneType)):
                    return None
                octave.append(unsafe_entry)
            keymap.append(octave)
        notation[u'keymap'] = keymap

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
                'arrow_down_small',
                'arrow_up_small',
                'col_expand',
                'col_reset_width',
                'col_shrink',
                'copy',
                'cut',
                'delete',
                'delete_small',
                'edit',
                'error',
                'new_pattern',
                'new_song',
                'paste',
                'play',
                'play_from_cursor',
                'play_pattern',
                'record',
                'redo',
                'remove_pattern',
                'remove_song',
                'replace',
                'rest',
                'reuse_pattern',
                'silence',
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


