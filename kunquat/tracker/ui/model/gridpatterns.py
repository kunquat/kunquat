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

from copy import deepcopy

import tstamp


STYLE_COUNT = 4

PLACEHOLDER_GRID_PATTERN = {
    'name'  : u'1/4',
    'length': [1, 0],
    'lines' : [ [[0, 0], 0] ],
}


class GridPatterns():

    def __init__(self):
        self._controller = None
        self._session = None
        self._store = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._store = controller.get_store()

    def _is_valid_grid_line(self, line):
        if (not isinstance(line, list)) or (len(line) < 2):
            return False

        ts_raw, style = line

        try:
            ts = tstamp.Tstamp(ts_raw)
            if ts < 0:
                return False
        except (ValueError, TypeError):
            return False

        if not isinstance(style, int) or not (0 <= style < STYLE_COUNT):
            return False

        return True

    def _is_valid_grid_pattern(self, gp):
        if not isinstance(gp, dict):
            return False

        if ('name' not in gp) or (not isinstance(gp['name'], unicode)):
            return False

        if 'length' not in gp:
            return False
        try:
            length_ts = tstamp.Tstamp(gp['length'])
            if length_ts <= 0:
                return False
        except (ValueError, TypeError):
            return False

        if ('lines' not in gp) or (not isinstance(gp['lines'], list)):
            return False
        if not all(self._is_valid_grid_line(line) for line in gp['lines']):
            return False

        return True

    def _get_raw_grid_dict(self):
        key = 'i_grid_patterns.json'
        data = self._store.get(key, {})
        return data if isinstance(data, dict) else {}

    def get_grid_pattern_ids(self):
        raw_dict = self._get_raw_grid_dict()
        valid_keys = [k for k in raw_dict if isinstance(k, int) and (k >= 0)]
        return valid_keys

    def get_grid_pattern(self, gp_id):
        raw_dict = self._get_raw_grid_dict()
        gp = raw_dict.get(gp_id, None)
        return gp if self._is_valid_grid_pattern(gp) else PLACEHOLDER_GRID_PATTERN

    def select_grid_pattern(self, gp_id):
        self._session.select_grid_pattern(gp_id)

    def get_selected_grid_pattern_id(self):
        return self._session.get_selected_grid_pattern_id()


