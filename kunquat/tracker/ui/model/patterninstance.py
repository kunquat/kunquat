# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2016
#          Toni Ruottu, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.kunquat.limits import *
from .column import Column
from .pattern import Pattern


class PatternInstance():

    def __init__(self, pattern_num, instance_num):
        self._pattern_num = pattern_num
        self._instance_num = instance_num
        self._pattern_id = 'pat_{:03x}'.format(pattern_num)
        self._instance_id = 'instance_{:03x}'.format(instance_num)
        self._store = None
        self._controller = None
        self._ui_model = None

    def __eq__(self, other):
        if isinstance(other, PatternInstance):
            return (self.get_pattern_num() == other.get_pattern_num() and
                    self.get_instance_num() == other.get_instance_num())
        return False

    def get_id(self):
        return self._instance_id

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def get_pattern(self):
        pattern = Pattern(self._pattern_id)
        pattern.set_controller(self._controller)
        return pattern

    def get_pattern_num(self):
        return self._pattern_num

    def get_instance_num(self):
        return self._instance_num

    def get_column(self, col_index):
        assert 0 <= col_index < COLUMNS_MAX
        column = Column(self._pattern_num, self._instance_num, col_index)
        column.set_controller(self._controller)
        column.set_ui_model(self._ui_model)
        return column

    def subscript(self, number):
        nums = [int(i) for i in str(number)]
        subs = [unichr(0x2080 + i) for i in nums]
        return u''.join(subs)

    def get_name(self):
        ambiguous_name = u'pattern {0}'.format(self._pattern_num)
        fullname = ambiguous_name + self.subscript(self._instance_num)
        return fullname

    def _get_full_id(self):
        return '{}/{}'.format(self._pattern_id, self._instance_id)

    def get_edit_create_pattern_instance(self):
        key = '{}/p_manifest.json'.format(self._get_full_id())
        edit = { key: {} }
        return edit

    def get_edit_remove_pattern_instance(self):
        edit = {}
        for key in self._store:
            start = '{}/'.format(self._get_full_id())
            if key.startswith(start):
                edit[key] = None
        return edit


