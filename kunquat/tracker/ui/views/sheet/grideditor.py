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

import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from config import *
from ruler import Ruler
import kunquat.tracker.ui.model.tstamp as tstamp
from kunquat.tracker.ui.model.gridpatterns import STYLE_COUNT
from kunquat.tracker.ui.views.headerline import HeaderLine
from kunquat.tracker.ui.views.numberslider import NumberSlider
import utils


class GridEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._grid_list = GridList()
        self._general_editor = GeneralEditor()

        self._grid_area = GridArea()
        self._subdiv_editor = SubdivEditor()
        self._line_editor = LineEditor()

        r = QVBoxLayout()
        r.setMargin(0)
        r.setSpacing(8)
        r.addWidget(self._subdiv_editor)
        r.addWidget(self._line_editor)

        el = QHBoxLayout()
        el.setMargin(0)
        el.setSpacing(4)
        el.addWidget(self._grid_area)
        el.addLayout(r)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(self._grid_list, 1)
        v.addSpacing(2)
        v.addWidget(HeaderLine('Grid editor'))
        v.addWidget(self._general_editor)
        v.addLayout(el, 4)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._grid_list.set_ui_model(ui_model)
        self._general_editor.set_ui_model(ui_model)
        self._grid_area.set_ui_model(ui_model)
        self._subdiv_editor.set_ui_model(ui_model)
        self._line_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._line_editor.unregister_updaters()
        self._subdiv_editor.unregister_updaters()
        self._grid_area.unregister_updaters()
        self._general_editor.unregister_updaters()
        self._grid_list.unregister_updaters()


class GridListModel(QAbstractListModel):

    def __init__(self):
        QAbstractItemModel.__init__(self)
        self._ui_model = None
        self._updater = None

        self._items = []

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._make_items()

    def get_item(self, index):
        row = index.row()
        if 0 <= row < len(self._items):
            item = self._items[row]
            return item
        return None

    def _make_items(self):
        sheet_manager = self._ui_model.get_sheet_manager()
        grid_catalog = sheet_manager.get_grid_catalog()

        for gp_id in grid_catalog.get_grid_pattern_ids():
            gp_name = grid_catalog.get_grid_pattern_name(gp_id)
            self._items.append((gp_id, gp_name))

    def unregister_updaters(self):
        pass

    # Qt interface

    def rowCount(self, parent):
        return len(self._items)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row = index.row()
            if 0 <= row < len(self._items):
                item = self._items[row]
                _, gp_name = item
                return QVariant(gp_name)

        return QVariant()

    def headerData(self, section, orientation, role):
        return QVariant()


class GridListView(QListView):

    def __init__(self):
        QListView.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setSelectionMode(QAbstractItemView.SingleSelection)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        for signal_type in ('clicked', 'activated'):
            signal = '{}(const QModelIndex&)'.format(signal_type)
            QObject.connect(
                    self, SIGNAL(signal), self._select_grid_pattern)

    def unregister_updaters(self):
        pass

    def _select_grid_pattern(self, index):
        item = self.model().get_item(index)
        if item:
            gp_id, _ = item

            sheet_manager = self._ui_model.get_sheet_manager()
            grid_catalog = sheet_manager.get_grid_catalog()
            grid_catalog.select_grid_pattern(gp_id)

            self._updater.signal_update(set(['signal_grid_pattern_selection']))


class GridList(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._grid_list_model = None
        self._grid_list_view = GridListView()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._grid_list_view)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._grid_list_view.set_ui_model(ui_model)
        self._update_model()

    def unregister_updaters(self):
        self._grid_list_view.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_grid_pattern_list' in signals:
            self._update_model()

    def _update_model(self):
        self._grid_list_model = GridListModel()
        self._grid_list_model.set_ui_model(self._ui_model)
        self._grid_list_view.setModel(self._grid_list_model)


class GridArea(QAbstractScrollArea):

    def __init__(self):
        QAbstractScrollArea.__init__(self)
        self.setFocusPolicy(Qt.NoFocus)

        self._ui_model = None
        self._updater = None

        # Widgets
        self.setViewport(GridView())

        self._corner = QWidget()
        self._corner.setStyleSheet('QWidget { background-color: #000 }')

        self._ruler = Ruler(is_grid_ruler=True)
        self._header = GridHeader()

        # Config
        self._config = None
        self._set_config({})

        # Layout
        g = QGridLayout()
        g.setSpacing(0)
        g.setMargin(0)
        g.addWidget(self._corner, 0, 0)
        g.addWidget(self._ruler, 1, 0)
        g.addWidget(self._header, 0, 1)
        self.setLayout(g)

        self.viewport().setFocusProxy(None)

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

        self.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.MinimumExpanding)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        # Default zoom level
        px_per_beat = self._config['trs_per_beat'] * self._config['tr_height']
        self._zoom_levels = utils.get_zoom_levels(
                16, px_per_beat, 512, self._config['zoom_factor'])
        self._default_zoom_index = self._zoom_levels.index(px_per_beat)

        self._set_px_per_beat(self._zoom_levels[self._default_zoom_index])

        self._ruler.set_ui_model(ui_model)
        self.viewport().set_ui_model(ui_model)

        self._update_selected_grid_pattern()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._ruler.unregister_updaters()
        self.viewport().unregister_updaters()

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

        for subcfg in ('ruler', 'header', 'edit_cursor', 'grid'):
            self._config[subcfg] = DEFAULT_CONFIG[subcfg].copy()
            if subcfg in config:
                self._config[subcfg].update(config[subcfg])

        fm = QFontMetrics(self._config['font'], self)
        self._config['font_metrics'] = fm
        self._config['tr_height'] = fm.tightBoundingRect('Ag').height() + 1

        self._header.set_config(self._config)
        self._ruler.set_config(self._config['ruler'])

        header_height = self._header.minimumSizeHint().height()
        ruler_width = self._ruler.sizeHint().width()

        self.setViewportMargins(ruler_width, header_height, 0, 0)

        self._corner.setFixedSize(ruler_width, header_height)
        self._header.setFixedHeight(header_height)
        self._ruler.setFixedWidth(ruler_width)
        self.viewport().set_config(self._config)

    def _set_px_per_beat(self, px_per_beat):
        self._ruler.set_px_per_beat(px_per_beat)
        self.viewport().set_px_per_beat(px_per_beat)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_grid_pattern_selection', 'signal_grid_pattern_modified'])
        if not signals.isdisjoint(update_signals):
            self._update_selected_grid_pattern()

    def _update_selected_grid_pattern(self):
        self._ruler.update_grid_pattern()

    def sizeHint(self):
        width = (self._ruler.width() +
                self.viewport().width() +
                self.verticalScrollBar().width())
        return QSize(width, 200)

    def paintEvent(self, event):
        self.viewport().paintEvent(event)

    def resizeEvent(self, event):
        self.viewport().resizeEvent(event)

    def mousePressEvent(self, event):
        self.viewport().mousePressEvent(event)


class GridHeader(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._width = DEFAULT_CONFIG['col_width']

    def set_config(self, config):
        self._config = config
        self._width = config['col_width']
        self.update()

    def resizeEvent(self, event):
        self.update()

    def minimumSizeHint(self):
        fm = QFontMetrics(self._config['header']['font'], self)
        height = fm.tightBoundingRect('Ag').height()
        return QSize(self._width, height)

    def sizeHint(self):
        return self.minimumSizeHint()

    def paintEvent(self, event):
        painter = QPainter(self)
        bg_colour = utils.scale_colour(
                self._config['header']['bg_colour'], self._config['inactive_dim'])
        painter.setBackground(bg_colour)
        painter.eraseRect(0, 0, self.width(), self.height())


class GridView(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._ui_model = None
        self._updater = None
        self._config = None

        self._width = DEFAULT_CONFIG['col_width']
        self._px_offset = 0
        self._px_per_beat = None

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)
        self.setFocusPolicy(Qt.StrongFocus)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def set_config(self, config):
        self._config = config

        fm = self._config['font_metrics']
        em_px = int(math.ceil(fm.tightBoundingRect('m').width()))
        self._width = self._config['col_width'] * em_px
        self.setFixedWidth(self._width)

    def set_px_per_beat(self, px_per_beat):
        self._px_per_beat = px_per_beat

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_grid_pattern_selection',
            'signal_grid_pattern_line_selection',
            'signal_grid_pattern_modified'])
        if not signals.isdisjoint(update_signals):
            self.update()

    def _get_visible_grid_pattern_id(self, grid_patterns):
        gp_id = grid_patterns.get_selected_grid_pattern_id()
        if gp_id == None:
            all_ids = grid_patterns.get_grid_pattern_ids()
            if all_ids:
                gp_id = all_ids[0]

        return gp_id

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)

        # Background
        painter.setBackground(self._config['canvas_bg_colour'])
        painter.eraseRect(QRect(0, 0, self._width, self.height()))

        # Get grid pattern info
        sheet_manager = self._ui_model.get_sheet_manager()
        grid_patterns = sheet_manager.get_grid_catalog()
        gp_id = grid_patterns.get_selected_grid_pattern_id()
        if gp_id == None:
            return

        gp_length = grid_patterns.get_grid_pattern_length(gp_id)
        gp_lines = grid_patterns.get_grid_pattern_lines(gp_id)
        selected_line_ts = grid_patterns.get_selected_grid_pattern_line()

        # Column background
        painter.setBackground(self._config['bg_colour'])
        length_rems = gp_length.beats * tstamp.BEAT + gp_length.rem
        height_px = length_rems * self._px_per_beat // tstamp.BEAT
        bg_extent = height_px - self._px_offset
        painter.eraseRect(QRect(0, 0, self._width, bg_extent))

        # Grid lines
        selected_line_found = False
        for line in gp_lines:
            line_ts_raw, line_style = line

            line_ts = tstamp.Tstamp(line_ts_raw)
            if line_ts >= gp_length:
                continue

            abs_y = utils.get_px_from_tstamp(line_ts, self._px_per_beat)
            y_offset = abs_y - self._px_offset
            if not 0 <= y_offset < self.height():
                continue

            pen = QPen(self._config['grid']['styles'][line_style])
            painter.setPen(pen)
            painter.drawLine(QPoint(0, y_offset), QPoint(self._width - 1, y_offset))

            if line_ts == selected_line_ts:
                selected_line_found = True

        if selected_line_found:
            cursor_config = self._config['grid']['edit_cursor']
            cursor_max_y = (cursor_config['height'] - 1) // 2

            abs_y = utils.get_px_from_tstamp(selected_line_ts, self._px_per_beat)
            y_offset = abs_y - self._px_offset

            painter.setRenderHint(QPainter.Antialiasing)
            painter.translate(QPointF(0.5, 0.5 + y_offset))
            painter.setPen(cursor_config['colour'])
            painter.setBrush(cursor_config['colour'])
            painter.drawPolygon(
                    QPoint(0, cursor_max_y),
                    QPoint(cursor_config['width'], 0),
                    QPoint(0, -cursor_max_y))

        end = time.time()
        elapsed = end - start
        #print('Grid pattern view updated in {:.2f} ms'.format(elapsed * 1000))

    def mousePressEvent(self, event):
        if not event.buttons() == Qt.LeftButton:
            return

        # Get grid pattern info
        sheet_manager = self._ui_model.get_sheet_manager()
        grid_patterns = sheet_manager.get_grid_catalog()
        gp_id = self._get_visible_grid_pattern_id(grid_patterns)
        if gp_id == None:
            return

        gp_length = grid_patterns.get_grid_pattern_length(gp_id)
        gp_lines = grid_patterns.get_grid_pattern_lines(gp_id)

        # Get timestamp at clicked position
        rel_y_offset = event.y()
        y_offset = rel_y_offset + self._px_offset
        click_ts = utils.get_tstamp_from_px(y_offset, self._px_per_beat)

        # Find the nearest grid line
        nearest_ts = None
        nearest_dist = gp_length * 2
        for line in gp_lines:
            line_ts, _ = line
            dist = abs(click_ts - line_ts)
            if dist < nearest_dist:
                nearest_ts = line_ts
                nearest_dist = dist

        assert nearest_ts != None
        grid_patterns.select_grid_pattern_line(nearest_ts)
        self._updater.signal_update(set(['signal_grid_pattern_line_selection']))


class GeneralEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._length = QDoubleSpinBox()
        self._length.setMinimum(1)
        self._length.setMaximum(32)
        self._length.setDecimals(3)

        self._spacing_style = LineStyle(is_major_enabled=True)
        self._spacing_value = QDoubleSpinBox()
        self._spacing_value.setMinimum(0.001)
        self._spacing_value.setMaximum(1.0)
        self._spacing_value.setDecimals(3)

        self._offset = QDoubleSpinBox()
        self._offset.setMinimum(0)
        self._offset.setMaximum(32)
        self._offset.setDecimals(3)

        ll = QHBoxLayout()
        ll.setMargin(0)
        ll.setSpacing(2)
        ll.addWidget(QLabel('Grid length:'), 0)
        ll.addWidget(self._length, 1)

        sl = QHBoxLayout()
        sl.setMargin(0)
        sl.setSpacing(2)
        sl.addWidget(QLabel('Min. spacing of style'), 0)
        sl.addWidget(self._spacing_style, 2)
        sl.addWidget(QLabel(':'), 0)
        sl.addWidget(self._spacing_value, 1)

        ol = QHBoxLayout()
        ol.setMargin(0)
        ol.setSpacing(2)
        ol.addWidget(QLabel('Grid offset:'), 0)
        ol.addWidget(self._offset, 1)

        v = QVBoxLayout()
        v.setContentsMargins(4, 0, 4, 0)
        v.setSpacing(2)
        v.addLayout(ll)
        v.addLayout(sl)
        v.addLayout(ol)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self._length, SIGNAL('valueChanged(double)'), self._change_length)

        QObject.connect(
                self._spacing_style,
                SIGNAL('currentIndexChanged(int)'),
                self._select_spacing_style)

        QObject.connect(
                self._spacing_value,
                SIGNAL('valueChanged(double)'),
                self._change_spacing)

        QObject.connect(
                self._offset, SIGNAL('valueChanged(double)'), self._change_offset)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_grid_pattern_selection', 'signal_grid_pattern_modified'])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _get_grid_patterns(self):
        sheet_manager = self._ui_model.get_sheet_manager()
        grid_patterns = sheet_manager.get_grid_catalog()
        return grid_patterns

    def _update_all(self):
        grid_patterns = self._get_grid_patterns()
        gp_id = grid_patterns.get_selected_grid_pattern_id()

        self.setEnabled(gp_id != None)
        if gp_id == None:
            return

        gp_length = grid_patterns.get_grid_pattern_length(gp_id)
        gp_length_f = float(gp_length)

        if gp_length_f != self._length.value():
            old_block = self._length.blockSignals(True)
            self._length.setValue(gp_length_f)
            self._length.blockSignals(old_block)

        spacing_value = grid_patterns.get_grid_pattern_line_style_spacing(
                gp_id, self._spacing_style.get_current_line_style())

        if spacing_value != self._spacing_value.value():
            old_block = self._spacing_value.blockSignals(True)
            self._spacing_value.setValue(spacing_value)
            self._spacing_value.blockSignals(old_block)

        gp_offset = grid_patterns.get_grid_pattern_offset(gp_id)
        gp_offset_f = float(gp_offset)

        if gp_offset_f != self._offset.value():
            old_block = self._offset.blockSignals(True)
            self._offset.setValue(gp_offset_f)
            self._offset.blockSignals(old_block)

    def _change_length(self, new_length):
        new_length_ts = tstamp.Tstamp(new_length)

        grid_patterns = self._get_grid_patterns()
        gp_id = grid_patterns.get_selected_grid_pattern_id()
        if gp_id == None:
            return

        grid_patterns.set_grid_pattern_length(gp_id, new_length_ts)
        self._updater.signal_update(set(['signal_grid_pattern_modified']))

    def _select_spacing_style(self, new_style):
        self._spacing_style.select_line_style(new_style)
        self._update_all()

    def _change_spacing(self, new_spacing):
        grid_patterns = self._get_grid_patterns()
        gp_id = grid_patterns.get_selected_grid_pattern_id()
        if gp_id == None:
            return

        line_style = self._spacing_style.get_current_line_style()
        grid_patterns.set_grid_pattern_line_style_spacing(
                gp_id, line_style, new_spacing)
        self._updater.signal_update(set(['signal_grid_pattern_modified']))

    def _change_offset(self, new_offset):
        new_offset_ts = tstamp.Tstamp(new_offset)

        grid_patterns = self._get_grid_patterns()
        gp_id = grid_patterns.get_selected_grid_pattern_id()
        if gp_id == None:
            return

        grid_patterns.set_grid_pattern_offset(gp_id, new_offset_ts)
        self._updater.signal_update(set(['signal_grid_pattern_modified']))


class SubdivEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._warp_value = 0.5

        self._subdiv_count = QSpinBox()
        self._subdiv_count.setMinimum(2)
        self._subdiv_count.setMaximum(32)
        self._subdiv_line_style = LineStyle()
        self._subdiv_warp = NumberSlider(3, 0.001, 0.999)
        self._subdiv_warp.set_number(self._warp_value)
        self._subdiv_apply = QPushButton('Subdivide')

        self._subdiv_line_style.select_line_style(1)

        sl = QGridLayout()
        sl.setMargin(0)
        sl.setSpacing(2)
        sl.setColumnStretch(0, 0)
        sl.setColumnStretch(1, 1)
        sl.addWidget(QLabel('Parts:'), 0, 0)
        sl.addWidget(self._subdiv_count, 0, 1)
        sl.addWidget(QLabel('Style:'), 1, 0)
        sl.addWidget(self._subdiv_line_style, 1, 1)
        sl.addWidget(QLabel('Warp:'), 2, 0)
        sl.addWidget(self._subdiv_warp, 2, 1)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Create subdivision'), 0, Qt.AlignTop)
        v.addLayout(sl, 0)
        v.addWidget(self._subdiv_apply, 0)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self._subdiv_warp, SIGNAL('numberChanged(float)'), self._update_warp)

        QObject.connect(self._subdiv_apply, SIGNAL('clicked()'), self._apply_subdivision)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_grid_pattern_selection',
            'signal_grid_pattern_line_selection',
            'signal_grid_pattern_modified'])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _get_grid_patterns(self):
        sheet_manager = self._ui_model.get_sheet_manager()
        grid_patterns = sheet_manager.get_grid_catalog()
        return grid_patterns

    def _get_selected_line_ts(self, grid_patterns, gp_id):
        if gp_id == None:
            return None

        # Get line selection info
        gp_lines = grid_patterns.get_grid_pattern_lines(gp_id)
        lines_dict = dict(gp_lines)
        selected_line_ts = grid_patterns.get_selected_grid_pattern_line()

        return selected_line_ts if selected_line_ts in lines_dict else None

    def _update_all(self):
        grid_patterns = self._get_grid_patterns()
        gp_id = grid_patterns.get_selected_grid_pattern_id()
        selected_line_ts = self._get_selected_line_ts(grid_patterns, gp_id)
        self.setEnabled(selected_line_ts != None)

    def _update_warp(self, new_warp):
        self._warp_value = new_warp
        self._subdiv_warp.set_number(self._warp_value)

    def _apply_subdivision(self):
        grid_patterns = self._get_grid_patterns()
        gp_id = grid_patterns.get_selected_grid_pattern_id()
        if gp_id == None:
            return

        part_count = self._subdiv_count.value()
        line_style = self._subdiv_line_style.get_current_line_style()
        assert line_style != 0

        selected_line_ts = self._get_selected_line_ts(grid_patterns, gp_id)

        grid_patterns.subdivide_grid_pattern_interval(
                gp_id, selected_line_ts, part_count, self._warp_value, line_style)
        self._updater.signal_update(set(['signal_grid_pattern_modified']))


class LineEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._line_style = LineStyle()
        self._remove_button = QPushButton('Remove line')

        ls = QHBoxLayout()
        ls.setMargin(0)
        ls.setSpacing(2)
        ls.addWidget(QLabel('Style:'), 0)
        ls.addWidget(self._line_style, 1)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Current line'), 0, Qt.AlignTop)
        v.addLayout(ls, 0)
        v.addWidget(self._remove_button, 0, Qt.AlignTop)
        v.addWidget(QWidget(), 1)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self._line_style,
                SIGNAL('currentIndexChanged(int)'),
                self._change_line_style)
        QObject.connect(
                self._remove_button, SIGNAL('clicked()'), self._remove_selected_line)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_grid_pattern_selection',
            'signal_grid_pattern_line_selection',
            'signal_grid_pattern_modified'])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _get_grid_patterns(self):
        sheet_manager = self._ui_model.get_sheet_manager()
        grid_patterns = sheet_manager.get_grid_catalog()
        return grid_patterns

    def _update_all(self):
        grid_patterns = self._get_grid_patterns()
        gp_id = grid_patterns.get_selected_grid_pattern_id()
        if gp_id == None:
            self.setEnabled(False)
            return

        # Get line selection info
        gp_lines = grid_patterns.get_grid_pattern_lines(gp_id)
        lines_dict = dict(gp_lines)
        selected_line_ts = grid_patterns.get_selected_grid_pattern_line()
        has_selected_line = selected_line_ts in lines_dict
        is_selected_line_major = (lines_dict.get(selected_line_ts, None) == 0)

        # Set editor parts as enabled if we have an appropriate selection
        self.setEnabled(has_selected_line)

        self._remove_button.setEnabled(not is_selected_line_major)
        self._line_style.setEnabled(not is_selected_line_major)

        # Set line style selection
        if has_selected_line:
            selected_line_style = lines_dict[selected_line_ts]
            self._line_style.select_line_style(selected_line_style)

    def _change_line_style(self, ls_index):
        line_style = self._line_style.get_current_line_style()
        assert line_style != None

        grid_patterns = self._get_grid_patterns()
        gp_id = grid_patterns.get_selected_grid_pattern_id()
        assert gp_id != None

        selected_line_ts = grid_patterns.get_selected_grid_pattern_line()
        grid_patterns.change_grid_pattern_line_style(
                gp_id, selected_line_ts, line_style)
        self._updater.signal_update(set(['signal_grid_pattern_modified']))

    def _remove_selected_line(self):
        grid_patterns = self._get_grid_patterns()
        gp_id = grid_patterns.get_selected_grid_pattern_id()
        assert gp_id != None

        selected_line_ts = grid_patterns.get_selected_grid_pattern_line()
        grid_patterns.remove_grid_pattern_line(gp_id, selected_line_ts)
        self._updater.signal_update(set(['signal_grid_pattern_modified']))


class LineStyleDelegate(QItemDelegate):

    def __init__(self, is_major_enabled):
        QItemDelegate.__init__(self)
        self._config = None

        self._first_style = 0 if is_major_enabled else 1

        self._pixmaps = {}
        self._list_pixmap = None

        self._pixmap_width = 0
        self._pixmap_height = 0
        self._pixmap_horiz_margin = 0

    def set_config(self, config):
        self._config = config

        fm = QFontMetrics(self._config['font'], QWidget())
        em_px = int(math.ceil(fm.tightBoundingRect('m').width()))
        self._line_length = self._config['col_width'] * em_px

        # Pixmap style
        h_margin = 10
        v_margin = 8
        p_width = self._line_length + 2 * h_margin + 1
        p_height = 1 + 2 * v_margin

        self._pixmap_horiz_margin = h_margin
        self._pixmap_width = p_width
        self._pixmap_height = p_height

        # Create line pixmaps
        for i in xrange(STYLE_COUNT):
            pixmap = self._create_line_pixmap(i, p_width, p_height, h_margin, v_margin)
            self._pixmaps[i] = pixmap

        # Create list pixmap (work-around for incorrect drawing of selected entries)
        list_width = self._pixmap_width
        list_height = (STYLE_COUNT - self._first_style) * self._pixmap_height
        self._list_pixmap = QPixmap(list_width, list_height)

        lpainter = QPainter(self._list_pixmap)
        for i in xrange(self._first_style, STYLE_COUNT):
            pixmap = self._pixmaps[i]
            lpainter.drawPixmap(0, 0, pixmap)
            lpainter.translate(0, pixmap.height())

    def get_line_pixmap(self, line_style):
        return self._pixmaps[line_style]

    def _create_line_pixmap(self, line_style, width, height, horiz_margin, vert_margin):
        pixmap = QPixmap(width, height)

        painter = QPainter(pixmap)

        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(QRect(0, 0, width, height))

        painter.setPen(self._config['grid']['styles'][line_style])

        painter.translate(horiz_margin, vert_margin)
        painter.drawLine(0, 0, self._line_length, 0)

        return pixmap

    def paint(self, painter, option, index):
        pixmap_index_data = index.data(Qt.UserRole).toInt()
        pixmap_index, _ = pixmap_index_data
        assert pixmap_index in self._pixmaps

        # Background
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(option.rect)

        # Line pixmap
        cursor_width = self._config['grid']['edit_cursor']['width']
        pixmap = self._pixmaps[pixmap_index]
        pixmap_pos = option.rect.translated(cursor_width, 0).topLeft()
        painter.drawPixmap(QRect(pixmap_pos, pixmap.size()), pixmap)
        QItemDelegate.paint(self, painter, option, index)

    def drawBackground(self, painter, option, index):
        pass

    def drawCheck(self, painter, option, rect, state):
        pass

    def drawDecoration(self, painter, option, rect, pixmap):
        pass

    def drawDisplay(self, painter, option, rect, text):
        pass

    def drawFocus(self, painter, option, rect):
        if option.state & QStyle.State_HasFocus:
            cursor_width = self._config['grid']['edit_cursor']['width']

            # Background
            painter.setBackground(self._config['bg_colour'])
            painter.eraseRect(rect)

            # Line pixmap
            pixmap_size = QSize(self._pixmap_width, self._pixmap_height)
            pixmap_pos = rect.translated(cursor_width, 0).topLeft()
            pixmap_rect = QRect(rect.topLeft(), pixmap_size)
            painter.drawPixmap(pixmap_pos, self._list_pixmap, pixmap_rect)

            # Focus cursor triangle
            cursor_config = self._config['grid']['edit_cursor']
            cursor_max_y = (cursor_config['height'] - 1) // 2

            painter.save()
            painter.setRenderHint(QPainter.Antialiasing)

            arrow_y_offset = pixmap_pos.y() + (self._pixmap_height - 1) // 2
            arrow_x_offset = (self._pixmap_horiz_margin // 2) - 1
            painter.translate(QPointF(0.5 + arrow_x_offset, 0.5 + arrow_y_offset))

            painter.setPen(cursor_config['colour'])
            painter.setBrush(cursor_config['colour'])

            painter.drawPolygon(
                    QPoint(0, cursor_max_y),
                    QPoint(cursor_config['width'], 0),
                    QPoint(0, -cursor_max_y))

            painter.restore()

    def sizeHint(self, option, index):
        pixmap_index_data = index.data(Qt.UserRole).toInt()
        pixmap_index, _ = pixmap_index_data
        assert pixmap_index in self._pixmaps

        pixmap = self._pixmaps[pixmap_index]
        return pixmap.size()


class LineStyle(QComboBox):

    def __init__(self, is_major_enabled=False):
        QComboBox.__init__(self)

        self._first_style = 0 if is_major_enabled else 1
        self._is_major_displayed = True
        self._ls_delegate = None

        self.set_config(DEFAULT_CONFIG)

    def set_config(self, config):
        self._config = config

        is_major_enabled = (self._first_style == 0)
        self._ls_delegate = LineStyleDelegate(is_major_enabled)
        self._ls_delegate.set_config(self._config)

        self.setItemDelegate(self._ls_delegate)

        for i in xrange(self._first_style, STYLE_COUNT):
            self.addItem(str(i), QVariant(i))

    def get_current_line_style(self):
        line_style, success = self.itemData(self.currentIndex()).toInt()
        return line_style if success else None

    def select_line_style(self, new_style):
        old_block = self.blockSignals(True)

        if new_style == 0:
            self._is_major_displayed = True
        else:
            self._is_major_displayed = False
            self.setCurrentIndex(new_style - self._first_style)

        self.blockSignals(old_block)
        self.update()

    def paintEvent(self, event):
        painter = QStylePainter(self)
        painter.setPen(self.palette().color(QPalette.Text))

        option = QStyleOptionComboBox()
        self.initStyleOption(option)
        painter.drawComplexControl(QStyle.CC_ComboBox, option)

        arrow_rect = painter.style().subControlRect(
                QStyle.CC_ComboBox, option, QStyle.SC_ComboBoxArrow, self)
        arrow_width = arrow_rect.width()

        if self._is_major_displayed:
            line_style = 0
        else:
            line_style = self.get_current_line_style()
        pixmap = self._ls_delegate.get_line_pixmap(line_style)

        top = (self.height() - pixmap.height()) // 2
        left = (self.width() - pixmap.width() - arrow_width) // 2

        painter.drawPixmap(left, top, pixmap)

        if not self.isEnabled():
            painter.fillRect(
                    left, top,
                    pixmap.width(), pixmap.height(),
                    self._config['disabled_colour'])


