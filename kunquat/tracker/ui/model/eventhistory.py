# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

class EventHistory():

    def __init__(self):
        self._controller = None
        self._context_filter = set(['fire', 'mix'])

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()

    def get_received_event_count(self):
        log = self._session.get_event_log()
        if log:
            return log[0][0] + 1
        else:
            return 0

    def get_log(self):
        return [e for e in self._session.get_event_log()
                if e[4] in self._context_filter]

    def get_event_count_with_filter(self, context):
        return sum(1 for e in self._session.get_event_log() if e[4] == context)

    def allow_context(self, context, allow):
        if allow:
            self._context_filter.add(context)
        else:
            self._context_filter.discard(context)

    def is_context_allowed(self, context):
        return context in self._context_filter


