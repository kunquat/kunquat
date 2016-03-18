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

from kqtifile import KqtiFile


class Share():

    def __init__(self, path):
        self._path = path
        self._instruments_path = os.path.join(self._path, 'instruments')
        self._icons_path = os.path.join(self._path, 'icons')
        self._keymaps_path = os.path.join(self._path, 'keymaps')
        self._notations_path = os.path.join(self._path, 'notations')

        self._notations = {}

        # TODO: read data from the share directory

        self._keymaps = {
            u'12tet': {
                'name': '12-tone Equal Temperament',
                'base_octave': 5,
                'keymap': [
                    [-5700, -5600, -5500, -5400, -5300, None, -5200, -5100, -5000, -4900, -4800, -4700, -4600, None],
                    [-4500, -4400, -4300, -4200, -4100, None, -4000, -3900, -3800, -3700, -3600, -3500, -3400, None],
                    [-3300, -3200, -3100, -3000, -2900, None, -2800, -2700, -2600, -2500, -2400, -2300, -2200, None],
                    [-2100, -2000, -1900, -1800, -1700, None, -1600, -1500, -1400, -1300, -1200, -1100, -1000, None],
                    [-900, -800, -700, -600, -500, None, -400, -300, -200, -100, 0, 100, 200, None],
                    [300, 400, 500, 600, 700, None, 800, 900, 1000, 1100, 1200, 1300, 1400, None],
                    [1500, 1600, 1700, 1800, 1900, None, 2000, 2100, 2200, 2300, 2400, 2500, 2600, None],
                    [2700, 2800, 2900, 3000, 3100, None, 3200, 3300, 3400, 3500, 3600, 3700, 3800, None],
                    [3900, 4000, 4100, 4200, 4300, None, 4400, 4500, 4600, 4700, 4800, 4900, 5000, None],
                    [5100, 5200, 5300, 5400, 5500, None, 5600, 5700, 5800, 5900, 6000, 6100, 6200, None],
                    [6300, 6400, 6500, 6600, 6700, None, 6800, 6900, 7000, 7100, 7200, 7300, 7400, None]
                ]
            },
            u'slendro': {
                "name": "Slendro from 440 Hz",
                "base_octave": 5,
                "keymap": [
                    [-6000, -5755, -5493, -5265, -5025, None],
                    [-4800, -4555, -4293, -4065, -3825, None],
                    [-3600, -3355, -3093, -2865, -2625, None],
                    [-2400, -2155, -1893, -1665, -1425, None],
                    [-1200, -955, -693, -465, -225, None],
                    [0, 245, 507, 735, 975, None],
                    [1200, 1445, 1707, 1935, 2175, None],
                    [2400, 2645, 2907, 3135, 3375, None],
                    [3600, 3845, 4107, 4335, 4575, None],
                    [4800, 5045, 5307, 5535, 5775, None],
                    [6000, 6245, 6507, 6735, 6975, None]
                ]
            }
        }

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


