# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.views.headerline import HeaderLine
from kunquat.tracker.ui.views.kqtcombobox import KqtComboBox
from .infoeditor import InfoEditor
from .prockeyboardmapper import ProcessorKeyboardMapper
from .processorupdater import ProcessorUpdater
from . import proctypeinfo


class Editor(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._control_mgr = None
        self._proc_editor = None

        self._signals = Signals()
        self._info_editor = InfoEditor()

        self._keyboard_mapper = ProcessorKeyboardMapper()

        self._test_output = QCheckBox('Test output')

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(5)
        v.addWidget(self._test_output)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._signals, self._info_editor, self._keyboard_mapper)
        self.register_action(
                'signal_proc_test_output_{}'.format(self._proc_id),
                self._update_test_toggle)
        self.register_action(
                'signal_proc_signals_{}'.format(self._proc_id),
                self._update_test_toggle)
        self.register_action('signal_style_changed', self._update_style)

        self._control_mgr = self._ui_model.get_control_manager()

        # Get the processor type
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        proctype = proc.get_type()

        # Create the type-specific editor
        cons = proctypeinfo.get_class(proctype)
        self._proc_editor = cons()
        self.add_to_updaters(self._proc_editor)

        # Set up tab view
        tabs = QTabWidget()
        tabs.addTab(self._proc_editor, self._proc_editor.get_name())
        tabs.addTab(self._signals, 'Signal Types')
        tabs.addTab(self._info_editor, 'Info')
        self.layout().addWidget(tabs)

        # Test output toggle
        self._test_output.stateChanged.connect(self._change_test_output_state)
        self._update_test_toggle()

        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

    def _is_processor_testable(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        return proc.get_existence() and (proc.get_signal_type() == 'voice')

    def _update_test_toggle(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        if not proc.get_existence():
            return

        old_block = self._test_output.blockSignals(True)
        if self._is_processor_testable():
            self._test_output.setEnabled(True)
            enabled = self._control_mgr.is_processor_testing_enabled(self._proc_id)
            self._test_output.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        else:
            self._test_output.setCheckState(Qt.Unchecked)
            self._test_output.setEnabled(False)
        self._test_output.blockSignals(old_block)

    def _change_test_output_state(self, state):
        enabled = (state == Qt.Checked)
        self._control_mgr.set_processor_testing_enabled(self._proc_id, enabled)
        self._updater.signal_update(
                'signal_proc_test_output{}'.format(self._proc_id))

    def keyPressEvent(self, event):
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()

    def keyReleaseEvent(self, event):
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()


class Signals(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._voice_signals = QRadioButton('Voice signals')
        self._mixed_signals = QRadioButton('Mixed signals')

        self._header = HeaderLine('Signal type')

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.addWidget(self._header)
        v.addWidget(self._voice_signals)
        v.addWidget(self._mixed_signals)
        v.addStretch(1)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_settings)
        self.register_action(self._get_connections_signal_type(), self._update_settings)
        self.register_action('signal_style_changed', self._update_style)

        self._voice_signals.clicked.connect(self._set_voice_signals)
        self._mixed_signals.clicked.connect(self._set_mixed_signals)

        self._update_style()
        self._update_settings()

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_signals', self._proc_id))

    def _get_connections_signal_type(self):
        return '_'.join(('signal_connections', self._au_id))

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def _update_settings(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        # Don't try to update if we have been removed
        if not proc.get_existence():
            return

        connections = au.get_connections()
        signal_type = proc.get_signal_type()

        # Update signal type selector
        signal_types = { 'voice': self._voice_signals, 'mixed': self._mixed_signals }
        widget = signal_types[signal_type]
        old_block = widget.blockSignals(True)
        widget.setChecked(True)
        widget.blockSignals(old_block)

    def _set_signals_type(self, signals_type):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        update_signals = [
            self._get_update_signal_type(), self._get_connections_signal_type()]

        proc.set_signal_type(signals_type)
        self._updater.signal_update(*update_signals)

    def _set_voice_signals(self):
        self._set_signals_type('voice')

    def _set_mixed_signals(self):
        self._set_signals_type('mixed')


