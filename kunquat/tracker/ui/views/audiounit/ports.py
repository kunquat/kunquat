# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.views.editorlist import EditorList
from kunquat.tracker.ui.views.headerline import HeaderLine
from .audiounitupdater import AudioUnitUpdater


class Ports(QWidget, AudioUnitUpdater):

    def __init__(self):
        super().__init__()

        self._input_ports = InputPorts()
        self._output_ports = OutputPorts()

        self.add_to_updaters(self._input_ports, self._output_ports)

        h = QHBoxLayout()
        h.setContentsMargins(4, 4, 4, 4)
        h.setSpacing(4)
        h.addWidget(self._input_ports)
        h.addWidget(self._output_ports)
        self.setLayout(h)


class PortsEditor(QWidget, AudioUnitUpdater):

    def __init__(self):
        super().__init__()
        self._editor = PortList(
                self._get_add_text, self._get_port_ids, self._get_free_port_id)

        self.add_to_updaters(self._editor)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(HeaderLine(self._get_title()))
        v.addWidget(self._editor)
        self.setLayout(v)

    # Port editor interface

    def _get_title(self):
        raise NotImplementedError

    def _get_add_text(self):
        raise NotImplementedError

    def _get_free_port_id(self):
        raise NotImplementedError

    def _get_port_ids(self):
        raise NotImplementedError


class InputPorts(PortsEditor):

    def _get_title(self):
        return 'Input ports'

    def _get_add_text(self):
        return 'Add input port'

    def _get_port_ids(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        return au.get_in_ports()

    def _get_free_port_id(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        return au.get_free_input_port_id()


class OutputPorts(PortsEditor):

    def _get_title(self):
        return 'Output ports'

    def _get_add_text(self):
        return 'Add output port'

    def _get_port_ids(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        return au.get_out_ports()

    def _get_free_port_id(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        return au.get_free_output_port_id()


class PortList(EditorList, AudioUnitUpdater):

    def __init__(self, get_add_text, get_port_ids, get_free_port_id):
        super().__init__()
        self._adder = None

        self._get_add_text = get_add_text
        self._get_port_ids = get_port_ids
        self._get_free_port_id = get_free_port_id

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_all)
        self._update_all()

    def _on_teardown(self):
        self.disconnect_widgets()

    def _make_adder_widget(self):
        self._adder = PortAdder(self._get_add_text, self._get_free_port_id)
        self.add_to_updaters(self._adder)
        return self._adder

    def _get_updated_editor_count(self):
        return len(self._get_port_ids())

    def _make_editor_widget(self, index):
        editor = PortEditor(index, self._get_port_ids)
        self.add_to_updaters(editor)
        return editor

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        self.remove_from_updaters(widget)

    def _get_update_signal_type(self):
        return 'signal_au_ports_{}'.format(self._au_id)

    def _update_all(self):
        self.update_list()
        self._adder.setEnabled(self._get_free_port_id() != None)


class PortAdder(QPushButton, AudioUnitUpdater):

    def __init__(self, get_add_text, get_free_port_id):
        super().__init__(get_add_text())
        self._get_free_port_id = get_free_port_id

    def _on_setup(self):
        self.clicked.connect(self._add_port)

    def _get_update_signal_type(self):
        return 'signal_au_ports_{}'.format(self._au_id)

    def _get_connections_signals(self):
        au_signal = 'signal_connections_{}'.format(self._au_id)
        if '/' in au_signal:
            parent_signal = au_signal[:au_signal.index('/')]
        else:
            parent_signal = 'signal_connections'
        return [au_signal, parent_signal]

    def _add_port(self):
        free_port_id = self._get_free_port_id()
        if not free_port_id:
            return

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_port_existence(free_port_id, True)
        self._updater.signal_update(
                self._get_update_signal_type(), *self._get_connections_signals())


class PortEditor(QWidget, AudioUnitUpdater):

    def __init__(self, index, get_port_ids):
        super().__init__()
        self._index = index
        self._get_port_ids = get_port_ids

        self._name_editor = QLineEdit()

        self._remove_button = QPushButton()
        self._remove_button.setToolTip('Remove')
        #self._remove_button.setStyleSheet('padding: 0 -2px;')
        self._remove_button.setIconSize(QSize(16, 16))

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(2)
        h.addWidget(self._name_editor)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def _on_setup(self):
        icon_bank = self._ui_model.get_icon_bank()
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))
        self._remove_button.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)

        self.register_action(self._get_update_signal_type(), self._update_all)

        self._name_editor.textChanged.connect(self._change_name)
        self._remove_button.clicked.connect(self._remove)

        self._update_all()

    def _get_update_signal_type(self):
        return 'signal_au_ports_{}'.format(self._au_id)

    def _update_all(self):
        port_ids = self._get_port_ids()
        if self._index >= len(port_ids):
            return

        port_id = port_ids[self._index]

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        port_name = au.get_port_name(port_id) or ''

        old_block = self._name_editor.blockSignals(True)
        if self._name_editor.text() != port_name:
            self._name_editor.setText(port_name)
        self._name_editor.blockSignals(old_block)

    def _get_connections_signals(self):
        au_signal = 'signal_connections_{}'.format(self._au_id)
        if '/' in au_signal:
            parent_signal = au_signal[:au_signal.index('/')]
        else:
            parent_signal = 'signal_connections'
        return [au_signal, parent_signal]

    def _change_name(self, name):
        port_ids = self._get_port_ids()
        port_id = port_ids[self._index]

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        au.set_port_name(port_id, name)
        self._updater.signal_update(
                self._get_update_signal_type(), *self._get_connections_signals())

    def _remove(self):
        port_ids = self._get_port_ids()
        port_id = port_ids[self._index]

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        au.remove_port(port_id)
        self._updater.signal_update(
                self._get_update_signal_type(), *self._get_connections_signals())


