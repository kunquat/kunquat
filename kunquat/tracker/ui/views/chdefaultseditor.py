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

        self._song_selector = SongSelector()
        self._ch_defaults_list = ChDefaultsList()

        song_layout = QHBoxLayout()
        song_layout.setMargin(5)
        song_layout.setSpacing(5)
        song_layout.addWidget(QLabel('Song'))
        song_layout.addWidget(self._song_selector)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addLayout(song_layout)
        v.addWidget(self._ch_defaults_list, 1000)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._song_selector.set_ui_model(ui_model)
        self._ch_defaults_list.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._ch_defaults_list.unregister_updaters()
        self._song_selector.unregister_updaters()


class SongSelector(QComboBox):

    def __init__(self):
        QComboBox.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_song()

        QObject.connect(self, SIGNAL('currentIndexChanged(int)'), self._select_track)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set(['signal_order_list', 'signal_ch_defaults_song'])
        if not signals.isdisjoint(update_signals):
            self._update_song()

    def _update_song(self):
        module = self._ui_model.get_module()
        album = module.get_album()

        selected_track_num = album.get_selected_track_num()

        songs = [album.get_song_by_track(i) for i in xrange(album.get_track_count())]

        old_block = self.blockSignals(True)
        self.clear()
        for i, song in enumerate(songs):
            display_name = song.get_name() or 'Song {}'.format(song.get_number())
            self.addItem(display_name)
            if selected_track_num == i:
                self.setCurrentIndex(i)
        self.blockSignals(old_block)

    def _select_track(self, track_num):
        module = self._ui_model.get_module()
        album = module.get_album()
        album.set_selected_track_num(track_num)
        self._updater.signal_update(set(['signal_ch_defaults_song']))


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

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return '_'.join(('signal_ch_defaults', str(self._ch_num)))

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_controls',
            'signal_order_list',
            'signal_ch_defaults_song',
            self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _get_control_text(self, control_id):
        parts = control_id.split('_')
        second = parts[1]
        control_num = int(second, 16)
        control = self._module.get_control(control_id)
        au = control.get_audio_unit()
        au_name = au.get_name() or '-'
        text = 'Audio unit {}: {}'.format(control_num, au_name)
        return text

    def _update_all(self):
        album = self._module.get_album()

        track_num = album.get_selected_track_num()
        if track_num >= 0:
            song = album.get_song_by_track(track_num)
            chd = song.get_channel_defaults()
            default_control_id = chd.get_default_control_id(self._ch_num)
        else:
            default_control_id = None

        control_ids = self._module.get_control_ids()
        control_catalog = dict(enumerate(sorted(control_ids)))

        old_block = self._au_selector.blockSignals(True)
        self._au_selector.clear()
        for i, control_id in control_catalog.items():
            self._au_selector.addItem(self._get_control_text(control_id))
            if default_control_id == control_id:
                self._au_selector.setCurrentIndex(i)
        self._au_selector.blockSignals(old_block)


