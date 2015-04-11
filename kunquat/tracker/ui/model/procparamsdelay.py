# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math

from procparams import ProcParams


class ProcParamsDelay(ProcParams):

    _TAPS_MAX = 32

    def __init__(self, proc_id, controller):
        ProcParams.__init__(self, proc_id, controller)

    def get_max_tap_count(self):
        return self._TAPS_MAX

    def get_max_delay(self):
        return self._get_value('p_f_max_delay.json', 2.0)

    def set_max_delay(self, value):
        self._set_value('p_f_max_delay.json', value)

    def _get_tap_subkey(self, tap_index, subkey):
        return 'tap_{:02x}/{}'.format(tap_index, subkey)

    def _get_tap_value(self, tap_index, subkey, default_value):
        tap_subkey = self._get_tap_subkey(tap_index, subkey)
        return self._get_value(tap_subkey, default_value)

    def _set_tap_value(self, tap_index, subkey, value):
        tap_subkey = self._get_tap_subkey(tap_index, subkey)
        self._set_value(tap_subkey, value)

    def _get_tap_existence(self, tap_index):
        has_delay = (self._get_tap_delay(tap_index) >= 0)
        has_volume = not math.isinf(self._get_tap_volume(tap_index))
        return (has_delay and has_volume)

    def _get_tap_delay(self, tap_index):
        default_delay = -1.0
        return self._get_tap_value(tap_index, 'p_f_delay.json', default_delay)

    def _set_tap_delay(self, tap_index, delay):
        self._set_tap_value(tap_index, 'p_f_delay.json', delay)

    def _get_tap_volume(self, tap_index):
        default_volume = 0.0
        return self._get_tap_value(tap_index, 'p_f_volume.json', default_volume)

    def _set_tap_volume(self, tap_index, volume):
        self._set_tap_value(tap_index, 'p_f_volume.json', volume)

    def _get_taps_raw(self):
        taps_raw = []
        for i in xrange(self._TAPS_MAX):
            if self._get_tap_existence(i):
                delay = self._get_tap_delay(i)
                volume = self._get_tap_volume(i)
                taps_raw.append([delay, volume])
            else:
                taps_raw.append(None)
        return taps_raw

    def _get_taps_and_packing_info(self):
        taps = self._get_taps_raw()
        has_holes = (None in taps) and (
                taps.index(None) < sum(1 for t in taps if t != None))
        taps = filter(lambda x: x != None, taps)
        return taps, has_holes

    def _get_taps(self):
        taps, _ = self._get_taps_and_packing_info()
        return taps

    def get_tap_count(self):
        return len(self._get_taps())

    def _set_all_taps(self, taps):
        # TODO: do this in one transaction
        for i, tap in enumerate(taps):
            delay, volume = tap
            self._set_tap_delay(i, delay)
            self._set_tap_volume(i, volume)
        for i in xrange(len(taps), self._TAPS_MAX):
            self._remove_tap(i)

    def add_tap(self):
        taps, has_holes = self._get_taps_and_packing_info()
        if has_holes:
            taps.append([0, 0])
            self._set_all_taps(taps)
        else:
            new_index = len(taps)
            self._set_tap_delay(new_index, 0)
            self._set_tap_volume(new_index, 0)

    def get_tap_delay(self, tap_index):
        return self._get_taps()[tap_index][0]

    def set_tap_delay(self, tap_index, delay):
        taps, has_holes = self._get_taps_and_packing_info()
        if has_holes:
            taps[tap_index][0] = delay
            self._set_all_taps(taps)
        else:
            self._set_tap_delay(tap_index, delay)

    def get_tap_volume(self, tap_index):
        return self._get_taps()[tap_index][1]

    def set_tap_volume(self, tap_index, volume):
        taps, has_holes = self._get_taps_and_packing_info()
        if has_holes:
            taps[tap_index][1] = volume
            self._set_all_taps(taps)
        else:
            self._set_tap_volume(tap_index, volume)

    def _remove_tap(self, tap_index):
        self._set_tap_delay(tap_index, None)
        self._set_tap_volume(tap_index, None)

    def remove_tap(self, tap_index):
        taps, has_holes = self._get_taps_and_packing_info()
        if has_holes:
            del taps[tap_index]
            self._set_all_taps(taps)
        else:
            self._remove_tap(tap_index)


