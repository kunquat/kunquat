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

from mainwindow import MainWindow


class RootView():

    def __init__(self):
        self._ui_model = None
        self._main_window = MainWindow()

    def show_main_window(self):
        # TODO: Check settings for UI visibility
        self._main_window.show()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._main_window.set_ui_model(ui_model)


