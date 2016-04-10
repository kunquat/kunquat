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
from types import NoneType

from kunquat.kunquat.limits import PATTERNS_MAX
from . import tstamp
from .gridpattern import GridPattern, STYLE_COUNT, DEFAULT_GRID_PATTERN
from .pattern import Pattern
from .patterninstance import PatternInstance


class GridManager():

    def __init__(self):
        self._controller = None
        self._ui_model = None
        self._session = None
        self._store = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._store = controller.get_store()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

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

    def get_all_grid_pattern_ids(self):
        return [u'0'] + self.get_editable_grid_pattern_ids()

    def get_editable_grid_pattern_ids(self):
        raw_dict = self._get_raw_master_dict()
        valid_keys = list(raw_dict.iterkeys())
        return valid_keys

    def select_grid_pattern(self, gp_id):
        assert isinstance(gp_id, (NoneType, unicode))
        if gp_id != self.get_selected_grid_pattern_id():
            self._session.select_grid_pattern_line(None)
        self._session.select_grid_pattern(gp_id)

    def get_selected_grid_pattern_id(self):
        return self._session.get_selected_grid_pattern_id()

    def get_grid_pattern(self, gp_id):
        assert isinstance(gp_id, (NoneType, unicode))
        gp_id = gp_id or u'0'
        gp = GridPattern(gp_id)
        gp.set_controller(self._controller)
        return gp

    def add_grid_pattern(self):
        raw_master_dict = self._get_raw_master_dict()

        # Find a new unique ID
        used_ids = set(raw_master_dict.iterkeys())
        for i in xrange(1, len(used_ids) + 2):
            new_id = unicode(i)
            if new_id not in used_ids:
                break
        else:
            assert False
        assert new_id != 0

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
        for i in xrange(32):
            line_ts_raw = list(tstamp.Tstamp(i) / 8)
            style = 0
            if i > 0:
                style = (1 if (i % 8) == 0 else (2 if (i % 2) == 0 else 4))
            lines.append([line_ts_raw, style])

        new_raw_dict = deepcopy(DEFAULT_GRID_PATTERN)
        new_raw_dict['name'] = placeholder

        raw_master_dict[new_id] = new_raw_dict
        self._set_raw_master_dict(raw_master_dict)

    def _get_all_patterns(self):
        for i in xrange(PATTERNS_MAX):
            pinst = PatternInstance(i, 0)
            pinst.set_controller(self._controller)
            pinst.set_ui_model(self._ui_model)
            pattern = pinst.get_pattern()
            if pattern.get_existence():
                yield pattern

    def remove_grid_pattern(self, gp_id):
        assert gp_id != u'0'
        assert isinstance(gp_id, unicode)

        raw_master_dict = self._get_raw_master_dict()
        del raw_master_dict[gp_id]

        transaction = { self._get_key(): raw_master_dict }

        # Remove references to the removed grid pattern
        for pattern in self._get_all_patterns():
            if pattern.get_base_grid_pattern_id() == gp_id:
                transaction.update(pattern.get_edit_set_base_grid_pattern_id(None))

        self._store.put(transaction)

    def set_grid_pattern_subdiv_part_count(self, count):
        assert count >= 2
        self._session.set_grid_pattern_subdiv_part_count(count)

    def get_grid_pattern_subdiv_part_count(self):
        return self._session.get_grid_pattern_subdiv_part_count()

    def set_grid_pattern_subdiv_line_style(self, style):
        assert 0 < style < STYLE_COUNT
        self._session.set_grid_pattern_subdiv_line_style(style)

    def get_grid_pattern_subdiv_line_style(self):
        return self._session.get_grid_pattern_subdiv_line_style()

    def set_grid_pattern_subdiv_warp(self, warp):
        assert 0 < warp < 1
        self._session.set_grid_pattern_subdiv_warp(warp)

    def get_grid_pattern_subdiv_warp(self):
        return self._session.get_grid_pattern_subdiv_warp()

    def set_zoom(self, zoom):
        self._session.set_grid_pattern_zoom(zoom)

    def set_zoom_range(self, minimum, maximum):
        self._session.set_grid_pattern_zoom_range(minimum, maximum)

    def get_zoom(self):
        return self._session.get_grid_pattern_zoom()

    def get_zoom_range(self):
        return self._session.get_grid_pattern_zoom_range()

    def set_default_grid_pattern_id(self, gp_id):
        assert isinstance(gp_id, (NoneType, unicode))
        self._session.set_default_grid_pattern_id(gp_id)

    def get_default_grid_pattern_id(self):
        return self._session.get_default_grid_pattern_id()


