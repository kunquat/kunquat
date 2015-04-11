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

from procnumslider import ProcNumSlider
from kunquat.tracker.ui.views.headerline import HeaderLine


def get_delay_params(obj):
    module = obj._ui_model.get_module()
    au = module.get_audio_unit(obj._au_id)
    proc = au.get_processor(obj._proc_id)
    type_params = proc.get_type_params()
    return type_params


class DelayProc(QWidget):

    @staticmethod
    def get_name():
        return u'Tap delay'

    def __init__(self):
        QWidget.__init__(self)

        self._max_delay = MaxDelaySpin()
        self._tap_list = TapList()

        v = QVBoxLayout()
        v.addWidget(self._max_delay)
        v.addWidget(self._tap_list)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._max_delay.set_au_id(au_id)
        self._tap_list.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._max_delay.set_proc_id(proc_id)
        self._tap_list.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._max_delay.set_ui_model(ui_model)
        self._tap_list.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._tap_list.unregister_updaters()
        self._max_delay.unregister_updaters()


class MaxDelaySpin(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._spin = QDoubleSpinBox()
        self._spin.setDecimals(4)
        self._spin.setMinimum(0.001)
        self._spin.setMaximum(60)
        self._spin.setValue(1)

        h = QHBoxLayout()
        h.setMargin(0)
        h.addWidget(QLabel('Maximum delay'))
        h.addWidget(self._spin)
        h.addStretch()
        self.setLayout(h)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Maximum)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_value()
        QObject.connect(self._spin, SIGNAL('valueChanged(double)'), self._value_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_delay', self._proc_id))

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_value()

    def _update_value(self):
        delay_params = get_delay_params(self)

        old_block = self._spin.blockSignals(True)
        new_delay = delay_params.get_max_delay()
        if new_delay != self._spin.value():
            self._spin.setValue(new_delay)
        self._spin.blockSignals(old_block)

    def _value_changed(self, value):
        delay_params = get_delay_params(self)
        delay_params.set_max_delay(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class TapListContainer(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.setSizeConstraint(QLayout.SetMinimumSize)
        self.setLayout(v)


class TapListArea(QScrollArea):

    def __init__(self):
        QScrollArea.__init__(self)

    def do_width_hack(self):
        self.widget().setFixedWidth(
                self.width() - self.verticalScrollBar().width() - 5)

    def resizeEvent(self, event):
        self.do_width_hack()


class TapList(QWidget):

    def __init__(self):
        QScrollArea.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None
        self._icon_bank = None

        self._area = TapListArea()

        self._init_container()

        self._area.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self._area.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        #self._area.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Delay taps'))
        v.addWidget(self._area)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._icon_bank = ui_model.get_icon_bank()
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_all()

    def unregister_updaters(self):
        self._disconnect_editors()
        self._updater.unregister_updater(self._perform_updates)

    def _init_container(self):
        self._area.setWidget(TapListContainer())
        add_button = QPushButton('Add tap')
        QObject.connect(add_button, SIGNAL('clicked()'), self._tap_added)
        self._area.widget().layout().addWidget(add_button)

    def _disconnect_editors(self):
        layout = self._area.widget().layout()
        for i in xrange(layout.count() - 1):
            editor = layout.itemAt(i).widget()
            editor.unregister_updaters()

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_delay', self._proc_id))

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_all()

    def _update_all(self):
        delay_params = get_delay_params(self)

        tap_count = delay_params.get_tap_count()

        layout = self._area.widget().layout()

        if tap_count < layout.count() - 1:
            # Create contents from scratch because
            # Qt doesn't update visuals properly on single item removal
            self._disconnect_editors()
            self._init_container()
            layout = self._area.widget().layout()

        # Create new items
        for i in xrange(layout.count() - 1, tap_count):
            editor = TapEditor(i, self._icon_bank)
            editor.set_au_id(self._au_id)
            editor.set_proc_id(self._proc_id)
            editor.set_ui_model(self._ui_model)
            layout.insertWidget(i, editor)

        max_count_reached = (tap_count >= delay_params.get_max_tap_count())
        layout.itemAt(layout.count() - 1).widget().setVisible(not max_count_reached)

        self._area.do_width_hack()

    def _tap_added(self):
        delay_params = get_delay_params(self)
        delay_params.add_tap()
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def resizeEvent(self, event):
        self._area.do_width_hack()


class RemoveButton(QPushButton):

    def __init__(self, icon):
        QPushButton.__init__(self)
        self.setStyleSheet('padding: 0 -2px;')
        self.setIcon(QIcon(icon))


class TapEditor(QWidget):

    def __init__(self, tap_index, icon_bank):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None
        self._tap_index = tap_index

        self._delay_slider = DelaySlider(tap_index)
        self._volume_slider = VolumeSlider(tap_index)
        self._remove_button = RemoveButton(icon_bank.get_icon_path('delete_small'))

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(self._delay_slider)
        h.addWidget(self._volume_slider)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._delay_slider.set_au_id(au_id)
        self._volume_slider.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._delay_slider.set_proc_id(proc_id)
        self._volume_slider.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._delay_slider.set_ui_model(ui_model)
        self._volume_slider.set_ui_model(ui_model)
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove)

    def unregister_updaters(self):
        self._volume_slider.unregister_updaters()
        self._delay_slider.unregister_updaters()

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_delay', self._proc_id))

    def _remove(self):
        delay_params = get_delay_params(self)
        delay_params.remove_tap(self._tap_index)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class DelaySlider(ProcNumSlider):

    def __init__(self, tap_index):
        ProcNumSlider.__init__(self, 4, 0, 60, title='Delay')
        self._tap_index = tap_index
        self.set_number(0)

    def _update_value(self):
        delay_params = get_delay_params(self)

        if self._tap_index >= delay_params.get_tap_count():
            # We have been removed
            return

        self.set_range(0, delay_params.get_max_delay())
        self.set_number(delay_params.get_tap_delay(self._tap_index))

    def _value_changed(self, value):
        delay_params = get_delay_params(self)
        delay_params.set_tap_delay(self._tap_index, value)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_delay', self._proc_id))


class VolumeSlider(ProcNumSlider):

    def __init__(self, tap_index):
        ProcNumSlider.__init__(self, 1, -64.0, 18.0, title='Volume')
        self._tap_index = tap_index
        self.set_number(0)

    def _update_value(self):
        delay_params = get_delay_params(self)

        if self._tap_index >= delay_params.get_tap_count():
            # We have been removed
            return

        self.set_number(delay_params.get_tap_volume(self._tap_index))

    def _value_changed(self, value):
        delay_params = get_delay_params(self)
        delay_params.set_tap_volume(self._tap_index, value)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_delay', self._proc_id))


