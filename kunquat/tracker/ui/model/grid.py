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

import tstamp


DEFAULT_GRID = {
    'length': [4, 0],
    'lines': [
        [[0, 0], 0],
        [[0, 220540320], 2],
        [[0, 441080640], 2],
        [[0, 661620960], 2],
        [[1, 0], 1],
        [[1, 220540320], 2],
        [[1, 441080640], 2],
        [[1, 661620960], 2],
        [[2, 0], 1],
        [[2, 220540320], 2],
        [[2, 441080640], 2],
        [[2, 661620960], 2],
        [[3, 0], 1],
        [[3, 220540320], 2],
        [[3, 441080640], 2],
        [[3, 661620960], 2],
    ]
}


class Grid():

    def __init__(self):
        self._controller = None
        self._store = None

    def set_controller(self, controller):
        self._controller = controller
        self._store = controller.get_store()

    def _get_grid_spec(self):
        spec = { 'length': tstamp.Tstamp(DEFAULT_GRID['length']) }

        line_list = []
        for line in DEFAULT_GRID['lines']:
            line_ts_raw, line_style = line
            line_ts = tstamp.Tstamp(line_ts_raw)
            if line_ts < spec['length']:
                line_list.append((line_ts, line_style))

        line_list.sort()
        spec['lines'] = line_list

        return spec

    def _get_next_or_current_line_info(self, pat_num, col_num, row_ts):
        grid_spec = self._get_grid_spec()

        grid_length = grid_spec['length']
        grid_lines = grid_spec['lines']
        if grid_length == 0 or not grid_lines:
            return None

        gp_row_ts = row_ts % grid_length

        line_index = 0
        line_pat_ts = (row_ts +
                (grid_length + grid_lines[0][0] - gp_row_ts) % grid_length)
        for i, line in enumerate(grid_lines):
            line_ts, _ = line
            if line_ts >= gp_row_ts:
                line_index = i
                line_pat_ts = row_ts + line_ts - gp_row_ts
                break

        return line_index, line_pat_ts

    def get_next_or_current_line(self, pat_num, col_num, row_ts):
        line_info = self._get_next_or_current_line_info(pat_num, col_num, row_ts)
        if not line_info:
            return None

        grid_spec = self._get_grid_spec()

        line_index, line_pat_ts = line_info
        _, line_style = grid_spec['lines'][line_index]

        return line_pat_ts, line_style

    def get_next_line(self, pat_num, col_num, row_ts):
        next_ts = row_ts + tstamp.Tstamp(0, 1)
        return self.get_next_or_current_line(pat_num, col_num, next_ts)

    def get_prev_or_current_line(self, pat_num, col_num, row_ts):
        next_ts = row_ts - tstamp.Tstamp(0, 1)
        return self.get_prev_line(pat_num, col_num, next_ts)

    def get_prev_line(self, pat_num, col_num, row_ts):
        line_info = self._get_next_or_current_line_info(pat_num, col_num, row_ts)
        if not line_info:
            return None

        grid_spec = self._get_grid_spec()
        grid_length = grid_spec['length']
        grid_lines = grid_spec['lines']

        next_line_index, next_line_pat_ts = line_info
        line_index = (len(grid_lines) + next_line_index - 1) % len(grid_lines)

        line_ts, line_style = grid_lines[line_index]

        next_line_ts, _ = grid_lines[next_line_index]
        ts_to_next_line = next_line_ts - line_ts
        if next_line_index < line_index:
            ts_to_next_line += grid_length

        line_pat_ts = next_line_pat_ts - ts_to_next_line

        return line_pat_ts, line_style

    def get_grid_lines(self, pat_num, col_num, start_ts, stop_ts):
        lines = []
        grid_spec = self._get_grid_spec()

        grid_length = grid_spec['length']
        grid_lines = grid_spec['lines']
        if grid_length > 0 and grid_lines:
            # Find our first line in the grid pattern
            line_index, line_pat_ts = self._get_next_or_current_line_info(
                    pat_num, col_num, start_ts)

            # Add lines from the grid pattern until stop_ts is reached
            while line_pat_ts < stop_ts:
                _, line_style = grid_lines[line_index]
                lines.append((line_pat_ts, line_style))

                next_line_index = (line_index + 1) % len(grid_lines)

                cur_line_ts, _ = grid_lines[line_index]
                next_line_ts, _ = grid_lines[next_line_index]
                ts_to_next_line = next_line_ts - cur_line_ts
                if next_line_index < line_index:
                    ts_to_next_line += grid_length

                line_index = next_line_index
                line_pat_ts += ts_to_next_line

        return lines


