# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2011-2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math
import string

import kqt_limits as lim
import timestamp as ts


class Note(float):
    pass


class HitIndex(int):
    pass


def isfinite(x):
    return not (math.isinf(x) or math.isnan(x))


"""
def is_global(ttype):
    ttype = str(ttype)
    return ttype in global_triggers or ttype in general_triggers


def is_channel(ttype):
    ttype = str(ttype)
    return ttype in channel_triggers or ttype in general_triggers
"""


def is_key(value):
    if value is None:
        return False
    return all(x in '_./' or x.isalpha() for x in str(value))


def is_cond_char(c):
    return (c in string.ascii_lowercase or
            c in string.digits or
            c in '_(). ' '!=<>+-*/%^|&\'"')


nonneg_ts = (ts.Timestamp, lambda x: x >= 0, '0')
any_ts = (ts.Timestamp, lambda x: True, '0')
finite_float = (float, isfinite, '0')
nonneg_float = (float, lambda x: x >= 0 and isfinite(x), '0')
pos_float = (float, lambda x: x > 0 and isfinite(x), '1')
force = (float, lambda x: x <= 18 and not math.isnan(x), '0')
volume = (float, lambda x: x <= 0 and not math.isnan(x), '0')
any_float = (float, lambda x: True, '0')
any_bool = (bool, lambda x: True, 'false')
any_int = (int, lambda x: True, '0')
key = (str, is_key, '')
pitch = (Note, isfinite, '0')
note_entry = (int, lambda x: x >= 0, '0') # FIXME
any_str = (str, lambda x: x != None, '')
cond_str = (str, lambda x: all(is_cond_char(c) for c in str(x)), '')


triggers = {
        'wpd': [nonneg_ts],
        'w.jc': [(int, lambda x: x >= 0 and x < 65536, '0')],
        'w.jr': [nonneg_ts],
        'w.js': [(int, lambda x: x >= -1 and x < lim.SECTIONS_MAX, '-1')],
        'w.jss': [(int, lambda x: x >= -1 and x < lim.SUBSONGS_MAX, '-1')],
        'wj': [],

        'w.s': [(int, lambda x: x >= 0 and x < lim.SCALES_MAX, '0')],
        'w.so': [finite_float],
        'wms': [(int, lambda x: x >= 0 and x < lim.SCALES_MAX, '0')],
        'w.sfp': [note_entry],
        'wssi': [note_entry],

        'w.t': [(float, lambda x: x >= 1.0 and x <= 999.0, '120')],
        'w/t': [(float, lambda x: x >= 1.0 and x <= 999.0, '120')],
        'w/=t': [nonneg_ts],
        'w.v': [volume],
        'w/v': [volume],
        'w/=v': [nonneg_ts],

        'c.i': [(int, lambda x: x >= 0 and x < lim.INSTRUMENTS_MAX, '0')],
        'c.g': [(int, lambda x: x >= 0 and x < lim.GENERATORS_MAX, '0')],
        'c.e': [(int, lambda x: x >= 0 and x < lim.EFFECTS_MAX, '0')],
        'c.ge': [],
        'c.ie': [],
        'c.d': [(int, lambda x: x >= 0 and x < lim.DSPS_MAX, '0')],

        'cn+': [pitch],
        'ch': [(HitIndex, lambda x: 0 <= x < lim.HITS_MAX, '0')],
        'cn-': [],

        'c.f': [force],
        'c/f': [force],
        'c/=f': [nonneg_ts],
        'cTs': [nonneg_float],
        #'cTsd': [nonneg_ts],
        'cTd': [(float, lambda x: x >= 0.0 and x <= 24.0, '0')],
        'cTdd': [nonneg_ts],

        'c/p': [pitch],
        'c/=p': [nonneg_ts],
        'cVs': [nonneg_float],
        #'cVsd': [nonneg_ts],
        'cVd': [nonneg_float],
        'cVdd': [nonneg_ts],

        'c<Arp': [],
        'c.Arpn': [pitch],
        'c.Arpi': [(int, lambda x: 0 <= x < lim.ARPEGGIO_NOTES_MAX, '1')],
        'c.Arps': [(float, lambda x: x > 0 and isfinite(x), '24')],
        'cArp+': [],
        'cArp-': [],

        'c.l': [(float, lambda x: x >= 0 and x <= 99, '0')],
        'c/l': [(float, lambda x: x >= 0 and x <= 99, '0')],
        'c/=l': [nonneg_ts],
        'cAs': [nonneg_float],
        #'cAsd': [nonneg_ts],
        'cAd': [nonneg_float],
        'cAdd': [nonneg_ts],

        'c.r': [(float, lambda x: x >= 0 and x <= 99, '0')],
        #'c/r': [(float, lambda x: x >= 0 and x <= 99, '0')],
        #'c/=r': [nonneg_ts],

        'c.P': [(float, lambda x: x >= -1 and x <= 1, '0')],
        'c/P': [(float, lambda x: x >= -1 and x <= 1, '0')],
        'c/=P': [nonneg_ts],

        'c.gBn': [key],
        'c.gB': [any_bool],
        'c.gIn': [key],
        'c.gI': [any_int],
        'c.gFn': [key],
        'c.gF': [any_float],
        'c.gTn': [key],
        'c.gT': [any_ts],

        'i.sus': [(float, lambda x: x >= 0 and x <= 1, '0')],

        'g.Bn': [key],
        'g.B': [any_bool],
        'g.In': [key],
        'g.I': [any_int],
        'g.Fn': [key],
        'g.F': [any_float],
        'g.Tn': [key],
        'g.T': [any_ts],

        'ebp+': [],
        'ebp-': [],

        'd.Bn': [key],
        'd.B': [any_bool],
        'd.In': [key],
        'd.I': [any_int],
        'd.Fn': [key],
        'd.F': [any_float],
        'd.Tn': [key],
        'd.T': [any_ts],

        '#': [any_str],

        '#?': [cond_str],
        '#if': [any_bool],
        '#endif': [],

        '#signal': [any_str],
        '#callBn': [any_str],
        '#callB': [any_bool],
        '#callIn': [any_str],
        '#callI': [any_int],
        '#callFn': [any_str],
        '#callF': [any_float],

        '>pause': [],
        '>resume': [],
        '>pattern': [(int, lambda x: 0 <= x < lim.PATTERNS_MAX, '0')],

        '>.Bn': [key],
        '>.B': [any_bool],
        '>.In': [key],
        '>.I': [any_int],
        '>.Fn': [key],
        '>.F': [any_float],
        '>.Tn': [key],
        '>.T': [any_ts],

        '>.gr': [nonneg_ts],
        '>.gs': [(int, lambda x: -1 <= x < lim.SECTIONS_MAX, '-1')],
        '>.gss': [(int, lambda x: -1 <= x < lim.SUBSONGS_MAX, '-1')],
        '>g': [],

        '>infinite': [any_bool],
}


