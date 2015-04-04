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

from name import Name
import proctypeinfo
from kunquat.tracker.ui.views.headerline import HeaderLine
from kunquat.tracker.ui.views.keyboardmapper import KeyboardMapper


class Editor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None
        self._control_manager = None
        self._name = Name()
        self._proc_editor = None
        self._signals = Signals()

        self._keyboard_mapper = KeyboardMapper()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(5)
        v.addWidget(self._name)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._name.set_au_id(au_id)
        self._signals.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._name.set_proc_id(proc_id)
        self._signals.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        assert self._au_id != None
        assert self._proc_id != None
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._control_manager = ui_model.get_control_manager()
        self._name.set_ui_model(ui_model)
        self._signals.set_ui_model(ui_model)
        self._keyboard_mapper.set_ui_model(ui_model)
        self._updater.register_updater(self._perform_updates)

        # Get the processor type
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        proctype = proc.get_type()

        # Create the type-specific editor
        cons = proctypeinfo.get_class(proctype)
        self._proc_editor = cons()
        self._proc_editor.set_au_id(self._au_id)
        self._proc_editor.set_proc_id(self._proc_id)
        self._proc_editor.set_ui_model(self._ui_model)

        # Set up tab view
        tabs = QTabWidget()
        tabs.addTab(self._proc_editor, self._proc_editor.get_name())
        tabs.addTab(self._signals, 'Signal Types')
        self.layout().addWidget(tabs)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._keyboard_mapper.unregister_updaters()
        self._proc_editor.unregister_updaters()
        self._signals.unregister_updaters()
        self._name.unregister_updaters()

    def _perform_updates(self, signals):
        pass

    def keyPressEvent(self, event):
        # TODO: This plays the complete audio unit,
        #       change after adding processor jamming support
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


class HeaderFrame(QWidget):

    def __init__(self, header_text, contents):
        QWidget.__init__(self)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(5)
        v.addWidget(HeaderLine(header_text), 0, Qt.AlignTop)
        v.addWidget(contents, 1, Qt.AlignTop)
        self.setLayout(v)


class Signals(QWidget):

    _SIGNAL_INFO = [
        (u'voice', 'Voice signals'),
        (u'mixed', 'Mixed signals'),
    ]

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._signal_type = QComboBox()
        for info in self._SIGNAL_INFO:
            _, text = info
            self._signal_type.addItem(text)

        self._vf_cut = QCheckBox('Cut')
        self._vf_pitch = QCheckBox('Pitch')
        self._vf_force = QCheckBox('Force')
        self._vf_filter = QCheckBox('Filter')
        self._vf_panning = QCheckBox('Panning')

        vf_layout = QVBoxLayout()
        vf_layout.addWidget(self._vf_cut)
        vf_layout.addWidget(self._vf_pitch)
        vf_layout.addWidget(self._vf_force)
        vf_layout.addWidget(self._vf_filter)
        vf_layout.addWidget(self._vf_panning)

        self._vf_container = QWidget()
        self._vf_container.setLayout(vf_layout)

        v = QHBoxLayout()
        v.addWidget(HeaderFrame('Signal type', self._signal_type))
        v.addWidget(HeaderFrame('Voice features', self._vf_container))
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_settings()

        QObject.connect(
                self._signal_type,
                SIGNAL('currentIndexChanged(int)'),
                self._signal_type_changed)

        vf_info = [
            (self._vf_cut, self._vf_cut_changed),
            (self._vf_pitch, self._vf_pitch_changed),
            (self._vf_force, self._vf_force_changed),
            (self._vf_filter, self._vf_filter_changed),
            (self._vf_panning, self._vf_panning_changed),
        ]

        for info in vf_info:
            checkbox, handler = info
            QObject.connect(
                    checkbox, SIGNAL('stateChanged(int)'), handler)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_signals', self._au_id, self._proc_id))

    def _get_connections_signal_type(self):
        return '_'.join(('signal_connections', self._au_id))

    def _perform_updates(self, signals):
        update_signals = set([
            self._get_update_signal_type(), self._get_connections_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_settings()

    def _update_settings(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        connections = au.get_connections()
        signal_type = proc.get_signal_type()

        # Update signal type selector
        old_block = self._signal_type.blockSignals(True)
        type_names = [info[0] for info in self._SIGNAL_INFO]
        index = type_names.index(signal_type)
        assert 0 <= index < len(self._SIGNAL_INFO)
        self._signal_type.setCurrentIndex(index)
        self._signal_type.blockSignals(old_block)

        # Update voice features
        self._vf_container.setEnabled(signal_type == 'voice')
        self._vf_cut.setEnabled(not connections.is_proc_connected_to_out(self._proc_id))

        vf_info = [
            (self._vf_cut, proc.get_vf_cut),
            (self._vf_pitch, proc.get_vf_pitch),
            (self._vf_force, proc.get_vf_force),
            (self._vf_filter, proc.get_vf_filter),
            (self._vf_panning, proc.get_vf_panning),
        ]

        for info in vf_info:
            checkbox, get_vf = info
            old_block = checkbox.blockSignals(True)
            checkbox.setCheckState(Qt.Checked if get_vf(0) else Qt.Unchecked)
            checkbox.blockSignals(old_block)

    def _signal_type_changed(self, index):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        proc.set_signal_type(self._SIGNAL_INFO[index][0])
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _vf_changed(self, state, vf_name):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        enabled = (state == Qt.Checked)

        vf_info = {
            'cut': proc.set_vf_cut,
            'pitch': proc.set_vf_pitch,
            'force': proc.set_vf_force,
            'filter': proc.set_vf_filter,
            'panning': proc.set_vf_panning,
        }

        vf_info[vf_name](0, enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _vf_cut_changed(self, state):
        self._vf_changed(state, 'cut')

    def _vf_pitch_changed(self, state):
        self._vf_changed(state, 'pitch')

    def _vf_force_changed(self, state):
        self._vf_changed(state, 'force')

    def _vf_filter_changed(self, state):
        self._vf_changed(state, 'filter')

    def _vf_panning_changed(self, state):
        self._vf_changed(state, 'panning')


