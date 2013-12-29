# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013
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

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()

    def get_log(self):
        return self._session.get_event_log()


