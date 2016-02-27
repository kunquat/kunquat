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

import os.path

from kqtifile import KqtiFile


class Share():

    def __init__(self, path):
        self._path = path
        self._instruments_path = os.path.join(self._path, 'instruments')
        self._icons_path = os.path.join(self._path, 'icons')

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

        self._notations = {
            u'12tet': {
                "name": "12-tone Equal Temperament",

                "octave_names": ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"],

                "note_names": [
                    [-5700, "C0"], [-5600, "C#0"], [-5500, "D0"], [-5400, "D#0"], [-5300, "E0"], [-5200, "F0"], [-5100, "F#0"], [-5000, "G0"], [-4900, "G#0"], [-4800, "A0"], [-4700, "A#0"], [-4600, "B0"],
                    [-4500, "C1"], [-4400, "C#1"], [-4300, "D1"], [-4200, "D#1"], [-4100, "E1"], [-4000, "F1"], [-3900, "F#1"], [-3800, "G1"], [-3700, "G#1"], [-3600, "A1"], [-3500, "A#1"], [-3400, "B1"],
                    [-3300, "C2"], [-3200, "C#2"], [-3100, "D2"], [-3000, "D#2"], [-2900, "E2"], [-2800, "F2"], [-2700, "F#2"], [-2600, "G2"], [-2500, "G#2"], [-2400, "A2"], [-2300, "A#2"], [-2200, "B2"],
                    [-2100, "C3"], [-2000, "C#3"], [-1900, "D3"], [-1800, "D#3"], [-1700, "E3"], [-1600, "F3"], [-1500, "F#3"], [-1400, "G3"], [-1300, "G#3"], [-1200, "A3"], [-1100, "A#3"], [-1000, "B3"],
                    [-900, "C4"], [-800, "C#4"], [-700, "D4"], [-600, "D#4"], [-500, "E4"], [-400, "F4"], [-300, "F#4"], [-200, "G4"], [-100, "G#4"], [0, "A4"], [100, "A#4"], [200, "B4"],
                    [300, "C5"], [400, "C#5"], [500, "D5"], [600, "D#5"], [700, "E5"], [800, "F5"], [900, "F#5"], [1000, "G5"], [1100, "G#5"], [1200, "A5"], [1300, "A#5"], [1400, "B5"],
                    [1500, "C6"], [1600, "C#6"], [1700, "D6"], [1800, "D#6"], [1900, "E6"], [2000, "F6"], [2100, "F#6"], [2200, "G6"], [2300, "G#6"], [2400, "A6"], [2500, "A#6"], [2600, "B6"],
                    [2700, "C7"], [2800, "C#7"], [2900, "D7"], [3000, "D#7"], [3100, "E7"], [3200, "F7"], [3300, "F#7"], [3400, "G7"], [3500, "G#7"], [3600, "A7"], [3700, "A#7"], [3800, "B7"],
                    [3900, "C8"], [4000, "C#8"], [4100, "D8"], [4200, "D#8"], [4300, "E8"], [4400, "F8"], [4500, "F#8"], [4600, "G8"], [4700, "G#8"], [4800, "A8"], [4900, "A#8"], [5000, "B8"],
                    [5100, "C9"], [5200, "C#9"], [5300, "D9"], [5400, "D#9"], [5500, "E9"], [5600, "F9"], [5700, "F#9"], [5800, "G9"], [5900, "G#9"], [6000, "A9"], [6100, "A#9"], [6200, "B9"],
                    [6300, "C10"], [6400, "C#10"], [6500, "D10"], [6600, "D#10"], [6700, "E10"], [6800, "F10"], [6900, "F#10"], [7000, "G10"], [7100, "G#10"], [7200, "A10"], [7300, "A#10"], [7400, "B10"]
                ]
            },
            u'slendro': {
                "name": "Slendro from 0c",

                "octave_names": ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"],

                "note_names": [
                    [-6000, "ji0"], [-5755, "ro0"], [-5493, "lu0"], [-5265, "ma0"], [-5025, "nam0"],
                    [-4800, "ji1"], [-4555, "ro1"], [-4293, "lu1"], [-4065, "ma1"], [-3825, "nam1"],
                    [-3600, "ji2"], [-3355, "ro2"], [-3093, "lu2"], [-2865, "ma2"], [-2625, "nam2"],
                    [-2400, "ji3"], [-2155, "ro3"], [-1893, "lu3"], [-1665, "ma3"], [-1425, "nam3"],
                    [-1200, "ji4"], [-955, "ro4"], [-693, "lu4"], [-465, "ma4"], [-225, "nam4"],
                    [0, "ji5"], [245, "ro5"], [507, "lu5"], [735, "ma5"], [975, "nam5"],
                    [1200, "ji6"], [1445, "ro6"], [1707, "lu6"], [1935, "ma6"], [2175, "nam6"],
                    [2400, "ji7"], [2645, "ro7"], [2907, "lu7"], [3135, "ma7"], [3375, "nam7"],
                    [3600, "ji8"], [3845, "ro8"], [4107, "lu8"], [4335, "ma8"], [4575, "nam8"],
                    [4800, "ji9"], [5045, "ro9"], [5307, "lu9"], [5535, "ma9"], [5775, "nam9"],
                    [6000, "ji10"], [6245, "ro10"], [6507, "lu10"], [6735, "ma10"], [6975, "nam10"]
                ]
            }
        }

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
                'new_pattern',
                'new_song',
                'paste',
                'play',
                'play_from_cursor',
                'play_pattern',
                'record',
                'remove_pattern',
                'remove_song',
                'replace',
                'rest',
                'reuse_pattern',
                'silence',
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


