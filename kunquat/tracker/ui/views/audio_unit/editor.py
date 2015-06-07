# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2015
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

from force_editor import ForceEditor
from filtereditor import FilterEditor
from infoeditor import InfoEditor
from testbutton import TestButton
from kunquat.tracker.ui.views.connectionseditor import ConnectionsEditor
from kunquat.tracker.ui.views.keyboardmapper import KeyboardMapper


class Editor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._au_id = None
        self._control_manager = None

        self._test_button = TestButton()
        self._tabs = QTabWidget()

        self._force_editor = ForceEditor()
        self._filter_editor = FilterEditor()
        self._conns_editor = ConnectionsEditor()
        self._info_editor = InfoEditor()
        self._tabs.addTab(self._force_editor, 'Force')
        self._tabs.addTab(self._filter_editor, 'Filter')
        self._tabs.addTab(self._conns_editor, 'Connections')
        self._tabs.addTab(self._info_editor, 'Info')

        self._keyboard_mapper = KeyboardMapper()

        v = QVBoxLayout()
        v.setMargin(4)
        v.setSpacing(4)
        v.addWidget(self._test_button)
        v.addWidget(self._tabs)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._test_button.set_au_id(au_id)
        self._force_editor.set_au_id(au_id)
        self._filter_editor.set_au_id(au_id)
        self._conns_editor.set_au_id(au_id)
        self._info_editor.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._control_manager = ui_model.get_control_manager()
        self._test_button.set_ui_model(ui_model)
        self._force_editor.set_ui_model(ui_model)
        self._filter_editor.set_ui_model(ui_model)
        self._conns_editor.set_ui_model(ui_model)
        self._info_editor.set_ui_model(ui_model)
        self._keyboard_mapper.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._keyboard_mapper.unregister_updaters()
        self._info_editor.unregister_updaters()
        self._conns_editor.unregister_updaters()
        self._filter_editor.unregister_updaters()
        self._force_editor.unregister_updaters()
        self._test_button.unregister_updaters()

    def keyPressEvent(self, event):
        module = self._ui_model.get_module()
        control_id = module.get_control_id_by_au_id(self._au_id)
        if not control_id:
            return

        self._control_manager.set_control_id_override(control_id)
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()
        self._control_manager.set_control_id_override(None)

    def keyReleaseEvent(self, event):
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()


