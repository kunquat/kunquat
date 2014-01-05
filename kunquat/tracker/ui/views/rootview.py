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
from mainwindow import MainWindow
from aboutwindow import AboutWindow


class RootView():

    def __init__(self):
        self._ui_model = None
        self._updater = None
        self._visible = set()
        self._main_window = MainWindow()
        self._about_window = None

    def show_main_window(self):
        # TODO: Check settings for UI visibility
        self._main_window.show()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._main_window.set_ui_model(ui_model)
        self._updater = self._ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def _perform_updates(self, signals):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_update = visibility_manager.get_visible()

        opened = visibility_update - self._visible
        closed = self._visible - visibility_update

        for ui in opened:
            if ui == UI_ABOUT: # TODO: check ui type
                self._about_window = AboutWindow()
                self._about_window.set_ui_model(self._ui_model)
                # TODO: Check settings for UI visibility
                self._about_window.show()

        for ui in closed:
            if ui == UI_ABOUT:
                self._about_window.unregister_updaters()
                self._about_window.deleteLater()
                self._about_window = None

        self._visible = set(visibility_update)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        # TODO: forward call to main window


