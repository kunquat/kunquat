# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from pattern import Pattern


class PatternInstance():

    def __init__(self, pattern_num, instance_num):
        self._pattern_id = 'pat_{:03x}'.format(pattern_num)
        self._instance_id = 'instance_{:03x}'.format(instance_num)
        self._store = None
        self._controller = None

    def set_controller(self, controller):
        self._store = controller.get_store()
        self._controller = controller

    def get_pattern(self):
        pattern = Pattern(self._pattern_id)
        pattern.set_controller(self._controller)
        return pattern


