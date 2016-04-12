# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
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

from kunquat.tracker.ui.views.editorlist import EditorList
from kunquat.tracker.ui.views.headerline import HeaderLine


class Ports(QWidget):

    def __init__(self):
        super().__init__()

        self._input_ports = InputPorts()
        self._output_ports = OutputPorts()

        h = QHBoxLayout()
        h.setMargin(4)
        h.setSpacing(4)
        h.addWidget(self._input_ports)
        h.addWidget(self._output_ports)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._input_ports.set_au_id(au_id)
        self._output_ports.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._input_ports.set_ui_model(ui_model)
        self._output_ports.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._output_ports.unregister_updaters()
        self._input_ports.unregister_updaters()


class PortsEditor(QWidget):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._ui_model = None

        self._editor = PortList(
                self._get_add_text, self._get_port_ids, self._get_free_port_id)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine(self._get_title()))
        v.addWidget(self._editor)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._editor.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._editor.unregister_updaters()

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


class PortList(EditorList):

    def __init__(self, get_add_text, get_port_ids, get_free_port_id):
        super().__init__()
        self._au_id = None
        self._ui_model = None

        self._adder = None

        self._get_add_text = get_add_text
        self._get_port_ids = get_port_ids
        self._get_free_port_id = get_free_port_id

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_all()

    def unregister_updaters(self):
        self.disconnect_widgets()
        self._updater.unregister_updater(self._perform_updates)

    def _make_adder_widget(self):
        self._adder = PortAdder(self._get_add_text, self._get_free_port_id)
        self._adder.set_au_id(self._au_id)
        self._adder.set_ui_model(self._ui_model)
        return self._adder

    def _get_updated_editor_count(self):
        return len(self._get_port_ids())

    def _make_editor_widget(self, index):
        editor = PortEditor(index, self._get_port_ids)
        editor.set_au_id(self._au_id)
        editor.set_ui_model(self._ui_model)
        return editor

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()

    def _get_update_signal_type(self):
        return 'signal_au_ports_{}'.format(self._au_id)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_all()

    def _update_all(self):
        self.update_list()
        self._adder.setEnabled(self._get_free_port_id() != None)


class PortAdder(QPushButton):

    def __init__(self, get_add_text, get_free_port_id):
        super().__init__(get_add_text())
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._get_free_port_id = get_free_port_id

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        QObject.connect(self, SIGNAL('clicked()'), self._add_port)

    def unregister_updaters(self):
        pass

    def _get_update_signal_type(self):
        return 'signal_au_ports_{}'.format(self._au_id)

    def _get_connections_signals(self):
        au_signal = 'signal_connections_{}'.format(self._au_id)
        if '/' in au_signal:
            parent_signal = au_signal[:au_signal.index('/')]
        else:
            parent_signal = 'signal_connections'
        return set([au_signal, parent_signal])

    def _add_port(self):
        free_port_id = self._get_free_port_id()
        if not free_port_id:
            return

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_port_existence(free_port_id, True)
        self._updater.signal_update(
                set([self._get_update_signal_type()]) | self._get_connections_signals())


class PortEditor(QWidget):

    def __init__(self, index, get_port_ids):
        super().__init__()
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._index = index
        self._get_port_ids = get_port_ids

        self._name_editor = QLineEdit()

        self._remove_button = QPushButton()
        self._remove_button.setToolTip('Remove')
        self._remove_button.setStyleSheet('padding: 0 -2px;')

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(self._name_editor)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))
        self._remove_button.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)

        QObject.connect(
                self._name_editor,
                SIGNAL('textChanged(const QString&)'),
                self._change_name)

        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return 'signal_au_ports_{}'.format(self._au_id)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_all()

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
        return set([au_signal, parent_signal])

    def _change_name(self, name):
        port_ids = self._get_port_ids()
        port_id = port_ids[self._index]

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        au.set_port_name(port_id, name)
        self._updater.signal_update(
                set([self._get_update_signal_type()]) | self._get_connections_signals())

    def _remove(self):
        port_ids = self._get_port_ids()
        port_id = port_ids[self._index]

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        au.remove_port(port_id)
        self._updater.signal_update(
                set([self._get_update_signal_type()]) | self._get_connections_signals())


