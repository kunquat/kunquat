# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import string

from kunquat.tracker.ui.qt import *

import kunquat.tracker.ui.model.tstamp as tstamp
from .editorlist import EditorList
from .headerline import HeaderLine
from .updater import Updater
from .varvalidators import *


class IAControls(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._inf_toggle = InfiniteToggle()
        self._runtime_var_list = RuntimeVarList()

        self.add_to_updaters(self._inf_toggle, self._runtime_var_list)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._inf_toggle)
        v.addWidget(HeaderLine('Runtime environment state'))
        v.addWidget(self._runtime_var_list)
        self.setLayout(v)


class InfiniteToggle(QCheckBox, Updater):

    def __init__(self):
        super().__init__()
        self.setText('Enable infinite mode')

    def _on_setup(self):
        self.register_action('infinite_mode', self._update_inf_setting)
        self.stateChanged.connect(self._toggle_infinite_mode)
        self._update_inf_setting()

    def _update_inf_setting(self):
        playback_mgr = self._ui_model.get_playback_manager()
        old_block = self.blockSignals(True)
        self.setCheckState(
                Qt.Checked if playback_mgr.get_infinite_mode() else Qt.Unchecked)
        self.blockSignals(old_block)

    def _toggle_infinite_mode(self, new_state):
        enabled = (new_state == Qt.Checked)

        playback_mgr = self._ui_model.get_playback_manager()
        playback_mgr.set_infinite_mode(enabled)

        self._updater.signal_update('infinite_mode')


class RuntimeVarList(EditorList, Updater):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.register_action('signal_runtime_env', self.update_list)
        self.register_action('signal_environment', self.update_list)
        self.update_list()

    def _on_teardown(self):
        self.disconnect_widgets()

    def _get_var_names(self):
        module = self._ui_model.get_module()
        env = module.get_environment()
        var_names = env.get_var_names()
        return var_names

    def _get_updated_editor_count(self):
        var_names = self._get_var_names()
        return len(var_names)

    def _make_editor_widget(self, index):
        var_names = self._get_var_names()

        editor = RuntimeVarEditor()
        self.add_to_updaters(editor)
        editor.update_name(var_names[index])
        return editor

    def _update_editor(self, index, editor):
        var_names = self._get_var_names()
        editor.update_name(var_names[index])

    def _disconnect_widget(self, widget):
        self.remove_from_updaters(widget)


class RuntimeVarEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._name = None
        self._header = QLabel()
        self._editor = RuntimeVarValueEditor()

        self.add_to_updaters(self._editor)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(self._header, 2)
        h.addWidget(self._editor, 1)
        self.setLayout(h)

    def update_name(self, name):
        self._name = name
        self._header.setText(self._name)
        self._editor.set_var_name(self._name)


class RuntimeVarValueEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._var_name = None

        self._update_flag = False

        self._editors = {
            bool:           QCheckBox(),
            int:            QLineEdit(),
            float:          QLineEdit(),
            tstamp.Tstamp:  QLineEdit(),
        }

        self._editors[bool].setText(' ') # work around broken clickable region

        self._editors[int].setValidator(IntValidator())
        self._editors[float].setValidator(FloatValidator())
        self._editors[tstamp.Tstamp].setValidator(FloatValidator())

    def _on_setup(self):
        module = self._ui_model.get_module()
        env = module.get_environment()
        var_types = env.get_var_types()

        s = QStackedLayout()
        for t in var_types:
            s.addWidget(self._editors[t])
        self.setLayout(s)

        self._editors[bool].stateChanged.connect(self._change_bool_value)
        self._editors[int].editingFinished.connect(self._change_int_value)
        self._editors[float].editingFinished.connect(self._change_float_value)
        self._editors[tstamp.Tstamp].editingFinished.connect(self._change_tstamp_value)

    def set_var_name(self, name):
        self._var_name = name

        editor = self.layout().currentWidget()
        if ((not editor) or editor.hasFocus()) and (editor != self._editors[bool]):
            self._update_flag = True
        else:
            self._set_var_value()

    def _set_var_value(self):
        module = self._ui_model.get_module()
        env = module.get_environment()
        playback_mgr = self._ui_model.get_playback_manager()

        var_types = env.get_var_types()
        var_type = env.get_var_type(self._var_name)
        var_type_index = var_types.index(var_type)
        self.layout().setCurrentIndex(var_type_index)

        runtime_var_value = playback_mgr.get_runtime_var_value(self._var_name)
        if runtime_var_value == None:
            runtime_var_value = env.get_var_init_value(self._var_name)

        editor = self._editors[var_type]
        old_block = editor.blockSignals(True)
        if var_type == bool:
            editor.setCheckState(Qt.Checked if runtime_var_value else Qt.Unchecked)
        elif var_type == int:
            editor.setText(str(int(runtime_var_value)))
        elif var_type == float:
            editor.setText(str(float(runtime_var_value)))
        elif var_type == tstamp.Tstamp:
            editor.setText(str(float(runtime_var_value)))
        else:
            assert False
        editor.blockSignals(old_block)

    def _change_bool_value(self, new_state):
        enabled = (new_state == Qt.Checked)

        playback_mgr = self._ui_model.get_playback_manager()
        playback_mgr.set_runtime_var_value(self._var_name, enabled)

    def _change_int_value(self):
        value = int(self._editors[int].text())

        playback_mgr = self._ui_model.get_playback_manager()
        playback_mgr.set_runtime_var_value(self._var_name, value)

    def _change_float_value(self):
        value = float(self._editors[float].text())

        playback_mgr = self._ui_model.get_playback_manager()
        playback_mgr.set_runtime_var_value(self._var_name, value)

    def _change_tstamp_value(self):
        value = tstamp.Tstamp(float(self._editors[tstamp.Tstamp].text()))

        playback_mgr = self._ui_model.get_playback_manager()
        playback_mgr.set_runtime_var_value(self._var_name, value)

    def focusOutEvent(self, event):
        if self._update_flag:
            self._set_var_value()
            self._update_flag = False


