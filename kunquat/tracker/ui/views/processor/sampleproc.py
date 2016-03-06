# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from kunquat.tracker.ui.views.axisrenderer import HorizontalAxisRenderer, VerticalAxisRenderer
from kunquat.tracker.ui.views.keyboardmapper import KeyboardMapper
from kunquat.tracker.ui.views.utils import lerp_val
import utils


class SampleProc(QTabWidget):

    @staticmethod
    def get_name():
        return u'Sample synthesis'

    def __init__(self):
        QTabWidget.__init__(self)

        self._note_map_editor = NoteMapEditor()
        self._samples = Samples()

        self.addTab(self._note_map_editor, 'Note map')
        self.addTab(self._samples, 'Samples')

    def set_au_id(self, au_id):
        self._note_map_editor.set_au_id(au_id)
        self._samples.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._note_map_editor.set_proc_id(proc_id)
        self._samples.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._note_map_editor.set_ui_model(ui_model)
        self._samples.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._samples.unregister_updaters()
        self._note_map_editor.unregister_updaters()


class NoteMapEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._note_map = NoteMap()
        self._note_map_entry = NoteMapEntry()

        h = QHBoxLayout()
        h.setMargin(2)
        h.setSpacing(4)
        h.addWidget(self._note_map)
        h.addWidget(self._note_map_entry)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._note_map.set_au_id(au_id)
        self._note_map_entry.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._note_map.set_proc_id(proc_id)
        self._note_map_entry.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._note_map.set_ui_model(ui_model)
        self._note_map_entry.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._note_map_entry.unregister_updaters()
        self._note_map.unregister_updaters()


class NoteMap(QWidget):

    _DEFAULT_CONFIG = {
        'padding'                  : 5,
        'bg_colour'                : QColor(0, 0, 0),
        'point_colour'             : QColor(0xee, 0xcc, 0xaa),
        'focused_point_colour'     : QColor(0xff, 0x77, 0x22),
        'focused_point_axis_colour': QColor(0xff, 0x77, 0x22, 0x7f),
        'point_size'               : 7,
    }

    _FONT = QFont(QFont().defaultFamily(), 9)
    _FONT.setWeight(QFont.Bold)

    _AXIS_CONFIG = {
        'axis_x': {
            'height'          : 20,
            'marker_min_dist' : 6,
            'marker_min_width': 2,
            'marker_max_width': 5,
            'label_min_dist'  : 30,
            'draw_zero_label' : True,
        },
        'axis_y': {
            'width'           : 50,
            'marker_min_dist' : 6,
            'marker_min_width': 2,
            'marker_max_width': 5,
            'label_min_dist'  : 50,
            'draw_zero_label' : True,
        },
        'label_font'  : _FONT,
        'label_colour': QColor(0xcc, 0xcc, 0xcc),
        'line_colour' : QColor(0xcc, 0xcc, 0xcc),
    }

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._axis_x_renderer = HorizontalAxisRenderer()
        self._axis_x_renderer.set_config(self._AXIS_CONFIG, self)
        self._axis_x_renderer.set_val_range([-48, 0])

        self._axis_y_renderer = VerticalAxisRenderer()
        self._axis_y_renderer.set_config(self._AXIS_CONFIG, self)
        self._axis_y_renderer.set_val_range([-7200, 7200])

        self._config = None
        self._set_config({})

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self.setMouseTracking(True)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _set_config(self, config):
        self._config = self._DEFAULT_CONFIG.copy()
        self._config.update(config)

    def _perform_updates(self, signals):
        pass

    def _get_vis_coords(self, point):
        cents, dB = point

        def map_range(val, src_range, target_range):
            start_diff = val - src_range[0]
            pos_norm = (val - src_range[0]) / float(src_range[1] - src_range[0])
            return lerp_val(target_range[0], target_range[1], min(max(0, pos_norm), 1))

        cents_range = self._axis_y_renderer.get_val_range()
        y_range = self._axis_y_renderer.get_axis_length() - 1, 0
        area_y = map_range(cents, cents_range, y_range)

        dB_range = self._axis_x_renderer.get_val_range()
        x_range = 0, self._axis_x_renderer.get_axis_length() - 1
        area_x = map_range(dB, dB_range, x_range)

        padding = self._config['padding']
        x = area_x + padding + self._axis_y_renderer.get_width() - 1
        y = area_y + padding

        return x, y

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        padding = self._config['padding']

        # Axes
        axis_x_height = self._AXIS_CONFIG['axis_x']['height']
        axis_x_top = self.height() - padding - axis_x_height
        axis_y_width = self._AXIS_CONFIG['axis_y']['width']
        axis_x_length = self.width() - axis_y_width - (padding * 2) + 1
        axis_y_length = self.height() - axis_x_height - (padding * 2) + 1

        painter.setTransform(
                QTransform().translate(padding, axis_x_top))
        self._axis_x_renderer.set_width(self.width())
        self._axis_x_renderer.set_axis_length(axis_x_length)
        self._axis_x_renderer.set_x_offset(axis_y_width - 1)
        self._axis_x_renderer.set_y_offset_x(self.width() - axis_y_width - padding * 2)
        self._axis_x_renderer.render(painter)

        painter.setTransform(QTransform().translate(padding, padding))
        self._axis_y_renderer.set_height(self.height())
        self._axis_y_renderer.set_axis_length(axis_y_length)
        self._axis_y_renderer.set_x_offset_y(
                (self.height() - axis_x_height - padding * 2) // 2)
        self._axis_y_renderer.render(painter)

        # Note map points
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setPen(Qt.NoPen)
        painter.setTransform(QTransform())
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        point_size = self._config['point_size']
        point_offset = -(point_size // 2)
        for point in sample_params.get_note_map_points():
            x, y = self._get_vis_coords(point)
            painter.setBrush(QColor(self._config['point_colour']))
            rect = QRect(x + point_offset, y + point_offset, point_size, point_size)
            painter.drawEllipse(rect)

        end = time.time()
        elapsed = end - start
        #print('Note map view updated in {:.2f} ms'.format(elapsed * 1000))


class NoteMapEntry(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        pass


class Samples(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._sample_list = SampleList()
        self._sample_editor = SampleEditor()

        h = QHBoxLayout()
        h.setMargin(2)
        h.setSpacing(4)
        h.addWidget(self._sample_list)
        h.addWidget(self._sample_editor)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._sample_list.set_au_id(au_id)
        self._sample_editor.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._sample_list.set_proc_id(proc_id)
        self._sample_editor.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._sample_list.set_ui_model(ui_model)
        self._sample_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._sample_editor.unregister_updaters()
        self._sample_list.unregister_updaters()


class SampleListToolBar(QToolBar):

    def __init__(self):
        QToolBar.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._load_button = QToolButton()
        self._load_button.setText('Load sample')
        self._load_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove sample')
        self._remove_button.setEnabled(False)

        self.addWidget(self._load_button)
        self.addWidget(self._remove_button)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_enabled()

    def _update_enabled(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        id_count = len(sample_params.get_sample_ids())
        self._load_button.setEnabled(id_count < sample_params.get_max_sample_count())


class SampleListModel(QAbstractListModel):

    def __init__(self):
        QAbstractListModel.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._items = []

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._make_items()

    def unregister_updaters(self):
        pass

    def get_item(self, index):
        row = index.row()
        if 0 <= row < len(self._items):
            item = self._items[row]
            return item
        return None

    def _make_items(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        for sample_id in sample_params.get_sample_ids():
            name = sample_params.get_sample_name(sample_id)
            self._items.append((sample_id, name))
        self._items = sorted(self._items, key=lambda x: x[1])

    # Qt interface

    def rowCount(self, parent):
        return len(self._items)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row = index.row()
            if 0 <= row < len(self._items):
                _, vis_name = self._items[row]
                if vis_name == None:
                    vis_name = '-'
                return QVariant(vis_name)

        return QVariant()

    def headerData(self, section, orientation, role):
        return QVariant()


class SampleListView(QListView):

    def __init__(self):
        QListView.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._keyboard_mapper = KeyboardMapper()

        self.setSelectionMode(QAbstractItemView.SingleSelection)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        self._keyboard_mapper.set_ui_model(ui_model)

        for signal_type in ('clicked', 'activated'):
            signal = '{}(const QModelIndex&)'.format(signal_type)
            QObject.connect(self, SIGNAL(signal), self._select_sample)

    def unregister_updaters(self):
        self._keyboard_mapper.unregister_updaters()

    def _get_update_signal_type(self):
        return 'signal_proc_select_sample_{}'.format(self._proc_id)

    def _select_sample(self, index):
        item = self.model().get_item(index)
        if item:
            sample_id, _ = item
            sample_params = utils.get_proc_params(
                    self._ui_model, self._au_id, self._proc_id)
            sample_params.set_selected_sample_id(sample_id)
            self._updater.signal_update(set([self._get_update_signal_type()]))

    def keyPressEvent(self, event):
        if self._keyboard_mapper.is_handled_key(event):
            event.ignore()
        else:
            QListView.keyPressEvent(self, event)

    def keyReleaseEvent(self, event):
        if self._keyboard_mapper.is_handled_key(event):
            event.ignore()
        else:
            QListView.keyReleaseEvent(self, event)


class SampleList(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._toolbar = SampleListToolBar()

        self._list_model = None
        self._list_view = SampleListView()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._list_view)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._toolbar.set_au_id(au_id)
        self._list_view.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._toolbar.set_proc_id(proc_id)
        self._list_view.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._toolbar.set_ui_model(ui_model)
        self._list_view.set_ui_model(ui_model)
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_model()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._list_view.unregister_updaters()
        self._toolbar.unregister_updaters()

    def _get_update_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_model()

    def _update_model(self):
        self._list_model = SampleListModel()
        self._list_model.set_au_id(self._au_id)
        self._list_model.set_proc_id(self._proc_id)
        self._list_model.set_ui_model(self._ui_model)
        self._list_view.setModel(self._list_model)


class SampleEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._name_editor = QLineEdit()

        gl = QGridLayout()
        gl.setMargin(0)
        gl.setSpacing(2)
        gl.addWidget(QLabel('Name:'), 0, 0)
        gl.addWidget(self._name_editor, 0, 1)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addLayout(gl)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self._name_editor, SIGNAL('editingFinished()'), self._change_name)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_list_update_signal_type(self):
        return 'signal_proc_sample_list_{}'.format(self._proc_id)

    def _get_selection_update_signal_type(self):
        return 'signal_proc_select_sample_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        update_signals = set([
            self._get_list_update_signal_type(),
            self._get_selection_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        sample_id = sample_params.get_selected_sample_id()
        has_sample = sample_id in sample_params.get_sample_ids()
        self.setEnabled(has_sample)

        name = u''
        if has_sample:
            name = sample_params.get_sample_name(sample_id)

        old_block = self._name_editor.blockSignals(True)
        if self._name_editor.text() != name:
            self._name_editor.setText(name)
        self._name_editor.blockSignals(old_block)

    def _change_name(self):
        sample_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        sample_id = sample_params.get_selected_sample_id()
        sample_params.set_sample_name(sample_id, unicode(self._name_editor.text()))
        self._updater.signal_update(set([self._get_list_update_signal_type()]))


