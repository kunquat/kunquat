# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014
#          Toni Ruottu, Finland 2014
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
        self._pattern_num = pattern_num
        self._instance_num = instance_num
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

    def get_pattern_num(self):
        return self._pattern_num

    def get_instance_num(self):
        return self._instance_num

    def subscript(self, number):
        nums = [int(i) for i in str(number)]
        subs = [unichr(0x2080 + i) for i in nums]
        return u''.join(subs)

    def get_name(self):
        ambiguous_name = u'pattern {0}'.format(self._pattern_num)
        fullname = ambiguous_name + self.subscript(self._instance_num)
        return fullname
