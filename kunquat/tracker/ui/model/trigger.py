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

import kunquat.kunquat.events as events


class Trigger():

    def __init__(self, trigger_type, argument):
        assert (argument == None) or (type(argument) == unicode)
        self._type = trigger_type
        self._argument = argument

    def get_type(self):
        return self._type

    def get_argument(self):
        return self._argument

    def get_argument_type(self):
        try:
            return events.trigger_events_by_name[self._type]['arg_type']
        except KeyError:
            return None


