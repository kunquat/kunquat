# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from itertools import chain

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.tracker.ui.views.kqtcombobox import KqtComboBox
from .aukeyboardmapper import AudioUnitKeyboardMapper
from .aunumslider import AuNumSlider
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

        self._keyboard_mapper = AudioUnitKeyboardMapper()

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._test_panel.set_au_id(au_id)
        self._components.set_au_id(au_id)
        self._hits.set_au_id(au_id)
        self._expressions.set_au_id(au_id)
        self._ports.set_au_id(au_id)
        self._info_editor.set_au_id(au_id)
        self._keyboard_mapper.set_au_id(au_id)

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
        v.setContentsMargins(4, 4, 4, 4)
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
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()

    def keyReleaseEvent(self, event):
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()


class TestPanel(QWidget):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._ui_model = None

        self._test_button = TestButton()
        self._test_force = TestForce()
        self._expressions = [TestExpression(i) for i in range(2)]

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(self._test_button, 1)
        h.addWidget(self._test_force, 1)
        h.addWidget(QLabel('Channel expression:'))
        h.addWidget(self._expressions[0])
        h.addWidget(QLabel('Note expression:'))
        h.addWidget(self._expressions[1])
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._test_button.set_au_id(au_id)
        self._test_force.set_au_id(au_id)
        for expr in self._expressions:
            expr.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._test_button.set_ui_model(ui_model)
        self._test_force.set_ui_model(ui_model)
        for expr in self._expressions:
            expr.set_ui_model(ui_model)

    def unregister_updaters(self):
        for expr in self._expressions:
            expr.unregister_updaters()
        self._test_force.unregister_updaters()
        self._test_button.unregister_updaters()


class TestForce(AuNumSlider):

    def __init__(self):
        super().__init__(1, -30, 12, 'Force:')

    def _get_update_signal_type(self):
        return 'signal_au_test_force_{}'.format(self._au_id)

    def _get_audio_unit(self):
        module = self._ui_model.get_module()
        return module.get_audio_unit(self._au_id)

    def _update_value(self):
        au = self._get_audio_unit()
        self.set_number(au.get_test_force())

    def _value_changed(self, new_value):
        au = self._get_audio_unit()
        au.set_test_force(new_value)
        self._updater.signal_update(self._get_update_signal_type())


class TestExpression(KqtComboBox):

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

        if self._index == 1:
            # Apply instrument default as the initial note expression
            module = self._ui_model.get_module()
            au = module.get_audio_unit(self._au_id)
            note_expr = au.get_default_note_expression()
            au.set_test_expression(self._index, note_expr)

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
        self.setEnabled(len(expr_names) > 0)
        self.set_items(chain(['(none)'], (name for name in expr_names)))
        self.setCurrentIndex(0)
        for i, expr_name in enumerate(expr_names):
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


