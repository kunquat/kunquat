# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2018
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
from .editorlist import EditorList
from .headerline import HeaderLine
from .kqtcombobox import KqtComboBox
from .updater import Updater
from .varnamevalidator import MaybeVarNameValidator


class ChDefaultsEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()

        self._ch_defaults_list = ChDefaultsList()
        self.add_to_updaters(self._ch_defaults_list)

        self._header = HeaderLine('Channel defaults')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header)
        v.addWidget(self._ch_defaults_list, 1000)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._header.update_style(style_mgr)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))


class ChDefaultsList(EditorList, Updater):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.update_list()

    def _on_teardown(self):
        self.disconnect_widgets()

    def _get_updated_editor_count(self):
        return CHANNELS_MAX

    def _make_editor_widget(self, index):
        chd = ChDefaults(index)
        self.add_to_updaters(chd)
        return chd

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        self.remove_from_updaters(widget)


class ChDefaults(QWidget, Updater):

    def __init__(self, ch_num):
        super().__init__()
        self._ch_num = ch_num
        self._module = None

        self._control_catalogue = {}

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

    def _on_setup(self):
        self.register_action('signal_controls', self._update_all)
        self.register_action('signal_order_list', self._update_all)
        self.register_action(self._get_update_signal_type(), self._update_all)
        self.register_action('signal_style_changed', self._update_style)

        self._module = self._ui_model.get_module()

        self._au_selector.currentIndexChanged.connect(self._select_audio_unit)
        self._init_expr.editingFinished.connect(self._change_init_expr)

        self._update_style()
        self._update_all()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

    def _get_update_signal_type(self):
        return '_'.join(('signal_ch_defaults', str(self._ch_num)))

    def _get_control_text(self, control_id):
        parts = control_id.split('_')
        second = parts[1]
        control_num = int(second, 16)
        control = self._module.get_control(control_id)
        au = control.get_audio_unit()
        if not au.get_existence():
            return '-'
        au_type = 'Instrument' if au.is_instrument() else 'Effect'
        au_name = au.get_name() or '-'
        text = '{} {}: {}'.format(au_type, control_num, au_name)
        return text

    def _update_all(self):
        chd = self._module.get_channel_defaults()
        default_control_id = chd.get_default_control_id(self._ch_num) if chd else None

        control_ids = sorted(self._module.get_control_ids())
        try:
            default_index = control_ids.index(default_control_id)
        except ValueError:
            default_index = -1

        old_block = self._au_selector.blockSignals(True)
        self._au_selector.set_items(
                self._get_control_text(control_id) for control_id in control_ids)
        self._au_selector.setCurrentIndex(default_index)
        self._au_selector.blockSignals(old_block)

        self._control_catalogue = dict(enumerate(control_ids))

        old_block = self._init_expr.blockSignals(True)
        expr_name = chd.get_initial_expression(self._ch_num)
        if expr_name != self._init_expr.text():
            self._init_expr.setText(expr_name)
        self._init_expr.blockSignals(old_block)

    def _select_audio_unit(self, index):
        control_id = self._control_catalogue[index]
        chd = self._module.get_channel_defaults()
        if chd:
            chd.set_default_control_id(self._ch_num, control_id)
            self._updater.signal_update('signal_ch_defaults')

    def _change_init_expr(self):
        expr_name = str(self._init_expr.text())
        chd = self._module.get_channel_defaults()
        if chd:
            chd.set_initial_expression(self._ch_num, expr_name)


