# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2015
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
        self._is_ui_visible = True
        self._is_closing = False

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._updater = controller.get_updater()

    def run_hidden(self):
        self._is_ui_visible = False

    def is_show_allowed(self):
        return self._is_ui_visible and not self._is_closing

    def hide_all(self):
        self._is_closing = True
        self._session.hide_all()

    def show_main(self):
        if self._is_closing:
            return
        self._session.show_ui(UI_MAIN)
        self._updater.signal_update()

    def hide_main(self):
        self._session.hide_ui(UI_MAIN)
        self._updater.signal_update()

    def show_about(self):
        if self._is_closing:
            return
        self._session.show_ui(UI_ABOUT)
        self._updater.signal_update()

    def hide_about(self):
        self._session.hide_ui(UI_ABOUT)
        self._updater.signal_update()

    def show_event_log(self):
        if self._is_closing:
            return
        self._session.show_ui(UI_EVENT_LOG)
        self._updater.signal_update()

    def hide_event_log(self):
        self._session.hide_ui(UI_EVENT_LOG)
        self._updater.signal_update()

    def show_connections(self):
        self._session.show_ui(UI_CONNECTIONS)
        self._updater.signal_update()

    def hide_connections(self):
        self._session.hide_ui(UI_CONNECTIONS)
        self._updater.signal_update()

    def show_instrument(self, index):
        self._session.show_ui((UI_INSTRUMENT, index))
        self._updater.signal_update()

    def hide_instrument(self, index):
        self._session.hide_ui((UI_INSTRUMENT, index))
        self._updater.signal_update()

    def show_orderlist(self):
        self._session.show_ui(UI_ORDERLIST)
        self._updater.signal_update()

    def hide_orderlist(self):
        self._session.hide_ui(UI_ORDERLIST)
        self._updater.signal_update()

    def show_processor(self, ins_id, proc_id):
        self._session.show_ui((UI_PROCESSOR, ins_id, proc_id))
        self._updater.signal_update()

    def hide_processor(self, ins_id, proc_id):
        self._session.hide_ui((UI_PROCESSOR, ins_id, proc_id))
        self._updater.signal_update()

    def get_visible(self):
        return self._session.get_visible()


