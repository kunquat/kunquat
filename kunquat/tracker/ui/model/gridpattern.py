# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from copy import deepcopy
import math

from . import tstamp


STYLE_COUNT = 9

def _get_default_lines_raw():
    lines_raw = []
    for i in range(32):
        ts = tstamp.Tstamp(i) / 8
        ts_raw = list(ts)
        style = 0
        if i > 0:
            style = (3 if (i % 8) == 0 else (7 if (i % 2) == 0 else 8))
        lines_raw.append([ts_raw, style])

    return lines_raw

DEFAULT_GRID_PATTERN = {
    'name'  : u'<default grid>',
    'length': [4, 0],
    'offset': [0, 0],
    'min_style_spacing': [0.6] * STYLE_COUNT,
    'lines' : _get_default_lines_raw(),
}


class GridPattern():

    def __init__(self, gp_id):
        self._controller = None
        self._session = None
        self._store = None

        assert isinstance(gp_id, unicode)
        self._id = gp_id

        self._model_data = None

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

    def _make_model_dict(self, gp):
        if not isinstance(gp, dict):
            return None

        result = {}

        if ('name' not in gp) or (not isinstance(gp['name'], unicode)):
            return None
        result['name'] = gp['name']

        try:
            length_ts = tstamp.Tstamp(gp['length'])
            if length_ts < 1:
                return None
            result['length'] = length_ts
        except (KeyError, ValueError, TypeError):
            return None

        try:
            offset_ts = tstamp.Tstamp(gp['offset'])
            if offset_ts < 0:
                return None
            result['offset'] = offset_ts
        except (KeyError, ValueError, TypeError):
            return None

        if (('min_style_spacing' not in gp) or
                (not isinstance(gp['min_style_spacing'], list))):
            return None
        spacings = gp['min_style_spacing']
        if len(spacings) < STYLE_COUNT:
            return None
        if not all((isinstance(sp, (float, int)) and sp > 0) for sp in spacings):
            return None
        result['min_style_spacing'] = gp['min_style_spacing']

        if ('lines' not in gp) or (not isinstance(gp['lines'], list)):
            return None
        if not all(self._is_valid_grid_line(line) for line in gp['lines']):
            return None
        for prev_line, line in zip(gp['lines'], gp['lines'][1:]):
            prev_ts, _ = prev_line
            cur_ts, _ = line
            if prev_ts >= cur_ts:
                return None
        if sum(1 for line in gp['lines'] if line[0] < gp['length'] and line[1] == 0) != 1:
            return None
        result['lines'] = [(tstamp.Tstamp(ts_raw), style)
                for (ts_raw, style) in gp['lines']]

        return result

    def _get_key(self):
        key = 'i_grid_patterns.json'
        return key

    def _get_raw_master_dict(self):
        key = self._get_key()
        data = self._store.get(key, {})
        return data if isinstance(data, dict) else {}

    def _make_placeholder_grid_pattern(self):
        raw_gp = deepcopy(DEFAULT_GRID_PATTERN)
        raw_gp['name'] = u'<invalid>'
        return self._make_model_dict(raw_gp)

    def _get_model_data(self):
        if self._model_data:
            return self._model_data

        raw_master_dict = self._get_raw_master_dict()
        raw_gp = raw_master_dict.get(self._id, DEFAULT_GRID_PATTERN)
        model_gp = (self._make_model_dict(raw_gp) or
                self._make_placeholder_grid_pattern())
        assert model_gp

        shifted_lines = []
        for line in model_gp['lines']:
            ts, style = line
            if ts >= model_gp['length']:
                break
            final_ts = (ts + model_gp['offset']) % model_gp['length']
            shifted_lines.append((final_ts, style))
        model_gp['shifted_lines'] = sorted(shifted_lines)

        self._model_data = model_gp
        return self._model_data

    def get_name(self):
        gp = self._get_model_data()
        return gp['name']

    def get_length(self):
        gp = self._get_model_data()
        return gp['length']

    def get_offset(self):
        gp = self._get_model_data()
        return gp['offset']

    def get_line_style_spacing(self, line_style):
        gp = self._get_model_data()
        return gp['min_style_spacing'][line_style]

    def get_lines(self):
        gp = self._get_model_data()
        return gp['shifted_lines']

    def select_line(self, line_ts):
        self._session.select_grid_pattern_line(line_ts)

    def _select_line_delta(self, delta):
        assert delta in (-1, 1)

        # Get lines
        lines = self.get_lines()

        cur_selection = self.get_selected_line()

        # Find the list index of the current selection
        for i, line in enumerate(lines):
            line_ts, _ = line
            if line_ts == cur_selection:
                selected_index = i
                break
        else:
            # No line selected, so select the first line
            line_ts, _ = lines[0]
            self.select_line(line_ts)
            return

        selected_index = min(max(0, (selected_index + delta)), len(lines) - 1)
        selected_line = lines[selected_index]
        selected_line_ts, _ = selected_line
        self.select_line(selected_line_ts)

    def select_prev_line(self):
        self._select_line_delta(-1)

    def select_next_line(self):
        self._select_line_delta(1)

    def get_selected_line(self):
        return self._session.get_selected_grid_pattern_line()

    def _set_grid_pattern_data(self, model_data):
        md = model_data
        raw_dict = {
            'name'             : md['name'],
            'length'           : list(md['length']),
            'offset'           : list(md['offset']),
            'min_style_spacing': md['min_style_spacing'],
            'lines'            : [[list(ts), style] for (ts, style) in md['lines']],
        }

        # Clear cache so that our data is validated on the next retrieval
        self._model_data = {}

        master_key = self._get_key()
        raw_master_dict = self._get_raw_master_dict()
        raw_master_dict[self._id] = raw_dict
        self._store[master_key] = raw_master_dict

    def set_name(self, name):
        assert self._id != 0
        assert isinstance(name, unicode)
        gp = self._get_model_data()
        gp['name'] = name
        self._set_grid_pattern_data(gp)

    def set_length(self, length):
        assert self._id != 0
        gp = self._get_model_data()
        gp['length'] = length
        self._set_grid_pattern_data(gp)

    def set_offset(self, offset):
        assert self._id != 0
        gp = self._get_model_data()
        gp['offset'] = offset

        # Filter out lines beyond the grid length to avoid confusion
        gp['lines'] = [(ts, s) for (ts, s) in gp['lines'] if ts < gp['length']]

        self._set_grid_pattern_data(gp)

    def set_line_style_spacing(self, line_style, spacing):
        assert self._id != 0
        gp = self._get_model_data()
        gp['min_style_spacing'][line_style] = spacing
        self._set_grid_pattern_data(gp)

    def _get_warp_func(self, warp_value):
        def warp_func(x):
            return x**(math.log(1.0 / warp_value, 2))
        return warp_func

    def subdivide_interval(
            self, line_ts, part_count, warp_value, line_style):
        assert self._id != 0

        gp = self._get_model_data()
        lines = [line for line in gp['lines'] if line[0] < gp['length']]

        # Remove offset from the timestamp argument
        line_ts = (line_ts + gp['length'] - gp['offset']) % gp['length']

        # Get the length of the interval
        all_cur_line_tss = (ts for (ts, _) in lines)
        cur_line_tss = [ts for ts in all_cur_line_tss if ts < gp['length']]
        following_line_tss = filter(lambda ts: line_ts < ts < gp['length'], cur_line_tss)
        if following_line_tss:
            next_ts = following_line_tss[0]
        else:
            first_ts = cur_line_tss[0]
            assert first_ts <= line_ts
            next_ts = first_ts + gp['length']
        interval_length = next_ts - line_ts
        assert interval_length > 0

        # Get the timestamps of new lines
        warp_func = self._get_warp_func(warp_value)
        new_line_tss = []
        for i in range(1, part_count):
            if warp_value == 0.5:
                rel_ts = tstamp.Tstamp(i * interval_length) / part_count
            else:
                pos_norm = warp_func(float(i) / float(part_count))
                rel_ts = tstamp.Tstamp(pos_norm) * interval_length
            new_ts = (rel_ts + line_ts) % gp['length']
            new_line_tss.append(new_ts)

        # Escape if we got any duplicate timestamps
        total_line_count = len(new_line_tss) + len(cur_line_tss)
        if len(set(new_line_tss) | set(cur_line_tss)) < total_line_count:
            return

        # Merge new lines with the existing ones
        added_lines = [(list(ts), line_style) for ts in new_line_tss]
        new_lines = sorted(lines + added_lines)

        gp['lines'] = new_lines
        self._set_grid_pattern_data(gp)

    def change_line_style(self, line_ts, new_style):
        assert self._id != 0
        gp = self._get_model_data()
        lines = gp['lines']

        # Remove offset from the timestamp argument
        line_ts = (line_ts + gp['length'] - gp['offset']) % gp['length']

        new_lines = []
        for line in lines:
            cur_line_ts, cur_line_style = line
            if cur_line_ts != line_ts:
                if cur_line_ts < gp['length']:
                    new_lines.append(line)
            else:
                new_lines.append((cur_line_ts, new_style))

        gp['lines'] = new_lines
        self._set_grid_pattern_data(gp)

    def remove_line(self, line_ts):
        assert self._id != 0
        gp = self._get_model_data()
        lines = gp['lines']

        # Remove offset from the timestamp argument
        line_ts = (line_ts + gp['length'] - gp['offset']) % gp['length']

        new_lines = []
        for line in lines:
            cur_line_ts, _ = line
            if (cur_line_ts != line_ts) and (cur_line_ts < gp['length']):
                new_lines.append(line)

        gp['lines'] = new_lines
        self._set_grid_pattern_data(gp)


