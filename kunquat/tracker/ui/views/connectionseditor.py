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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from connections import Connections
import processor.proctypeinfo as proctypeinfo


class ConnectionsEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._toolbar = ConnectionsToolBar()
        self._connections = Connections()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._connections)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._toolbar.set_au_id(au_id)
        self._connections.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._toolbar.set_ui_model(ui_model)
        self._connections.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._connections.unregister_updaters()
        self._toolbar.unregister_updaters()


class ConnectionsToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._add_ins_button = QToolButton()
        self._add_ins_button.setText('Add instrument')

        self._add_proc_button = QToolButton()
        self._add_proc_button.setText('Add processor')
        self._add_proc_button.setPopupMode(QToolButton.InstantPopup)

        procmenu = QMenu()
        for info in proctypeinfo.get_sorted_type_info_list():
            proc_type, cls = info
            action = QAction(procmenu)
            action.setText(cls.get_name())
            action.setData(proc_type)
            procmenu.addAction(action)

        self._add_proc_button.setMenu(procmenu)

    def set_au_id(self, au_id):
        assert self._ui_model == None, "Audio unit ID must be set before UI model"
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        if self._au_id == None:
            self.addWidget(self._add_ins_button)
            QObject.connect(
                    self._add_ins_button,
                    SIGNAL('clicked()'),
                    self._add_instrument)

        if self._au_id != None:
            self.addWidget(self._add_proc_button)
            QObject.connect(
                    self._add_proc_button.menu(),
                    SIGNAL('triggered(QAction*)'),
                    self._add_processor)

    def unregister_updaters(self):
        pass

    def _add_instrument(self):
        module = self._ui_model.get_module()
        new_control_id = module.get_free_control_id()
        new_au_id = module.get_free_au_id()
        if new_control_id and new_au_id:
            module.add_instrument(new_au_id)
            module.add_control(new_control_id)
            control = module.get_control(new_control_id)
            control.connect_to_au(new_au_id)
            self._updater.signal_update(set(['signal_connections']))

    def _add_processor(self, action):
        assert action != None
        proc_type = str(action.data().toString())

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        new_proc_id = au.get_free_processor_id()
        if new_proc_id != None:
            au.add_processor(new_proc_id, proc_type)
            update_signal = '_'.join(('signal_connections', self._au_id))
            self._updater.signal_update(set([update_signal]))


