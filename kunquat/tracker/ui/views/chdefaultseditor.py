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


CHANNELS_MAX = 64 # TODO: define in libkunuqat interface


class ChDefaultsEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._ch_defaults_list = ChDefaultsList()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._ch_defaults_list, 1000)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ch_defaults_list.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._ch_defaults_list.unregister_updaters()


class ChDefaultsListContainer(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(3)
        v.setSizeConstraint(QLayout.SetMinimumSize)
        self.setLayout(v)


class ChDefaultsList(QScrollArea):

    def __init__(self):
        QScrollArea.__init__(self)

        self.setWidget(ChDefaultsListContainer())

        for i in xrange(CHANNELS_MAX):
            chd = ChDefaults(i)
            self.widget().layout().addWidget(chd)

        self._do_width_hack()

    def set_ui_model(self, ui_model):
        layout = self.widget().layout()
        for i in xrange(layout.count()):
            chd = layout.itemAt(i).widget()
            chd.set_ui_model(ui_model)

    def unregister_updaters(self):
        layout = self.widget().layout()
        for i in xrange(layout.count()):
            chd = layout.itemAt(i).widget()
            chd.unregister_updaters()

    def _do_width_hack(self):
        self.widget().setFixedWidth(
                self.width() - self.verticalScrollBar().width() - 5)

    def resizeEvent(self, event):
        self._do_width_hack()


class ChDefaults(QWidget):

    def __init__(self, ch_num):
        QWidget.__init__(self)
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

        self._au_selector = QComboBox()
        self._au_selector.setSizePolicy(
                QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(5)
        h.addWidget(num_widget)
        h.addWidget(self._au_selector)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._module = ui_model.get_module()
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_all()

        QObject.connect(
                self._au_selector,
                SIGNAL('currentIndexChanged(int)'),
                self._select_audio_unit)

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

    def _select_audio_unit(self, index):
        control_id = self._control_catalog[index]
        chd = self._module.get_channel_defaults()
        if chd:
            chd.set_default_control_id(self._ch_num, control_id)


