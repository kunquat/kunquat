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

import string

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from editorlist import EditorList
from headerline import HeaderLine


class IAControls(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._inf_toggle = InfiniteToggle()
        self._runtime_var_list = RuntimeVarList()

        v = QVBoxLayout()
        v.setMargin(4)
        v.setSpacing(4)
        v.addWidget(self._inf_toggle)
        v.addWidget(HeaderLine('Runtime environment state'))
        v.addWidget(self._runtime_var_list)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._inf_toggle.set_ui_model(ui_model)
        self._runtime_var_list.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._runtime_var_list.unregister_updaters()
        self._inf_toggle.unregister_updaters()


class InfiniteToggle(QCheckBox):

    def __init__(self):
        QCheckBox.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setText('Enable infinite mode')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('stateChanged(int)'), self._toggle_infinite_mode)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'infinite_mode' in signals:
            self._update_inf_setting()

    def _update_inf_setting(self):
        playback_manager = self._ui_model.get_playback_manager()
        old_block = self.blockSignals(True)
        self.setCheckState(
                Qt.Checked if playback_manager.get_infinite_mode() else Qt.Unchecked)
        self.blockSignals(old_block)

    def _toggle_infinite_mode(self, new_state):
        enabled = (new_state == Qt.Checked)

        playback_manager = self._ui_model.get_playback_manager()
        playback_manager.set_infinite_mode(enabled)

        self._updater.signal_update(set(['infinite_mode']))


class RuntimeVarList(EditorList):

    def __init__(self):
        EditorList.__init__(self)
        self._ui_model = None
        self._updater = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self.update_list()

    def unregister_updaters(self):
        self.disconnect_widgets()
        self._updater.unregister_updater(self._perform_updates)

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
        editor.set_ui_model(self._ui_model)
        editor.update_name(var_names[index])
        return editor

    def _update_editor(self, index, editor):
        var_names = self._get_var_names()
        editor.update_name(var_names[index])

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()

    def _perform_updates(self, signals):
        if 'signal_environment' in signals:
            self.update_list()


class RuntimeVarEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._name = None
        self._header = QLabel()

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addWidget(self._header)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def update_name(self, name):
        self._name = name
        self._header.setText(self._name)

    def _perform_updates(self, signals):
        pass


