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

from gridpattern import STYLE_COUNT


class Grid():

    def __init__(self):
        self._controller = None
        self._store = None
        self._ui_model = None

    def set_controller(self, controller):
        self._controller = controller
        self._store = controller.get_store()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def _get_allowed_line_styles(self, spec, tr_height_ts):
        allowed_styles = set(xrange(STYLE_COUNT))

        for style in xrange(1, STYLE_COUNT):
            min_line_dist = spec['min_style_spacing'][style] * tr_height_ts
            first_ts = None
            prev_ts = None

            for line in spec['lines']:
                line_ts, line_style = line

                # Ignore disallowed lines or lines of lower priority
                if (line_style not in allowed_styles) or (line_style > style):
                    continue

                # Disallow style if too close to another checked line
                if (prev_ts != None) and ((line_ts - prev_ts) < min_line_dist):
                    allowed_styles.discard(style)
                    break

                if first_ts == None:
                    first_ts = line_ts
                prev_ts = line_ts
            else:
                if first_ts != None:
                    # Check wrap-around distance
                    next_ts = first_ts + spec['length']
                    if (next_ts - prev_ts) < min_line_dist:
                        allowed_styles.discard(style)

        return allowed_styles

    def _get_grid_spec(self, gp_id, tr_height_ts):
        grid_manager = self._ui_model.get_grid_manager()
        gp = grid_manager.get_grid_pattern(gp_id)

        gp_length = gp.get_length()
        gp_lines = gp.get_lines()
        gp_style_spacing = [gp.get_line_style_spacing(i) for i in xrange(STYLE_COUNT)]

        spec = {
            'length': gp_length,
            'lines': gp_lines,
            'min_style_spacing': gp_style_spacing,
        }

        # Filter out line styles with insufficient separation
        allowed_styles = self._get_allowed_line_styles(spec, tr_height_ts)
        filtered_lines = [line for line in gp_lines if line[1] in allowed_styles]
        spec['lines'] = filtered_lines

        return spec

    def _get_next_or_current_line_info(self, pat_num, col_num, row_ts, tr_height_ts):
        grid_spec = self._get_grid_spec(0, tr_height_ts)

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

    def get_next_or_current_line(self, pat_num, col_num, row_ts, tr_height_ts):
        line_info = self._get_next_or_current_line_info(
                pat_num, col_num, row_ts, tr_height_ts)
        if not line_info:
            return None

        grid_spec = self._get_grid_spec(0, tr_height_ts)

        line_index, line_pat_ts = line_info
        _, line_style = grid_spec['lines'][line_index]

        return line_pat_ts, line_style

    def get_next_line(self, pat_num, col_num, row_ts, tr_height_ts):
        next_ts = row_ts + tstamp.Tstamp(0, 1)
        return self.get_next_or_current_line(pat_num, col_num, next_ts, tr_height_ts)

    def get_prev_or_current_line(self, pat_num, col_num, row_ts, tr_height_ts):
        next_ts = row_ts - tstamp.Tstamp(0, 1)
        return self.get_prev_line(pat_num, col_num, next_ts, tr_height_ts)

    def get_prev_line(self, pat_num, col_num, row_ts, tr_height_ts):
        line_info = self._get_next_or_current_line_info(
                pat_num, col_num, row_ts, tr_height_ts)
        if not line_info:
            return None

        grid_spec = self._get_grid_spec(0, tr_height_ts)
        grid_length = grid_spec['length']
        grid_lines = grid_spec['lines']

        next_line_index, next_line_pat_ts = line_info
        line_index = (len(grid_lines) + next_line_index - 1) % len(grid_lines)

        line_ts, line_style = grid_lines[line_index]

        next_line_ts, _ = grid_lines[next_line_index]
        ts_to_next_line = next_line_ts - line_ts
        if next_line_index <= line_index:
            ts_to_next_line += grid_length

        line_pat_ts = next_line_pat_ts - ts_to_next_line

        return line_pat_ts, line_style

    def get_grid_lines(self, pat_num, col_num, start_ts, stop_ts, tr_height_ts):
        lines = []
        grid_spec = self._get_grid_spec(0, tr_height_ts)

        grid_length = grid_spec['length']
        grid_lines = grid_spec['lines']
        if grid_length > 0 and grid_lines:
            # Find our first line in the grid pattern
            line_index, line_pat_ts = self._get_next_or_current_line_info(
                    pat_num, col_num, start_ts, tr_height_ts)

            # Add lines from the grid pattern until stop_ts is reached
            while line_pat_ts < stop_ts:
                _, line_style = grid_lines[line_index]
                lines.append((line_pat_ts, line_style))

                next_line_index = (line_index + 1) % len(grid_lines)

                cur_line_ts, _ = grid_lines[line_index]
                next_line_ts, _ = grid_lines[next_line_index]
                ts_to_next_line = next_line_ts - cur_line_ts
                if next_line_index <= line_index:
                    ts_to_next_line += grid_length

                line_index = next_line_index
                line_pat_ts += ts_to_next_line

        return lines


