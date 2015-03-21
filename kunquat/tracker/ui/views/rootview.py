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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import kunquat.tracker.cmdline as cmdline
from kunquat.tracker.ui.identifiers import *
from mainwindow import MainWindow
from aboutwindow import AboutWindow
from eventlist import EventList
from connectionswindow import ConnectionsWindow
from orderlistwindow import OrderlistWindow
from instrumentwindow import InstrumentWindow
from procwindow import ProcWindow


class RootView():

    def __init__(self):
        self._ui_model = None
        self._task_executer = None
        self._updater = None
        self._visible = set()
        self._main_window = MainWindow()
        self._about_window = None
        self._event_log = None
        self._connections = None
        self._orderlist = None
        self._instrument_windows = {}
        self._proc_windows = {}
        self._module = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._main_window.set_ui_model(ui_model)
        self._updater = self._ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._module = self._ui_model.get_module()

    def show_main_window(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_main()

    def setup_module(self):
        module = self._ui_model.get_module()
        module_path = cmdline.get_kqt_file()
        if module_path:
            module.set_path(module_path)
            module.execute_load(self._task_executer)
        else:
            module.execute_create_sandbox(self._task_executer)

    def set_task_executer(self, task_executer):
        self._task_executer = task_executer

    def _perform_updates(self, signals):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_update = visibility_manager.get_visible()

        opened = visibility_update - self._visible
        closed = self._visible - visibility_update

        for ui in opened:
            # Check settings for UI visibility
            is_show_allowed = visibility_manager.is_show_allowed()

            if ui == UI_MAIN:
                if is_show_allowed:
                    self._main_window.show()
            elif ui == UI_ABOUT:
                self._about_window = AboutWindow()
                self._about_window.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._about_window.show()
            elif ui == UI_EVENT_LOG:
                self._event_log = EventList()
                self._event_log.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._event_log.show()
            elif ui == UI_CONNECTIONS:
                self._connections = ConnectionsWindow()
                self._connections.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._connections.show()
            elif ui == UI_ORDERLIST:
                self._orderlist = OrderlistWindow()
                self._orderlist.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._orderlist.show()
            elif type(ui) == tuple and ui[0] == UI_INSTRUMENT:
                ins_id = ui[1]
                ins_window = InstrumentWindow()
                ins_window.set_ins_id(ins_id)
                ins_window.set_ui_model(self._ui_model)
                self._instrument_windows[ins_id] = ins_window
                if is_show_allowed:
                    self._instrument_windows[ins_id].show()
            elif type(ui) == tuple and ui[0] == UI_PROCESSOR:
                ins_id = ui[1]
                proc_id = ui[2]
                proc_window = ProcWindow()
                proc_window.set_ins_id(ins_id)
                proc_window.set_proc_id(proc_id)
                proc_window.set_ui_model(self._ui_model)
                self._proc_windows[(ins_id, proc_id)] = proc_window
                if is_show_allowed:
                    self._proc_windows[(ins_id, proc_id)].show()
            else:
                raise ValueError('Unsupported UI type: {}'.format(ui))

        for ui in closed:
            if ui == UI_MAIN:
                visibility_manager.hide_all()
                self._main_window.hide()
            elif ui == UI_ABOUT:
                self._about_window.unregister_updaters()
                self._about_window.deleteLater()
                self._about_window = None
            elif ui == UI_EVENT_LOG:
                self._event_log.unregister_updaters()
                self._event_log.deleteLater()
                self._event_log = None
            elif ui == UI_CONNECTIONS:
                self._connections.unregister_updaters()
                self._connections.deleteLater()
                self._connections = None
            elif ui == UI_ORDERLIST:
                self._orderlist.unregister_updaters()
                self._orderlist.deleteLater()
                self._orderlist = None
            elif type(ui) == tuple and ui[0] == UI_INSTRUMENT:
                ins_id = ui[1]
                ins_window = self._instrument_windows.pop(ins_id)
                ins_window.unregister_updaters()
                ins_window.deleteLater()
            elif type(ui) == tuple and ui[0] == UI_PROCESSOR:
                ins_id = ui[1]
                proc_id = ui[2]
                proc_window = self._proc_windows.pop((ins_id, proc_id))
                proc_window.unregister_updaters()
                proc_window.deleteLater()
            else:
                raise ValueError('Unsupported UI type: {}'.format(ui))

        self._visible = set(visibility_update)

        if self._visible:
            if 'signal_start_save_module' in signals:
                self._start_save_module()
            if 'signal_save_module_finished' in signals:
                self._on_save_module_finished()
        else:
            QApplication.quit()

        self._ui_model.clock()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._main_window.unregister_updaters()

    def _start_save_module(self):
        self._main_window.setEnabled(False)
        self._module.flush(self._execute_save_module)

    def _execute_save_module(self):
        task = self._module.execute_save(self._task_executer)

    def _on_save_module_finished(self):
        self._module.finish_save()
        self._main_window.setEnabled(True)


