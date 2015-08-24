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
from kunquat.tracker.ui.views.headerline import HeaderLine
import utils


class GridEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._grid_list = GridList()

        self._grid_area = GridArea()

        el = QHBoxLayout()
        el.setMargin(0)
        el.setSpacing(0)
        el.addWidget(self._grid_area)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._grid_list, 1)
        v.addSpacing(4)
        v.addWidget(HeaderLine('Grid editor'))
        v.addLayout(el, 4)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._grid_list.set_ui_model(ui_model)
        self._grid_area.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._grid_area.unregister_updaters()
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
            grid_pattern = grid_catalog.get_grid_pattern(gp_id)
            self._items.append((gp_id, grid_pattern))

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
                _, gp = item
                return QVariant(gp['name'])

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
        if 'signal_grid_pattern_selection' in signals:
            self._update_selected_grid_pattern()

    def _update_selected_grid_pattern(self):
        self._ruler.update_grid_pattern()

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

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setBackground(self._config['header']['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        painter.setPen(self._config['header']['border_colour'])
        painter.drawLine(self.width() - 1, 0, self.width() - 1, self.height() - 1)


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
        if 'signal_grid_pattern_selection' in signals:
            self.update()

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)

        # Background
        painter.setBackground(self._config['canvas_bg_colour'])
        painter.eraseRect(QRect(0, 0, self._width, self.height()))

        # Get selected grid pattern
        sheet_manager = self._ui_model.get_sheet_manager()
        grid_patterns = sheet_manager.get_grid_catalog()
        gp_id = grid_patterns.get_selected_grid_pattern_id()
        if gp_id == None:
            all_ids = grid_patterns.get_grid_pattern_ids()
            if all_ids:
                gp_id = all_ids[0]
            else:
                return

        gp = grid_patterns.get_grid_pattern(gp_id)

        # Column background
        painter.setBackground(self._config['bg_colour'])
        length_ts = tstamp.Tstamp(gp['length'])
        length_rems = length_ts.beats * tstamp.BEAT + length_ts.rem
        height_px = length_rems * self._px_per_beat // tstamp.BEAT
        bg_extent = height_px - self._px_offset
        painter.eraseRect(QRect(0, 0, self._width, bg_extent))

        # Grid lines
        for line in gp['lines']:
            line_ts_raw, line_style = line

            line_ts = tstamp.Tstamp(line_ts_raw)
            if line_ts >= gp['length']:
                continue

            rems = line_ts.beats * tstamp.BEAT + line_ts.rem
            abs_y = rems * self._px_per_beat // tstamp.BEAT
            y_offset = abs_y - self._px_offset
            if not 0 <= y_offset < self.height():
                continue

            pen = QPen(self._config['grid']['styles'][line_style])
            painter.setPen(pen)
            painter.drawLine(QPoint(0, y_offset), QPoint(self._width - 1, y_offset))

        end = time.time()
        elapsed = end - start
        #print('Grid pattern view updated in {:.2f} ms'.format(elapsed * 1000))


