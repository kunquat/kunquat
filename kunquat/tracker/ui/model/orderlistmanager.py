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


class OrderlistManager():

    def __init__(self):
        self._controller = None
        self._session = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()

    def set_orderlist_selection(self, selection):
        self._session.set_orderlist_selection(selection)

    def get_orderlist_selection(self):
        return self._session.get_orderlist_selection()


