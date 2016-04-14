# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
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

from kunquat.tracker.ui.views.keyboardmapper import KeyboardMapper
from .components import Components
from .expressions import Expressions
from .hits import Hits
from .ports import Ports
from .infoeditor import InfoEditor
from .testbutton import TestButton


class Editor(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._au_id = None
        self._control_manager = None

        self._test_panel = TestPanel()
        self._tabs = QTabWidget()

        self._components = Components()
        self._hits = Hits()
        self._expressions = Expressions()
        self._ports = Ports()
        self._info_editor = InfoEditor()

        self._keyboard_mapper = KeyboardMapper()

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._test_panel.set_au_id(au_id)
        self._components.set_au_id(au_id)
        self._hits.set_au_id(au_id)
        self._expressions.set_au_id(au_id)
        self._ports.set_au_id(au_id)
        self._info_editor.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._control_manager = ui_model.get_control_manager()
        self._test_panel.set_ui_model(ui_model)
        self._components.set_ui_model(ui_model)
        self._hits.set_ui_model(ui_model)
        self._expressions.set_ui_model(ui_model)
        self._ports.set_ui_model(ui_model)
        self._info_editor.set_ui_model(ui_model)
        self._keyboard_mapper.set_ui_model(ui_model)

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        self._tabs.addTab(self._components, 'Components')
        if au.is_instrument():
            self._tabs.addTab(self._hits, 'Hits')
            self._tabs.addTab(self._expressions, 'Expressions')
        self._tabs.addTab(self._ports, 'Ports')
        self._tabs.addTab(self._info_editor, 'Info')

        v = QVBoxLayout()
        v.setMargin(4)
        v.setSpacing(4)
        if au.is_instrument():
            v.addWidget(self._test_panel)
        v.addWidget(self._tabs)
        self.setLayout(v)

    def unregister_updaters(self):
        self._keyboard_mapper.unregister_updaters()
        self._info_editor.unregister_updaters()
        self._ports.unregister_updaters()
        self._expressions.unregister_updaters()
        self._hits.unregister_updaters()
        self._components.unregister_updaters()
        self._test_panel.unregister_updaters()

    def keyPressEvent(self, event):
        module = self._ui_model.get_module()
        control_id = module.get_control_id_by_au_id(self._au_id)
        if not control_id:
            return

        au = module.get_audio_unit(self._au_id)

        self._control_manager.set_control_id_override(control_id)
        au.set_test_expressions_enabled(True)
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()
        self._control_manager.set_control_id_override(None)
        au.set_test_expressions_enabled(False)

    def keyReleaseEvent(self, event):
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()


class TestPanel(QWidget):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._ui_model = None

        self._test_button = TestButton()
        self._expressions = [TestExpression(i) for i in range(2)]

        expr_label = QLabel('Expressions:')
        expr_label.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(self._test_button)
        h.addWidget(expr_label)
        for expr in self._expressions:
            h.addWidget(expr)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._test_button.set_au_id(au_id)
        for expr in self._expressions:
            expr.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._test_button.set_ui_model(ui_model)
        for expr in self._expressions:
            expr.set_ui_model(ui_model)

    def unregister_updaters(self):
        for expr in self._expressions:
            expr.unregister_updaters()
        self._test_button.unregister_updaters()


class TestExpression(QComboBox):

    def __init__(self, index):
        super().__init__()
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._index = index

        self.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)
        self.setSizeAdjustPolicy(QComboBox.AdjustToContents)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self, SIGNAL('currentIndexChanged(int)'), self._change_expression)

        self._update_expression_list()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_expr_list_{}'.format(self._au_id) in signals:
            self._update_expression_list()

    def _update_expression_list(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        expr_names = sorted(au.get_expression_names())

        old_block = self.blockSignals(True)
        self.clear()
        self.setEnabled(len(expr_names) > 0)
        self.addItem('(none)')
        self.setCurrentIndex(0)
        for i, expr_name in enumerate(expr_names):
            self.addItem(expr_name)
            if au.get_test_expression(self._index) == expr_name:
                self.setCurrentIndex(i + 1) # + 1 compensates for the (none) entry
        self.blockSignals(old_block)

    def _change_expression(self, item_index):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        if item_index == 0:
            au.set_test_expression(self._index, '')
        else:
            expr_name = str(self.itemText(item_index))
            au.set_test_expression(self._index, expr_name)


