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


def get_chorus_params(obj):
    module = obj._ui_model.get_module()
    au = module.get_audio_unit(obj._au_id)
    proc = au.get_processor(obj._proc_id)
    chorus_params = proc.get_type_params()
    return chorus_params


class ChorusProc(QWidget):

    @staticmethod
    def get_name():
        return u'Chorus'

    def __init__(self):
        QWidget.__init__(self)

        self._voice_list = VoiceList()

        v = QVBoxLayout()
        v.setSpacing(10)
        v.addWidget(self._voice_list)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def set_au_id(self, au_id):
        self._voice_list.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._voice_list.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._voice_list.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._voice_list.unregister_updaters()


class VoiceListContainer(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.setSizeConstraint(QLayout.SetMinimumSize)
        self.setLayout(v)


class VoiceListArea(QScrollArea):

    def __init__(self):
        QScrollArea.__init__(self)

    def do_width_hack(self):
        self.widget().setFixedWidth(
                self.width() - self.verticalScrollBar().width() - 5)

    def resizeEvent(self, event):
        self.do_width_hack()


class VoiceList(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None
        self._icon_bank = None

        self._area = VoiceListArea()

        self._init_container()

        self._area.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self._area.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Chorus voices'))
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
        self._area.setWidget(VoiceListContainer())
        add_button = QPushButton('Add voice')
        QObject.connect(add_button, SIGNAL('clicked()'), self._add_voice)
        self._area.widget().layout().addWidget(add_button)

    def _disconnect_editors(self):
        layout = self._area.widget().layout()
        for i in xrange(layout.count() - 1):
            editor = layout.itemAt(i).widget()
            editor.unregister_updaters()

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_chorus_voice', self._proc_id))

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_all()

    def _update_all(self):
        chorus_params = get_chorus_params(self)

        voice_count = chorus_params.get_voice_count()

        layout = self._area.widget().layout()

        # Set voice count
        if voice_count < layout.count() - 1:
            self._disconnect_editors()
            self._init_container()
            layout = self._area.widget().layout()

        # Create new voice editors
        for i in xrange(layout.count() - 1, voice_count):
            editor = VoiceEditor(i, self._icon_bank)
            editor.set_au_id(self._au_id)
            editor.set_proc_id(self._proc_id)
            editor.set_ui_model(self._ui_model)
            layout.insertWidget(i, editor)

        max_count_reached = (voice_count >= chorus_params.get_max_voice_count())
        layout.itemAt(layout.count() - 1).widget().setVisible(not max_count_reached)

        self._area.do_width_hack()

    def _add_voice(self):
        chorus_params = get_chorus_params(self)
        chorus_params.add_voice()
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def resizeEvent(self, event):
        self._area.do_width_hack()


class RemoveButton(QPushButton):

    def __init__(self, icon):
        QPushButton.__init__(self)
        self.setStyleSheet('padding: 0 -2px;')
        self.setIcon(QIcon(icon))


class VoiceEditor(QWidget):

    def __init__(self, index, icon_bank):
        QWidget.__init__(self)
        self._index = index
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._delay_slider = DelaySlider(index)
        self._range_slider = RangeSlider(index)
        self._speed_slider = SpeedSlider(index)
        self._volume_slider = VolumeSlider(index)
        self._remove_button = RemoveButton(icon_bank.get_icon_path('delete_small'))

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(self._delay_slider)
        h.addWidget(self._range_slider)
        h.addWidget(self._speed_slider)
        h.addWidget(self._volume_slider)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._delay_slider.set_au_id(au_id)
        self._range_slider.set_au_id(au_id)
        self._speed_slider.set_au_id(au_id)
        self._volume_slider.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._delay_slider.set_proc_id(proc_id)
        self._range_slider.set_proc_id(proc_id)
        self._speed_slider.set_proc_id(proc_id)
        self._volume_slider.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._delay_slider.set_ui_model(ui_model)
        self._range_slider.set_ui_model(ui_model)
        self._speed_slider.set_ui_model(ui_model)
        self._volume_slider.set_ui_model(ui_model)

        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove)

    def unregister_updaters(self):
        self._volume_slider.unregister_updaters()
        self._speed_slider.unregister_updaters()
        self._range_slider.unregister_updaters()
        self._delay_slider.unregister_updaters()

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_chorus_voice', self._proc_id))

    def _remove(self):
        chorus_params = get_chorus_params(self)
        chorus_params.remove_voice(self._index)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class VoiceParamSlider(ProcNumSlider):

    def __init__(self, index, decimals, min_val, max_val, title):
        ProcNumSlider.__init__(self, decimals, min_val, max_val, title)
        self._index = index

    def _update_value(self):
        chorus_params = get_chorus_params(self)

        if self._index >= chorus_params.get_voice_count():
            # We have been removed
            return

        self.set_number(self._get_value(chorus_params))

    def _value_changed(self, value):
        chorus_params = get_chorus_params(self)
        self._set_value(chorus_params, value)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_chorus_voice', self._proc_id))

    # Protected interface

    def _get_value(self, chorus_params):
        raise NotImplementedError

    def _set_value(self, chorus_params, value):
        raise NotImplementedError


class DelaySlider(VoiceParamSlider):

    def __init__(self, index):
        VoiceParamSlider.__init__(self, index, 3, 0.0, 20.0, title='Delay')
        self.set_number(0)

    def _get_value(self, chorus_params):
        return chorus_params.get_voice_delay(self._index) * 1000.0

    def _set_value(self, chorus_params, value):
        chorus_params.set_voice_delay(self._index, value / 1000.0)


class RangeSlider(VoiceParamSlider):

    def __init__(self, index):
        VoiceParamSlider.__init__(self, index, 3, 0, 5.0, title='Range')
        self.set_number(0)

    def _get_value(self, chorus_params):
        return chorus_params.get_voice_range(self._index) * 1000.0

    def _set_value(self, chorus_params, value):
        chorus_params.set_voice_range(self._index, value / 1000.0)


class SpeedSlider(VoiceParamSlider):

    def __init__(self, index):
        VoiceParamSlider.__init__(self, index, 3, 0, 5, title='Osc. speed')

    def _get_value(self, chorus_params):
        return chorus_params.get_voice_speed(self._index)

    def _set_value(self, chorus_params, value):
        chorus_params.set_voice_speed(self._index, value)


class VolumeSlider(VoiceParamSlider):

    def __init__(self, index):
        VoiceParamSlider.__init__(self, index, 1, -64.0, 18.0, title='Volume')

    def _get_value(self, chorus_params):
        return chorus_params.get_voice_volume(self._index)

    def _set_value(self, chorus_params, value):
        chorus_params.set_voice_volume(self._index, value)


