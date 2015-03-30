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
from addproc import AddProc
from envgenproc import EnvgenProc
from ringmodproc import RingmodProc
from sampleproc import SampleProc
from unsupportedproc import UnsupportedProc
from kunquat.tracker.ui.views.keyboardmapper import KeyboardMapper


_proc_classes = {
    'add':      AddProc,
    'envgen':   EnvgenProc,
    'ringmod':  RingmodProc,
    'sample':   SampleProc,
}


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
        cons = _proc_classes.get(proctype, UnsupportedProc)
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


class Signals(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._voice_signals_enabled = QCheckBox('Enable voice signals')
        self._mixed_signals_enabled = QCheckBox('Enable mixed signals')

        v = QVBoxLayout()
        v.addWidget(self._voice_signals_enabled)
        v.addWidget(self._mixed_signals_enabled)
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
                self._voice_signals_enabled,
                SIGNAL('stateChanged(int)'),
                self._voice_signals_changed)
        QObject.connect(
                self._mixed_signals_enabled,
                SIGNAL('stateChanged(int)'),
                self._mixed_signals_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_signals', self._au_id, self._proc_id))

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_settings()

    def _update_settings(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        old_block = self._voice_signals_enabled.blockSignals(True)
        self._voice_signals_enabled.setCheckState(
                Qt.Checked if proc.get_voice_signals_enabled() else Qt.Unchecked)
        self._voice_signals_enabled.blockSignals(old_block)

        old_block = self._mixed_signals_enabled.blockSignals(True)
        self._mixed_signals_enabled.setCheckState(
                Qt.Checked if proc.get_mixed_signals_enabled() else Qt.Unchecked)
        self._mixed_signals_enabled.blockSignals(old_block)

    def _voice_signals_changed(self, new_state):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        enabled = (new_state == Qt.Checked)
        proc.set_voice_signals_enabled(enabled)
        self._updater.signal_update(set([self._get_update_signal_type]))

    def _mixed_signals_changed(self, new_state):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        enabled = (new_state == Qt.Checked)
        proc.set_mixed_signals_enabled(enabled)
        self._updater.signal_update(set([self._get_update_signal_type]))


