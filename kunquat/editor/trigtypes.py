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


"""
def is_cond_char(c):
    return (c in string.ascii_lowercase or
            c in string.digits or
            c in '_(). ' '!=<>+-*/%^|&\'"')
"""


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
#cond_str = (str, lambda x: all(is_cond_char(c) for c in str(x)), '')


triggers = {
        'mpd': [nonneg_ts],
        'm.jc': [(int, lambda x: x >= 0 and x < 65536, '0')],
        'm.jr': [nonneg_ts],
        'm.js': [(int, lambda x: x >= -1 and x < lim.SECTIONS_MAX, '-1')],
        'm.jss': [(int, lambda x: x >= -1 and x < lim.SUBSONGS_MAX, '-1')],
        'mj': [],

        'm.s': [(int, lambda x: x >= 0 and x < lim.SCALES_MAX, '0')],
        'm.so': [finite_float],
        'mms': [(int, lambda x: x >= 0 and x < lim.SCALES_MAX, '0')],
        'm.sfp': [note_entry],
        'mssi': [note_entry],

        'm.t': [(float, lambda x: x >= 1.0 and x <= 999.0, '120')],
        'm/t': [(float, lambda x: x >= 1.0 and x <= 999.0, '120')],
        'm/=t': [nonneg_ts],
        'm.v': [volume],
        'm/v': [volume],
        'm/=v': [nonneg_ts],

        '.i': [(int, lambda x: x >= 0 and x < lim.INSTRUMENTS_MAX, '0')],
        '.g': [(int, lambda x: x >= 0 and x < lim.GENERATORS_MAX, '0')],
        '.e': [(int, lambda x: x >= 0 and x < lim.EFFECTS_MAX, '0')],
        '.ge': [],
        '.ie': [],
        '.d': [(int, lambda x: x >= 0 and x < lim.DSPS_MAX, '0')],

        'n+': [pitch],
        'h': [(HitIndex, lambda x: 0 <= x < lim.HITS_MAX, '0')],
        'n-': [],

        '.f': [force],
        '/f': [force],
        '/=f': [nonneg_ts],
        'ts': [nonneg_float],
        #'tsd': [nonneg_ts],
        'td': [(float, lambda x: x >= 0.0 and x <= 24.0, '0')],
        'tdd': [nonneg_ts],

        '/p': [pitch],
        '/=p': [nonneg_ts],
        'vs': [nonneg_float],
        #'vsd': [nonneg_ts],
        'vd': [nonneg_float],
        'vdd': [nonneg_ts],

        '<arp': [],
        '.arpn': [pitch],
        '.arpi': [(int, lambda x: 0 <= x < lim.ARPEGGIO_NOTES_MAX, '1')],
        '.arps': [(float, lambda x: x > 0 and isfinite(x), '24')],
        'arp+': [],
        'arp-': [],

        '.l': [(float, lambda x: x >= 0 and x <= 99, '0')],
        '/l': [(float, lambda x: x >= 0 and x <= 99, '0')],
        '/=l': [nonneg_ts],
        'ws': [nonneg_float],
        #'wsd': [nonneg_ts],
        'wd': [nonneg_float],
        'wdd': [nonneg_ts],

        '.r': [(float, lambda x: x >= 0 and x <= 99, '0')],
        #'/r': [(float, lambda x: x >= 0 and x <= 99, '0')],
        #'/=r': [nonneg_ts],

        '.P': [(float, lambda x: x >= -1 and x <= 1, '0')],
        '/P': [(float, lambda x: x >= -1 and x <= 1, '0')],
        '/=P': [nonneg_ts],

        '.gBn': [key],
        '.gB': [any_bool],
        '.gIn': [key],
        '.gI': [any_int],
        '.gFn': [key],
        '.gF': [any_float],
        '.gTn': [key],
        '.gT': [any_ts],

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

        '?': [any_bool],
        '?if': [],
        '?else': [],
        '?endif': [],

        'signal': [any_str],
        'callBn': [any_str],
        'callB': [any_bool],
        'callIn': [any_str],
        'callI': [any_int],
        'callFn': [any_str],
        'callF': [any_float],

        'qlocation': [],
        'qvoices': [],
        'qf': [(int, lambda x: x >= 0 and x < lim.GENERATORS_MAX, '0')],

        'Ipause': [],
        'Iresume': [],
        'Ipattern': [(int, lambda x: 0 <= x < lim.PATTERNS_MAX, '0')],

        'I.Bn': [key],
        'I.B': [any_bool],
        'I.In': [key],
        'I.I': [any_int],
        'I.Fn': [key],
        'I.F': [any_float],
        'I.Tn': [key],
        'I.T': [any_ts],

        'I.gr': [nonneg_ts],
        'I.gs': [(int, lambda x: -1 <= x < lim.SECTIONS_MAX, '-1')],
        'I.gss': [(int, lambda x: -1 <= x < lim.SUBSONGS_MAX, '-1')],
        'Ig': [],

        'I.infinite': [any_bool],
}


