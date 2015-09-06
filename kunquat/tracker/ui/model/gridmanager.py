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
from gridpattern import GridPattern, STYLE_COUNT


class GridManager():

    def __init__(self):
        self._controller = None
        self._session = None
        self._store = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._store = controller.get_store()

    def _get_key(self):
        key = 'i_grid_patterns.json'
        return key

    def _set_raw_master_dict(self, raw_dict):
        key = self._get_key()
        self._store[key] = raw_dict

    def _get_raw_master_dict(self):
        key = self._get_key()
        data = self._store.get(key, {})
        return data if isinstance(data, dict) else {}

    def get_grid_pattern_ids(self):
        raw_dict = self._get_raw_master_dict()
        valid_keys = [k for k in raw_dict if isinstance(k, int) and (k >= 0)]
        return valid_keys

    def select_grid_pattern(self, gp_id):
        if gp_id != self.get_selected_grid_pattern_id():
            self._session.select_grid_pattern_line(None)
        self._session.select_grid_pattern(gp_id)

    def get_selected_grid_pattern_id(self):
        return self._session.get_selected_grid_pattern_id()

    def get_grid_pattern(self, gp_id):
        gp = GridPattern(gp_id)
        gp.set_controller(self._controller)
        return gp

    def add_grid_pattern(self):
        raw_master_dict = self._get_raw_master_dict()

        # Find a new unique ID
        used_ids = set(raw_master_dict.iterkeys())
        for i in xrange(len(used_ids) + 1):
            new_id = i
            if new_id not in used_ids:
                break
        else:
            assert False

        # Get used names
        used_names = set()
        for raw_grid in raw_master_dict.itervalues():
            if isinstance(raw_grid, dict) and 'name' in raw_grid:
                name = raw_grid['name']
                if isinstance(name, unicode):
                    used_names.add(raw_grid['name'])

        # Find a unique placeholder name for the new grid pattern
        for i in xrange(len(used_names) + 1):
            placeholder = u'New grid{}'.format(' ' + str(i + 1) if i > 0 else '')
            if placeholder not in used_names:
                break
        else:
            assert False

        # Create new grid pattern
        lines = []
        for i in xrange(16):
            line_ts_raw = list(tstamp.Tstamp(i) / 4)
            style = 0 if (i == 0) else (1 if (i % 4) == 0 else 2)
            lines.append([line_ts_raw, style])

        new_raw_dict = {
            'name'             : placeholder,
            'length'           : [4, 0],
            'offset'           : [0, 0],
            'min_style_spacing': [1] * STYLE_COUNT,
            'lines'            : lines,
        }

        raw_master_dict[new_id] = new_raw_dict
        self._set_raw_master_dict(raw_master_dict)

    def remove_grid_pattern(self, gp_id):
        raw_master_dict = self._get_raw_master_dict()
        del raw_master_dict[gp_id]
        self._set_raw_master_dict(raw_master_dict)

    def set_zoom(self, zoom):
        self._session.set_grid_pattern_zoom(zoom)

    def set_zoom_range(self, minimum, maximum):
        self._session.set_grid_pattern_zoom_range(minimum, maximum)

    def get_zoom(self):
        return self._session.get_grid_pattern_zoom()

    def get_zoom_range(self):
        return self._session.get_grid_pattern_zoom_range()


