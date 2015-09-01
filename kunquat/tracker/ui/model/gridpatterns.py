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
from itertools import izip

import tstamp


STYLE_COUNT = 5

PLACEHOLDER_GRID_PATTERN = {
    'name'  : u'1/4',
    'length': [1, 0],
    'min_style_spacing': [1] * STYLE_COUNT,
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
            if length_ts < 1:
                return False
        except (ValueError, TypeError):
            return False

        if (('min_style_spacing' not in gp) or
                (not isinstance(gp['min_style_spacing'], list))):
            return False
        spacings = gp['min_style_spacing']
        if len(spacings) < STYLE_COUNT:
            return False
        if not all((isinstance(sp, (float, int)) and sp > 0) for sp in spacings):
            return False

        if ('lines' not in gp) or (not isinstance(gp['lines'], list)):
            return False
        if not all(self._is_valid_grid_line(line) for line in gp['lines']):
            return False
        for prev_line, line in izip(gp['lines'], gp['lines'][1:]):
            prev_ts, _ = prev_line
            cur_ts, _ = line
            if prev_ts >= cur_ts:
                return False
        if sum(1 for line in gp['lines'] if line[0] < gp['length'] and line[1] == 0) != 1:
            return False

        return True

    def _get_key(self):
        key = 'i_grid_patterns.json'
        return key

    def _get_raw_grid_dict(self):
        key = self._get_key()
        data = self._store.get(key, {})
        return data if isinstance(data, dict) else {}

    def _get_grid_pattern(self, gp_id):
        raw_dict = self._get_raw_grid_dict()
        gp = raw_dict.get(gp_id, None)
        return gp if self._is_valid_grid_pattern(gp) else PLACEHOLDER_GRID_PATTERN

    def get_grid_pattern_ids(self):
        raw_dict = self._get_raw_grid_dict()
        valid_keys = [k for k in raw_dict if isinstance(k, int) and (k >= 0)]
        return valid_keys

    def get_grid_pattern_name(self, gp_id):
        gp = self._get_grid_pattern(gp_id)
        return gp['name']

    def get_grid_pattern_length(self, gp_id):
        gp = self._get_grid_pattern(gp_id)
        return tstamp.Tstamp(gp['length'])

    def get_grid_pattern_line_style_spacing(self, gp_id, line_style):
        gp = self._get_grid_pattern(gp_id)
        return gp['min_style_spacing'][line_style]

    def get_grid_pattern_lines(self, gp_id):
        gp = self._get_grid_pattern(gp_id)

        lines = []
        for line_raw in gp['lines']:
            ts_raw, style = line_raw
            ts = tstamp.Tstamp(ts_raw)
            if ts >= gp['length']:
                break
            lines.append((ts, style))

        return lines

    def select_grid_pattern(self, gp_id):
        if gp_id != self.get_selected_grid_pattern_id():
            self.select_grid_pattern_line(None)
        self._session.select_grid_pattern(gp_id)

    def get_selected_grid_pattern_id(self):
        return self._session.get_selected_grid_pattern_id()

    def select_grid_pattern_line(self, line_ts):
        self._session.select_grid_pattern_line(line_ts)

    def get_selected_grid_pattern_line(self):
        return self._session.get_selected_grid_pattern_line()

    def _set_raw_grid_dict(self, raw_dict):
        key = self._get_key()
        self._store[key] = raw_dict

    def set_grid_pattern_length(self, gp_id, length):
        raw_dict = self._get_raw_grid_dict()
        raw_dict[gp_id]['length'] = list(length)
        self._set_raw_grid_dict(raw_dict)

    def set_grid_pattern_line_style_spacing(self, gp_id, line_style, spacing):
        raw_dict = self._get_raw_grid_dict()
        raw_dict[gp_id]['min_style_spacing'][line_style] = spacing
        self._set_raw_grid_dict(raw_dict)

    def subdivide_grid_pattern_interval(self, gp_id, line_ts, part_count, line_style):
        raw_dict = self._get_raw_grid_dict()
        gp_dict = raw_dict[gp_id]
        lines = [line for line in gp_dict['lines'] if line[0] < gp_dict['length']]
        gp_length = tstamp.Tstamp(gp_dict['length'])

        # Get the length of the interval
        all_cur_line_tss = (tstamp.Tstamp(line[0]) for line in lines)
        cur_line_tss = [ts for ts in all_cur_line_tss if ts < gp_length]
        following_line_tss = filter(lambda ts: line_ts < ts < gp_length, cur_line_tss)
        if following_line_tss:
            next_ts = following_line_tss[0]
        else:
            first_ts = cur_line_tss[0]
            assert first_ts <= line_ts
            next_ts = first_ts + gp_length
        interval_length = next_ts - line_ts
        assert interval_length > 0

        # Get the timestamps of new lines
        new_line_tss = []
        for i in xrange(1, part_count):
            rel_ts = (i * interval_length) / part_count
            new_ts = (rel_ts + line_ts) % gp_length
            new_line_tss.append(new_ts)

        # Merge new lines with the existing ones
        added_lines = [[list(ts), line_style] for ts in new_line_tss]
        new_lines = sorted(lines + added_lines)

        gp_dict['lines'] = new_lines
        self._set_raw_grid_dict(raw_dict)

    def change_grid_pattern_line_style(self, gp_id, line_ts, new_style):
        raw_dict = self._get_raw_grid_dict()
        gp_dict = raw_dict[gp_id]
        lines = gp_dict['lines']

        new_lines = []
        for line in lines:
            cur_line_ts, cur_line_style = line
            if cur_line_ts != line_ts:
                if cur_line_ts < gp_dict['length']:
                    new_lines.append(line)
            else:
                new_lines.append([cur_line_ts, new_style])

        gp_dict['lines'] = new_lines
        self._set_raw_grid_dict(raw_dict)

    def remove_grid_pattern_line(self, gp_id, line_ts):
        raw_dict = self._get_raw_grid_dict()
        gp_dict = raw_dict[gp_id]
        lines = gp_dict['lines']

        new_lines = []
        for line in lines:
            cur_line_ts, _ = line
            if (cur_line_ts != line_ts) and (cur_line_ts < gp_dict['length']):
                new_lines.append(line)

        gp_dict['lines'] = new_lines
        self._set_raw_grid_dict(raw_dict)


