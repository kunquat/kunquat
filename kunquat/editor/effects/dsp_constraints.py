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


chorus = {
        'p_delay\\.jsonf':
        {
            'decimals': 3,
            'default': 0,
            'range': (0, 0.124),
        },
        'p_range\\.jsonf':
        {
            'decimals': 3,
            'default': 0,
            'range': (0, 0.124),
        },
        'p_speed\\.jsonf':
        {
            'decimals': 2,
            'default': 0,
            'range': (0, 10),
        },
        'p_volume\\.jsonf':
        {
            'decimals': 2,
            'default': 0,
            'range': (-48, 18),
        },
    }


convolution = {
        'p_max_ir_len.jsonf':
        {
            'decimals': 2,
            'default': 0.25,
            'range': (0, 4),
        },
        'p_volume.jsonf':
        {
            'decimals': 2,
            'default': 0,
            'range': (-48, 18),
        },
    }


delay = {
        'p_max_delay.jsonf':
        {
            'decimals': 2,
            'default': 2,
            'range': (0.25, 32),
        },
        'p_delay.jsonf':
        {
            'decimals': 2,
            'default': 2,
            'range': (0, 32),
        },
        'p_volume.jsonf':
        {
            'decimals': 2,
            'default': 2,
            'range': (-48, 18),
        },
    }


freeverb = {
        'p_refl.jsonf':
        {
            'decimals': 1,
            'default': 20,
            'range': (0, 200),
        },
        'p_damp.jsonf':
        {
            'decimals': 1,
            'default': 20,
            'range': (0, 100),
        },
    }


gaincomp = {
        'p_map.jsone':
        {
            'default':
            {
                'nodes': [[0, 1], [1, 1]]
            },
            'first_locked': [True, False],
            'last_locked': [True, False],
            'x_range': [0, 1],
            'y_range': [0, 1]
        }
    }


volume = {
        'p_volume.jsonf':
        {
            'decimals': 2,
            'default': 0,
            'range': (-48, 18),
        }
    }


constraints = defaultdict(lambda: None,
        {
            'chorus': chorus,
            'convolution': convolution,
            'delay': delay,
            'freeverb': freeverb,
            'gaincomp': gaincomp,
            'volume': volume,
        })


