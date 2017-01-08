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

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.kunquat.limits import *
from .connections import Connections
from .processor import proctypeinfo
from .kqtcombobox import KqtComboBox
from .kqtutils import get_kqt_file_path, open_kqt_au
from .saving import get_instrument_save_path, get_effect_save_path
from .stylecreator import StyleCreator


class ConnectionsEditor(QWidget):

    def __init__(self):
        super().__init__()

        self._toolbar = ConnectionsToolBar()
        self._connections = Connections()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._connections)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._toolbar.set_au_id(au_id)
        self._connections.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._toolbar.set_ui_model(ui_model)
        self._connections.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._connections.unregister_updaters()
        self._toolbar.unregister_updaters()


class ConnectionsToolBar(QToolBar):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._ui_model = None
        self._updater = None

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

        self._hit_edit = HitEditingToggle()
        self._hit_selector = HitSelector()

        self._expr_edit = ExpressionEditingToggle()
        self._expr_selector = ExpressionSelector()

        self._export_button = QToolButton()
        self._export_button.setText('Export')

    def set_au_id(self, au_id):
        assert self._ui_model == None, "Audio unit ID must be set before UI model"
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        # Instrument or processor adder
        if self._au_id == None:
            self.addWidget(self._add_ins_button)
            QObject.connect(
                    self._add_ins_button,
                    SIGNAL('clicked()'),
                    self._add_instrument)
        else:
            self.addWidget(self._add_proc_button)
            QObject.connect(
                    self._add_proc_button.menu(),
                    SIGNAL('triggered(QAction*)'),
                    self._add_processor)

        # Effect adder if allowed
        is_effect_allowed = (self._au_id == None)
        if self._au_id != None:
            module = self._ui_model.get_module()
            au = module.get_audio_unit(self._au_id)
            is_effect_allowed = au.is_instrument()
        if is_effect_allowed:
            self.addWidget(self._add_effect_button)
            QObject.connect(
                    self._add_effect_button,
                    SIGNAL('clicked()'),
                    self._add_effect)

        # Import button if allowed
        is_import_allowed = is_effect_allowed
        if is_import_allowed:
            if self._au_id == None:
                text = 'Import instrument/effect'
            else:
                text = 'Import effect'
            self._import_button.setText(text)
            self.addWidget(self._import_button)
            QObject.connect(
                    self._import_button,
                    SIGNAL('clicked()'),
                    self._import_au)

        if self._au_id != None:
            module = self._ui_model.get_module()
            au = module.get_audio_unit(self._au_id)

            if au.is_instrument():
                # Hit editing controls
                self.addSeparator()

                self._hit_edit.set_au_id(self._au_id)
                self._hit_edit.set_ui_model(self._ui_model)
                self.addWidget(self._hit_edit)

                self._hit_selector.set_au_id(self._au_id)
                self._hit_selector.set_ui_model(self._ui_model)
                self.addWidget(self._hit_selector)

                # Expression controls
                self.addSeparator()

                self._expr_edit.set_au_id(self._au_id)
                self._expr_edit.set_ui_model(self._ui_model)
                self.addWidget(self._expr_edit)

                self._expr_selector.set_au_id(self._au_id)
                self._expr_selector.set_ui_model(self._ui_model)
                self.addWidget(self._expr_selector)

            # Export
            self.addWidget(self._export_button)
            QObject.connect(self._export_button, SIGNAL('clicked()'), self._export_au)

    def unregister_updaters(self):
        if self._au_id != None:
            module = self._ui_model.get_module()
            au = module.get_audio_unit(self._au_id)

            if au.is_instrument():
                self._expr_selector.unregister_updaters()
                self._expr_edit.unregister_updaters()
                self._hit_selector.unregister_updaters()
                self._hit_edit.unregister_updaters()

    def _add_instrument(self):
        module = self._ui_model.get_module()
        new_control_id = module.get_free_control_id()
        new_au_id = module.get_free_au_id()
        if new_control_id and new_au_id:
            module.add_instrument(new_au_id)
            module.add_control(new_control_id)
            control = module.get_control(new_control_id)
            control.connect_to_au(new_au_id)
            update_signals = set(['signal_connections', 'signal_controls'])
            self._updater.signal_update(update_signals)

    def _add_processor(self, action):
        assert action != None
        proc_type = action.data()

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        new_proc_id = au.get_free_processor_id()
        if new_proc_id != None:
            au.add_processor(new_proc_id, proc_type)
            update_signal = '_'.join(('signal_connections', self._au_id))
            self._updater.signal_update(set([update_signal]))

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
            update_signals = set()
            if is_control_needed:
                parent_device.add_control(new_control_id)
                control = parent_device.get_control(new_control_id)
                control.connect_to_au(new_au_id)
                update_signals.add('signal_controls')

            update_signal = 'signal_connections'
            if self._au_id != None:
                update_signal = '_'.join((update_signal, self._au_id))
            update_signals.add(update_signal)

            self._updater.signal_update(update_signals)

    def _import_au(self):
        module = self._ui_model.get_module()
        if self._au_id == None:
            au_path = get_kqt_file_path(set(['kqti', 'kqte']))
        else:
            au_path = get_kqt_file_path(set(['kqte']))
        if au_path:
            container = module
            if self._au_id != None:
                container = module.get_audio_unit(self._au_id)
            open_kqt_au(au_path, self._ui_model, container)

    def _export_au(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        if au.is_instrument():
            au_path = get_instrument_save_path(au.get_name())
        else:
            au_path = get_effect_save_path(au.get_name())
        if not au_path:
            return

        au.start_export_au(au_path)


def _get_au_hit_signal_type(au_id):
    return 'signal_hit_{}'.format(au_id)


def _get_au_conns_edit_signal_type(au_id):
    return 'signal_au_conns_edit_mode_{}'.format(au_id)


class EditingToggle(QPushButton):

    def __init__(self, text):
        super().__init__(text)
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self._style_creator = StyleCreator()
        self._style_sheet = ''

        self.setCheckable(True)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('clicked()'), self._change_enabled)

        self._style_creator.set_ui_model(ui_model)

        self._style_sheet = QApplication.instance().styleSheet()
        self._update_enabled()

    def unregister_updaters(self):
        self._style_creator.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = self._get_update_signal_types()
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

        if 'signal_style_changed' in signals:
            self._update_style()

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

        self._updater.signal_update(set([_get_au_conns_edit_signal_type(self._au_id)]))


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

        self._updater.signal_update(set([_get_au_conns_edit_signal_type(self._au_id)]))


class HitSelector(KqtComboBox):

    def __init__(self):
        super().__init__()

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('currentIndexChanged(int)'), self._change_hit)

        self._update_hit_list()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if _get_au_hit_signal_type(self._au_id) in signals:
            self._update_hit_list()

    def _get_hit_vis_name(self, hit):
        name = hit.get_name()
        name = name if (type(name) == str) else '-'
        return name

    def _update_hit_list(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        prev_list_index = self.currentIndex()

        old_block = self.blockSignals(True)

        hits = ((i, au.get_hit(i)) for i in range(HITS_MAX))
        vis_names = ((i, self._get_hit_vis_name(hit)) for (i, hit) in hits
                if hit.get_existence())
        self.set_items(('{}: {}'.format(i, name), i) for (i, name) in vis_names)
        self.setEnabled(self.count() > 0)

        if self.isEnabled() and (prev_list_index == -1):
            self.setCurrentIndex(0)
            cur_hit_index = self.itemData(0)
            au.set_connections_hit_index(cur_hit_index)

        self.blockSignals(old_block)

    def _change_hit(self, item_index):
        hit_index = self.itemData(item_index)

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_connections_hit_index(hit_index)
        self._updater.signal_update(set(['signal_au_conns_hit_{}'.format(self._au_id)]))


class ExpressionSelector(KqtComboBox):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._ui_model = None

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

        prev_list_index = self.currentIndex()

        expr_names = sorted(au.get_expression_names())

        old_block = self.blockSignals(True)

        self.set_items(name for name in expr_names)
        self.setEnabled(self.count() > 0)

        if expr_names and (prev_list_index == -1):
            self.setCurrentIndex(0)
            au.set_connections_expr_name(expr_names[0])

        self.blockSignals(old_block)

    def _change_expression(self, item_index):
        expr_name = str(self.itemText(item_index))

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_connections_expr_name(expr_name)
        self._updater.signal_update(set(['signal_au_conns_expr_{}'.format(self._au_id)]))


