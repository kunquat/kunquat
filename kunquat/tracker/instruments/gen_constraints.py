# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import defaultdict

__all__ = ['constraints']


add = {
        '.*base.*\\.jsonln':
        {
            'length': 4096,
            'range': (-1, 1),
        },
        '.*pan\\.jsonf':
        {
            'decimals': 2,
            'default': 0,
            'range': [-1, 1]
        },
        '.*pitch\\.jsonf':
        {
            'decimals': 3,
            'default': 1,
            'range': [0.001, 64]
        },
        '.*scale_amount.*\\.jsonf':
        {
            'decimals': 2,
            'default': 0,
            'range': [-8, 8]
        },
        '.*scale_center.*\\.jsonf':
        {
            'default': 0,
            'range': [-6000, 6000]
        },
        '.*volume.*\\.jsonf':
        {
            'decimals': 2,
            'default': 0,
            'range': [-96, 8]
        },
        'p_force_mod_env\\.jsone':
        {
            'default':
            {
                'nodes': [[0, 1], [1, 1]]
            },
            'first_locked': [True, False],
            'last_locked': [True, False],
            'x_range': [0, 1],
            'y_range': [0, 1]
        },
        'p_mod\\.jsoni':
        {
            'default': 0,
            'range': [0, 1]
        },
        'p_mod_env\\.jsone':
        {
            'default':
            {
                'nodes': [[0, 1], [1, 0]]
            },
            'first_locked': [True, False],
            'last_locked': [False, True],
            'x_range': [0, 60],
            'y_range': [0, 1]
        }
    }


constraints = defaultdict(lambda: None,
    {
        'add': add,
    })


