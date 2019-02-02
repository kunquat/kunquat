# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.kunquat.limits import *
from .audiounit.audiounitupdater import AudioUnitUpdater
from .connections import Connections
from .filedialog import FileDialog
from .processor import proctypeinfo
from .kqtcombobox import KqtComboBox
from .kqtutils import get_au_file_info, open_kqt_au
from .saving import get_instrument_save_path, get_effect_save_path
from .stylecreator import StyleCreator


class ConnectionsEditor(QWidget, AudioUnitUpdater):

    def __init__(self):
        super().__init__()

        self._toolbar = ConnectionsToolBar()
        self._connections = Connections()

        self.add_to_updaters(self._toolbar, self._connections)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._connections)
        self.setLayout(v)


class ConnectionsToolBar(QToolBar, AudioUnitUpdater):

    def __init__(self):
        super().__init__()

        self._add_ins_button = QToolButton()
        self._add_ins_button.setText('Add instrument')

        self._add_proc_button = QToolButton()
        self._add_proc_button.setText('Add processor')
        self._add_proc_button.setPopupMode(QToolButton.InstantPopup)

        procmenu = QMenu()
        for info in proctypeinfo.get_sorted_type_info_list():
            proc_type, cls = info
            action = QAction(procmenu)
            action.setText(cls.get_name())
            action.setData(proc_type)
            procmenu.addAction(action)

        self._add_proc_button.setMenu(procmenu)

        self._add_effect_button = QToolButton()
        self._add_effect_button.setText('Add effect')

        self._import_button = QToolButton()

        self._hit_edit = None
        self._hit_selector = None

        self._expr_edit = None
        self._expr_selector = None

        self._export_button = QToolButton()
        self._export_button.setText('Export')

    def _on_setup(self):
        # Instrument or processor adder
        if self._au_id == None:
            self.addWidget(self._add_ins_button)
            self._add_ins_button.clicked.connect(self._add_instrument)
        else:
            self.addWidget(self._add_proc_button)
            self._add_proc_button.menu().triggered.connect(self._add_processor)

        # Effect adder if allowed
        is_effect_allowed = (self._au_id == None)
        if self._au_id != None:
            module = self._ui_model.get_module()
            au = module.get_audio_unit(self._au_id)
            is_effect_allowed = au.is_instrument()
        if is_effect_allowed:
            self.addWidget(self._add_effect_button)
            self._add_effect_button.clicked.connect(self._add_effect)

        # Import button if allowed
        is_import_allowed = is_effect_allowed
        if is_import_allowed:
            if self._au_id == None:
                text = 'Import instrument/effect'
            else:
                text = 'Import effect'
            self._import_button.setText(text)
            self.addWidget(self._import_button)
            self._import_button.clicked.connect(self._import_au)

        if self._au_id != None:
            module = self._ui_model.get_module()
            au = module.get_audio_unit(self._au_id)

            if au.is_instrument():
                # Hit editing controls
                self.addSeparator()

                self._hit_edit = HitEditingToggle()
                self.add_to_updaters(self._hit_edit)
                self.addWidget(self._hit_edit)

                self._hit_selector = HitSelector()
                self.add_to_updaters(self._hit_selector)
                self.addWidget(self._hit_selector)

                # Expression controls
                self.addSeparator()

                self._expr_edit = ExpressionEditingToggle()
                self.add_to_updaters(self._expr_edit)
                self.addWidget(self._expr_edit)

                self._expr_selector = ExpressionSelector()
                self.add_to_updaters(self._expr_selector)
                self.addWidget(self._expr_selector)

            # Export
            self.addWidget(self._export_button)
            self._export_button.clicked.connect(self._export_au)

    def _add_instrument(self):
        module = self._ui_model.get_module()
        new_control_id = module.get_free_control_id()
        new_au_id = module.get_free_au_id()
        if new_control_id and new_au_id:
            module.add_instrument(new_au_id)
            module.add_control(new_control_id)
            control = module.get_control(new_control_id)
            control.connect_to_au(new_au_id)
            update_signals = ['signal_connections', 'signal_controls']
            self._updater.signal_update(*update_signals)

    def _add_processor(self, action):
        assert action != None
        proc_type = action.data()

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        new_proc_id = au.get_free_processor_id()
        if new_proc_id != None:
            au.add_processor(new_proc_id, proc_type)
            update_signal = '_'.join(('signal_connections', self._au_id))
            self._updater.signal_update(update_signal)

    def _add_effect(self):
        module = self._ui_model.get_module()
        is_control_needed = True
        parent_device = module
        if self._au_id != None:
            is_control_needed = False
            parent_device = module.get_audio_unit(self._au_id)

        new_au_id = parent_device.get_free_au_id()
        new_control_id = module.get_free_control_id() if is_control_needed else None

        if (not is_control_needed or new_control_id) and new_au_id:
            parent_device.add_effect(new_au_id)
            update_signals = []
            if is_control_needed:
                parent_device.add_control(new_control_id)
                control = parent_device.get_control(new_control_id)
                control.connect_to_au(new_au_id)
                update_signals.append('signal_controls')

            update_signal = 'signal_connections'
            if self._au_id != None:
                update_signal = '_'.join((update_signal, self._au_id))
            update_signals.append(update_signal)

            self._updater.signal_update(*update_signals)

    def _import_au(self):
        module = self._ui_model.get_module()

        if self._au_id == None:
            au_path, au_type = get_au_file_info(
                    self._ui_model, FileDialog.TYPE_KQTI | FileDialog.TYPE_KQTE)
        else:
            au_path, au_type = get_au_file_info(self._ui_model, FileDialog.TYPE_KQTE)

        if au_path and (au_type in (FileDialog.TYPE_KQTI, FileDialog.TYPE_KQTE)):
            container = module
            if self._au_id != None:
                container = module.get_audio_unit(self._au_id)
            open_kqt_au(au_path, au_type, self._ui_model, container)

    def _export_au(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        if au.is_instrument():
            au_path = get_instrument_save_path(self._ui_model, au.get_name())
        else:
            au_path = get_effect_save_path(self._ui_model, au.get_name())
        if not au_path:
            return

        au.start_export_au(au_path)


def _get_au_hit_signal_type(au_id):
    return 'signal_hit_{}'.format(au_id)


def _get_au_conns_edit_signal_type(au_id):
    return 'signal_au_conns_edit_mode_{}'.format(au_id)


class EditingToggle(QPushButton, AudioUnitUpdater):

    def __init__(self, text):
        super().__init__(text)

        self._style_creator = StyleCreator()
        self._style_sheet = ''

        self.setCheckable(True)

    def _on_setup(self):
        self.clicked.connect(self._change_enabled)

        self._style_creator.set_ui_model(self._ui_model)

        for signal in self._get_update_signal_types():
            self.register_action(signal, self._update_enabled)
        self.register_action('signal_style_changed', self._update_style)

        self._style_sheet = QApplication.instance().styleSheet()
        self._update_enabled()
        self._update_style()

    def _on_teardown(self):
        self._style_creator.unregister_updaters()

    def _update_style(self):
        self._style_sheet = self._style_creator.get_updated_style_sheet()
        self.setStyleSheet(self._style_sheet)

    def setChecked(self, checked):
        super().setChecked(checked)
        self.setObjectName('Important' if checked else '')
        self.setStyleSheet(self._style_sheet)

    # Protected interface

    def _get_update_signal_types(self):
        raise NotImplementedError

    def _update_enabled(self):
        raise NotImplementedError

    def _change_enabled(self):
        raise NotImplementedError


class HitEditingToggle(EditingToggle):

    def __init__(self):
        super().__init__('Edit hit:')

    def _get_update_signal_types(self):
        return set([
            _get_au_conns_edit_signal_type(self._au_id),
            _get_au_hit_signal_type(self._au_id)])

    def _update_enabled(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        cur_mode = au.get_connections_edit_mode()

        for i in range(HITS_MAX):
            hit = au.get_hit(i)
            if hit.get_existence():
                allow_toggle = True
                break
        else:
            if cur_mode == 'hit_proc_filter':
                au.set_connections_edit_mode('normal')
            allow_toggle = False

        old_block = self.blockSignals(True)
        self.setEnabled(allow_toggle)
        self.setChecked((cur_mode == 'hit_proc_filter') and allow_toggle)
        self.blockSignals(old_block)

    def _change_enabled(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        mode = 'hit_proc_filter' if self.isChecked() else 'normal'
        au.set_connections_edit_mode(mode)

        self._updater.signal_update(_get_au_conns_edit_signal_type(self._au_id))


class ExpressionEditingToggle(EditingToggle):

    def __init__(self):
        super().__init__('Edit expression:')

    def _get_update_signal_types(self):
        return set([
            _get_au_conns_edit_signal_type(self._au_id),
            'signal_expr_list_{}'.format(self._au_id)])

    def _update_enabled(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        cur_mode = au.get_connections_edit_mode()
        if len(au.get_expression_names()) > 0:
            allow_toggle = True
        else:
            if cur_mode == 'expr_filter':
                au.set_connections_edit_mode('normal')
            allow_toggle = False

        old_block = self.blockSignals(True)
        self.setEnabled(allow_toggle)
        self.setChecked((cur_mode == 'expr_filter') and allow_toggle)
        self.blockSignals(old_block)

    def _change_enabled(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        mode = 'expr_filter' if self.isChecked() else 'normal'
        au.set_connections_edit_mode(mode)

        self._updater.signal_update(_get_au_conns_edit_signal_type(self._au_id))


class HitSelector(KqtComboBox, AudioUnitUpdater):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.currentIndexChanged.connect(self._change_hit)

        self.register_action(_get_au_hit_signal_type(self._au_id), self._update_hit_list)

        self._update_hit_list()

    def _get_hit_vis_name(self, hit):
        name = hit.get_name()
        name = name if (type(name) == str) else '-'
        return name

    def _update_hit_list(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        old_block = self.blockSignals(True)

        hits = ((i, au.get_hit(i)) for i in range(HITS_MAX))
        vis_names = ((i, self._get_hit_vis_name(hit)) for (i, hit) in hits
                if hit.get_existence())
        self.set_items(('{}: {}'.format(i, name), i) for (i, name) in vis_names)
        self.setEnabled(self.count() > 0)

        if self.isEnabled():
            prev_hit_index = au.get_connections_hit_index()
            prev_list_index = self.findData(prev_hit_index)
            if prev_list_index != -1:
                self.setCurrentIndex(prev_list_index)
                cur_hit_index = prev_hit_index
            else:
                self.setCurrentIndex(0)
                cur_hit_index = self.itemData(0)
            au.set_connections_hit_index(cur_hit_index)
        else:
            au.set_connections_hit_index(None)

        self.blockSignals(old_block)

    def _change_hit(self, item_index):
        hit_index = self.itemData(item_index)

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_connections_hit_index(hit_index)
        self._updater.signal_update('signal_au_conns_hit_{}'.format(self._au_id))


class ExpressionSelector(KqtComboBox, AudioUnitUpdater):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.currentIndexChanged.connect(self._change_expression)

        self.register_action(
                'signal_expr_list_{}'.format(self._au_id), self._update_expression_list)

        self._update_expression_list()

    def _update_expression_list(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        prev_list_index = self.currentIndex()

        expr_names = sorted(au.get_expression_names())

        old_block = self.blockSignals(True)

        self.set_items(name for name in expr_names)
        self.setEnabled(self.count() > 0)

        if expr_names:
            cur_name = au.get_connections_expr_name()
            if cur_name in expr_names:
                self.setCurrentIndex(expr_names.index(cur_name))
            else:
                self.setCurrentIndex(0)
                au.set_connections_expr_name(expr_names[0])

        self.blockSignals(old_block)

    def _change_expression(self, item_index):
        expr_name = str(self.itemText(item_index))

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_connections_expr_name(expr_name)
        self._updater.signal_update('signal_au_conns_expr_{}'.format(self._au_id))


