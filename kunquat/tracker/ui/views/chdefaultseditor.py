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
from .editorlist import EditorList
from .headerline import HeaderLine
from .kqtcombobox import KqtComboBox
from .varnamevalidator import MaybeVarNameValidator


class ChDefaultsEditor(QWidget):

    def __init__(self):
        super().__init__()

        self._ch_defaults_list = ChDefaultsList()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Channel defaults'))
        v.addWidget(self._ch_defaults_list, 1000)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ch_defaults_list.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._ch_defaults_list.unregister_updaters()


class ChDefaultsList(EditorList):

    def __init__(self):
        super().__init__()
        self._ui_model = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self.update_list()

    def unregister_updaters(self):
        self.disconnect_widgets()

    def _get_updated_editor_count(self):
        return CHANNELS_MAX

    def _make_editor_widget(self, index):
        chd = ChDefaults(index)
        chd.set_ui_model(self._ui_model)
        return chd

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()


class ChDefaults(QWidget):

    def __init__(self, ch_num):
        super().__init__()
        self._ch_num = ch_num
        self._ui_model = None
        self._module = None
        self._updater = None

        self._control_catalog = {}

        num_widget = QLabel('{}'.format(self._ch_num))
        num_font = QFont()
        num_font.setWeight(QFont.Bold)
        num_widget.setFont(num_font)
        num_widget.setAlignment(Qt.AlignRight | Qt.AlignVCenter)
        num_widget.setMargin(5)
        fm = QFontMetrics(num_font)
        width = fm.boundingRect('{}'.format(CHANNELS_MAX - 1)).width()
        width += 20
        num_widget.setFixedWidth(width)

        self._au_selector = KqtComboBox()
        self._au_selector.setSizePolicy(
                QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        self._init_expr = QLineEdit()
        self._init_expr.setValidator(MaybeVarNameValidator())

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(5)
        h.addWidget(num_widget)
        h.addWidget(self._au_selector)
        h.addWidget(QLabel('Initial expression:'))
        h.addWidget(self._init_expr)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._module = ui_model.get_module()
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self._au_selector,
                SIGNAL('currentIndexChanged(int)'),
                self._select_audio_unit)

        QObject.connect(
                self._init_expr, SIGNAL('editingFinished()'), self._change_init_expr)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return '_'.join(('signal_ch_defaults', str(self._ch_num)))

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_controls',
            'signal_order_list',
            self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _get_control_text(self, control_id):
        parts = control_id.split('_')
        second = parts[1]
        control_num = int(second, 16)
        control = self._module.get_control(control_id)
        au = control.get_audio_unit()
        au_type = 'Instrument' if au.is_instrument() else 'Effect'
        au_name = au.get_name() or '-'
        text = '{} {}: {}'.format(au_type, control_num, au_name)
        return text

    def _update_all(self):
        chd = self._module.get_channel_defaults()
        default_control_id = chd.get_default_control_id(self._ch_num) if chd else None

        control_ids = self._module.get_control_ids()
        self._control_catalog = dict(enumerate(sorted(control_ids)))

        old_block = self._au_selector.blockSignals(True)
        self._au_selector.clear()
        for i, control_id in self._control_catalog.items():
            self._au_selector.addItem(self._get_control_text(control_id))
            if default_control_id == control_id:
                self._au_selector.setCurrentIndex(i)
        self._au_selector.blockSignals(old_block)

        old_block = self._init_expr.blockSignals(True)
        expr_name = chd.get_initial_expression(self._ch_num)
        if expr_name != self._init_expr.text():
            self._init_expr.setText(expr_name)
        self._init_expr.blockSignals(old_block)

    def _select_audio_unit(self, index):
        control_id = self._control_catalog[index]
        chd = self._module.get_channel_defaults()
        if chd:
            chd.set_default_control_id(self._ch_num, control_id)

    def _change_init_expr(self):
        expr_name = str(self._init_expr.text())
        chd = self._module.get_channel_defaults()
        if chd:
            chd.set_initial_expression(self._ch_num, expr_name)


