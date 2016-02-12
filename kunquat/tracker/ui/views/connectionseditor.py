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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from connections import Connections
from kunquat.kunquat.limits import *
import processor.proctypeinfo as proctypeinfo


class ConnectionsEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._toolbar = ConnectionsToolBar()
        self._connections = Connections()

        v = QVBoxLayout()
        v.setMargin(0)
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
        QToolBar.__init__(self)
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

        self._hit_edit = HitEditingToggle()
        self._hit_selector = HitSelector()

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

        if self._au_id != None:
            module = self._ui_model.get_module()
            au = module.get_audio_unit(self._au_id)

            if au.is_instrument():
                # Hit editing controls
                self._hit_edit.set_au_id(self._au_id)
                self._hit_edit.set_ui_model(self._ui_model)
                self.addWidget(self._hit_edit)

                self._hit_selector.set_au_id(self._au_id)
                self._hit_selector.set_ui_model(self._ui_model)
                self.addWidget(self._hit_selector)

    def unregister_updaters(self):
        if self._au_id != None:
            module = self._ui_model.get_module()
            au = module.get_audio_unit(self._au_id)

            if au.is_instrument():
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
        proc_type = str(action.data().toString())

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


def _get_au_hit_signal_type(au_id):
    return 'signal_hit_{}'.format(au_id)


def _get_au_conns_edit_signal_type(au_id):
    return 'signal_au_conns_edit_mode_{}'.format(au_id)


class HitEditingToggle(QPushButton):

    def __init__(self):
        QPushButton.__init__(self, 'Edit hit:')
        self._au_id = None
        self._ui_model = None
        self._updater = None

        self.setCheckable(True)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('clicked()'), self._change_enabled)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            _get_au_conns_edit_signal_type(self._au_id),
            _get_au_hit_signal_type(self._au_id)])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

    def _update_enabled(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        cur_mode = au.get_connections_edit_mode()

        old_block = self.blockSignals(True)
        self.setDown(cur_mode == 'hit_proc_filter')
        self.blockSignals(old_block)

        for i in xrange(HITS_MAX):
            hit = au.get_hit(i)
            if hit.get_existence():
                self.setEnabled(True)
                break
        else:
            if cur_mode == 'hit_proc_filter':
                au.set_connections_edit_mode('normal')
            self.setEnabled(False)

    def _change_enabled(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        mode = 'hit_proc_filter' if self.isChecked() else 'normal'
        au.set_connections_edit_mode(mode)

        self._updater.signal_update(set([_get_au_conns_edit_signal_type(self._au_id)]))


class HitSelector(QComboBox):

    def __init__(self):
        QComboBox.__init__(self)

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
        name = name if (type(name) == unicode) else u'-'
        return name

    def _update_hit_list(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)

        prev_list_index = self.currentIndex()

        old_block = self.blockSignals(True)
        self.clear()
        is_enabled = False
        for i in xrange(HITS_MAX):
            hit = au.get_hit(i)
            if hit.get_existence():
                is_enabled = True
                vis_name = self._get_hit_vis_name(hit)
                self.addItem(u'{}: {}'.format(i, vis_name), QVariant(i))
        self.setEnabled(is_enabled)
        self.blockSignals(old_block)

        if is_enabled and prev_list_index == -1:
            self.setCurrentIndex(0)
            cur_hit_index, success = self.itemData(0).toInt()
            assert success
            au.set_connections_hit_index(cur_hit_index)

    def _change_hit(self, item_index):
        hit_index, success = self.itemData(item_index).toInt()
        assert success

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.set_connections_hit_index(hit_index)
        self._updater.signal_update(set(['signal_au_conns_hit_{}'.format(self._au_id)]))


