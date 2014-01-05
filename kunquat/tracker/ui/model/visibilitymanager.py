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

from kunquat.tracker.ui.identifiers import *


class VisibilityManager():

    def __init__(self):
        self._controller = None
        self._session = None
        self._updater = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._updater = controller.get_updater()

    def show_about(self):
        self._session.show_ui(UI_ABOUT)
        self._updater.signal_update()

    def hide_about(self):
        self._session.hide_ui(UI_ABOUT)
        self._updater.signal_update()

    def get_visible(self):
        return self._session.get_visible()


