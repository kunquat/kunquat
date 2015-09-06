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

from gridpattern import GridPattern


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

    def _set_raw_grid_dict(self, raw_dict):
        key = self._get_key()
        self._store[key] = raw_dict

    def _get_raw_grid_dict(self):
        key = self._get_key()
        data = self._store.get(key, {})
        return data if isinstance(data, dict) else {}

    def get_grid_pattern_ids(self):
        raw_dict = self._get_raw_grid_dict()
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

    def set_zoom(self, zoom):
        self._session.set_grid_pattern_zoom(zoom)

    def set_zoom_range(self, minimum, maximum):
        self._session.set_grid_pattern_zoom_range(minimum, maximum)

    def get_zoom(self):
        return self._session.get_grid_pattern_zoom()

    def get_zoom_range(self):
        return self._session.get_grid_pattern_zoom_range()


