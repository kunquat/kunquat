# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

class Share():

    def __init__(self, path):
        self._path = path

        # TODO: read data from the share directory

        self._keymaps = {
            '12tet': {
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
            'slendro': {
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

    def get_keymap_names(self):
        return self._keymaps.keys()

    def get_keymap_data(self, name):
        return self._keymaps[name]


