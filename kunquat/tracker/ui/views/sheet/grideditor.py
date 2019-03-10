# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2019
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

from kunquat.tracker.ui.qt import *

import kunquat.tracker.ui.model.tstamp as tstamp
from kunquat.tracker.ui.model.gridpattern import STYLE_COUNT
from kunquat.tracker.ui.views.headerline import HeaderLine
from kunquat.tracker.ui.views.kqtcombobox import KqtComboBox
from kunquat.tracker.ui.views.numberslider import NumberSlider
from kunquat.tracker.ui.views.updater import Updater
from kunquat.tracker.ui.views.varprecspinbox import VarPrecSpinBox
from .config import *
from .ruler import Ruler
from . import utils


class GridEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()

        self._grid_list = GridList()
        self._general_editor = GeneralEditor()

        self._grid_area = GridArea()
        self._subdiv_editor = SubdivEditor()
        self._line_editor = LineEditor()

        self.add_to_updaters(
                self._grid_list,
                self._general_editor,
                self._grid_area,
                self._subdiv_editor,
                self._line_editor)

        self._line_layout = QVBoxLayout()
        self._line_layout.setContentsMargins(0, 0, 0, 0)
        self._line_layout.setSpacing(8)
        self._line_layout.addWidget(self._subdiv_editor)
        self._line_layout.addWidget(self._line_editor)

        self._details_layout = QHBoxLayout()
        self._details_layout.setContentsMargins(0, 0, 0, 0)
        self._details_layout.setSpacing(4)
        self._details_layout.addWidget(self._grid_area)
        self._details_layout.addLayout(self._line_layout)

        self._header = HeaderLine('Grid editor')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._grid_list, 1)
        v.addSpacing(2)
        v.addWidget(self._header)
        v.addWidget(self._general_editor)
        v.addLayout(self._details_layout, 4)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        self._header.update_style(style_mgr)

        self._line_layout.setSpacing(style_mgr.get_scaled_size_param('large_padding'))
        self._details_layout.setSpacing(
                style_mgr.get_scaled_size_param('medium_padding'))

        layout = self.layout()
        spacing = style_mgr.get_scaled_size_param('small_padding')
        layout.setSpacing(spacing)
        for i in range(layout.count()):
            spacer = layout.itemAt(i).spacerItem()
            if spacer:
                spacer.changeSize(2, spacing)


class GridListModel(QAbstractListModel, Updater):

    def __init__(self):
        super().__init__()
        self._items = []

    def _on_setup(self):
        self._make_items()

    def get_item(self, index):
        row = index.row()
        if 0 <= row < len(self._items):
            item = self._items[row]
            return item
        return None

    def _make_items(self):
        grid_mgr = self._ui_model.get_grid_manager()

        for gp_id in grid_mgr.get_editable_grid_pattern_ids():
            gp = grid_mgr.get_grid_pattern(gp_id)
            gp_name = gp.get_name()
            self._items.append((gp_id, gp_name))

        self._items = sorted(self._items, key=lambda x: x[1])

    # Qt interface

    def rowCount(self, parent):
        return len(self._items)

    def data(self, index, role):
        if role == Qt.DisplayRole:
            row = index.row()
            if 0 <= row < len(self._items):
                item = self._items[row]
                _, gp_name = item
                return gp_name

        return None

    def headerData(self, section, orientation, role):
        return None


class GridListView(QListView, Updater):

    def __init__(self):
        super().__init__()
        self.setSelectionMode(QAbstractItemView.SingleSelection)

    def _on_setup(self):
        self.clicked.connect(self._select_grid_pattern)
        self.activated.connect(self._select_grid_pattern)

    def _select_grid_pattern(self, index):
        item = self.model().get_item(index)
        if item:
            gp_id, _ = item

            grid_mgr = self._ui_model.get_grid_manager()
            grid_mgr.select_grid_pattern(gp_id)

            self._updater.signal_update('signal_grid_pattern_selection')


class GridList(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._toolbar = GridListToolBar()

        self._grid_list_model = None
        self._grid_list_view = GridListView()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._grid_list_view)
        self.setLayout(v)

    def _on_setup(self):
        self.add_to_updaters(self._toolbar, self._grid_list_view)
        self.register_action('signal_grid_pattern_list', self._update_model)

        self._update_model()

    def _update_model(self):
        if self._grid_list_model:
            self.remove_from_updaters(self._grid_list_model)
        self._grid_list_model = GridListModel()
        self.add_to_updaters(self._grid_list_model)
        self._grid_list_view.setModel(self._grid_list_model)


class GridListToolBar(QToolBar, Updater):

    def __init__(self):
        super().__init__()
        self._new_button = QToolButton()
        self._new_button.setText('New grid')
        self._new_button.setEnabled(True)

        self._remove_button = QToolButton()
        self._remove_button.setText('Remove grid')
        self._remove_button.setEnabled(False)

        self.addWidget(self._new_button)
        self.addWidget(self._remove_button)

    def _on_setup(self):
        self.register_action('signal_grid_pattern_list', self._update_all)
        self.register_action('signal_grid_pattern_selection', self._update_all)

        self._new_button.clicked.connect(self._add_grid_pattern)
        self._remove_button.clicked.connect(self._remove_grid_pattern)

        self._update_all()

    def _update_all(self):
        grid_mgr = self._ui_model.get_grid_manager()
        gp_count = len(grid_mgr.get_editable_grid_pattern_ids())
        selected_gp_id = grid_mgr.get_selected_grid_pattern_id()
        self._remove_button.setEnabled((gp_count > 0) and (selected_gp_id != None))

    def _add_grid_pattern(self):
        grid_mgr = self._ui_model.get_grid_manager()
        grid_mgr.add_grid_pattern()
        self._updater.signal_update('signal_grid_pattern_list')

    def _remove_grid_pattern(self):
        grid_mgr = self._ui_model.get_grid_manager()

        gp_id = grid_mgr.get_selected_grid_pattern_id()
        if gp_id == None:
            return

        grid_mgr.remove_grid_pattern(gp_id)
        grid_mgr.select_grid_pattern(None)
        self._updater.signal_update(
            'signal_grid_pattern_list',
            'signal_grid_pattern_modified',
            'signal_grid_pattern_selection')


class Corner(QWidget):

    def __init__(self):
        super().__init__()

        self._bg_colour = QColor(0, 0, 0)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_config(self, config):
        self._bg_colour = config['bg_colour']

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setBackground(self._bg_colour)
        painter.eraseRect(event.rect())


class GridArea(QAbstractScrollArea, Updater):

    def __init__(self):
        super().__init__()
        self.setFocusPolicy(Qt.NoFocus)

        self._config = None

        # Widgets
        self.setViewport(GridView())

        self._corner = Corner()

        self._ruler = Ruler(is_grid_ruler=True)
        self._header = GridHeader()

        # Layout
        g = QGridLayout()
        g.setSpacing(0)
        g.setContentsMargins(0, 0, 0, 0)
        g.addWidget(self._corner, 0, 0)
        g.addWidget(self._ruler, 1, 0)
        g.addWidget(self._header, 0, 1)
        self.setLayout(g)

        self.viewport().setFocusProxy(None)

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

        self.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.MinimumExpanding)

    def _on_setup(self):
        self.register_action(
                'signal_grid_pattern_selection', self._update_selected_grid_pattern)
        self.register_action(
                'signal_grid_pattern_modified', self._update_selected_grid_pattern)
        self.register_action('signal_grid_zoom', self._update_zoom)
        self.register_action('signal_style_changed', self._update_style)

        self._update_config()

        grid_mgr = self._ui_model.get_grid_manager()

        # Default zoom level
        px_per_beat = self._config['trs_per_beat'] * self._config['tr_height']
        self._zoom_levels = utils.get_zoom_levels(
                16, px_per_beat, 512, self._config['zoom_factor'])
        self._default_zoom_index = self._zoom_levels.index(px_per_beat)
        grid_mgr.set_zoom_range(
                -self._default_zoom_index,
                len(self._zoom_levels) - self._default_zoom_index - 1)

        self._set_px_per_beat(self._zoom_levels[self._default_zoom_index])

        self.add_to_updaters(self._ruler, self.viewport())

        self.viewport().followCursor.connect(self._follow_cursor)

        self._update_selected_grid_pattern()

    def _update_style(self):
        self._update_config()
        self.update()

    def _update_config(self):
        style_mgr = self._ui_model.get_style_manager()

        self._config = DEFAULT_CONFIG.copy()
        config = get_config_with_custom_style(style_mgr)
        self._config.update(config)

        for subcfg in ('ruler', 'header', 'edit_cursor', 'grid'):
            self._config[subcfg] = DEFAULT_CONFIG[subcfg].copy()
            if subcfg in config:
                self._config[subcfg].update(config[subcfg])

        self._corner.set_config(self._config)

        fm = QFontMetrics(self._config['font'], self)
        self._config['font_metrics'] = fm
        self._config['tr_height'] = (fm.tightBoundingRect('A').height() +
                self._config['trigger']['padding_y'] * 2)

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

    def _update_zoom(self):
        grid_mgr = self._ui_model.get_grid_manager()

        zoom_level = grid_mgr.get_zoom()
        cur_zoom_index = zoom_level + self._default_zoom_index
        self._set_px_per_beat(self._zoom_levels[cur_zoom_index])

    def _update_selected_grid_pattern(self):
        self._ruler.update_grid_pattern()

    def _update_scrollbars(self):
        grid_mgr = self._ui_model.get_grid_manager()
        gp_id = grid_mgr.get_selected_grid_pattern_id()
        if gp_id == None:
            self.verticalScrollBar().setRange(0, 0)
            return

        total_height_px = self.viewport().get_total_height()
        vp_height = self.viewport().height()
        vscrollbar = self.verticalScrollBar()
        vscrollbar.setPageStep(vp_height)
        vscrollbar.setRange(0, total_height_px - vp_height)

    def _follow_cursor(self, new_y_offset_str):
        new_y_offset = int(new_y_offset_str)

        vscrollbar = self.verticalScrollBar()
        old_y_offset = vscrollbar.value()

        self._update_scrollbars()
        vscrollbar.setValue(new_y_offset)

        self.viewport().update()

    def scrollContentsBy(self, dx, dy):
        px_offset = self.verticalScrollBar().value()
        self._ruler.set_px_offset(px_offset)
        self.viewport().set_px_offset(px_offset)

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
        super().__init__()
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


class GridView(QWidget, Updater):

    followCursor = Signal(str, name='followCursor')

    def __init__(self):
        super().__init__()
        self._config = None

        self._width = DEFAULT_CONFIG['col_width']
        self._px_offset = 0
        self._px_per_beat = None

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)
        self.setFocusPolicy(Qt.StrongFocus)

    def _on_setup(self):
        self.register_action('signal_grid_pattern_selection', self._follow_edit_cursor)
        self.register_action(
                'signal_grid_pattern_line_selection', self._follow_edit_cursor)
        self.register_action('signal_grid_pattern_modified', self.update)
        self.register_action('signal_grid_zoom', self.update)

    def set_config(self, config):
        self._config = config

        fm = self._config['font_metrics']
        em_px = int(math.ceil(fm.tightBoundingRect('m').width()))
        self._width = self._config['col_width'] * em_px
        self.setFixedWidth(self._width)

    def set_px_offset(self, new_offset):
        if self._px_offset != new_offset:
            self._px_offset = new_offset
            self.update()

    def set_px_per_beat(self, px_per_beat):
        if self._px_per_beat != px_per_beat:
            orig_px_per_beat = self._px_per_beat
            orig_px_offset = self._px_offset

            self._px_per_beat = px_per_beat

            if not self._ui_model:
                return

            # Get old edit cursor offset
            grid_mgr = self._ui_model.get_grid_manager()
            selected_line_ts = tstamp.Tstamp(0)
            gp_id = grid_mgr.get_selected_grid_pattern_id()
            if gp_id != None:
                gp = grid_mgr.get_grid_pattern(gp_id)
                selected_line_ts = gp.get_selected_line() or tstamp.Tstamp(0)
            orig_relative_offset = utils.get_px_from_tstamp(
                    selected_line_ts, orig_px_per_beat) - orig_px_offset

            # Adjust vertical position so that edit cursor maintains its height
            new_cursor_offset = utils.get_px_from_tstamp(selected_line_ts, px_per_beat)
            new_px_offset = new_cursor_offset - orig_relative_offset
            self.followCursor.emit(str(new_px_offset))

    def get_total_height(self):
        grid_mgr = self._ui_model.get_grid_manager()
        gp_id = grid_mgr.get_selected_grid_pattern_id()
        if gp_id == None:
            return 0
        gp = grid_mgr.get_grid_pattern(gp_id)

        gp_length = gp.get_length()

        return (utils.get_px_from_tstamp(gp_length, self._px_per_beat) +
                self._config['tr_height'])

    def _follow_edit_cursor(self):
        grid_mgr = self._ui_model.get_grid_manager()
        gp_id = grid_mgr.get_selected_grid_pattern_id()
        if gp_id == None:
            return
        gp = grid_mgr.get_grid_pattern(gp_id)

        selected_line_ts = gp.get_selected_line() or tstamp.Tstamp(0)
        cursor_abs_y = utils.get_px_from_tstamp(selected_line_ts, self._px_per_beat)
        cursor_rel_y = cursor_abs_y - self._px_offset

        is_scrolling_required = False

        min_snap_dist = self._config['edit_cursor']['min_snap_dist']
        min_centre_dist = min(min_snap_dist, self.height() // 2)
        min_y_offset = min_centre_dist
        max_y_offset = self.height() - min_centre_dist

        if cursor_rel_y < min_centre_dist:
            is_scrolling_required = True
            new_px_offset = self._px_offset - (min_y_offset - cursor_rel_y)
        elif cursor_rel_y >= max_y_offset:
            is_scrolling_required = True
            new_px_offset = self._px_offset + (cursor_rel_y - max_y_offset)

        if is_scrolling_required:
            self.followCursor.emit(str(new_px_offset))

        self.update()

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)

        # Background
        painter.setBackground(self._config['canvas_bg_colour'])
        painter.eraseRect(QRect(0, 0, self._width, self.height()))

        # Get grid pattern info
        grid_mgr = self._ui_model.get_grid_manager()
        gp_id = grid_mgr.get_selected_grid_pattern_id()
        if gp_id == None:
            return
        gp = grid_mgr.get_grid_pattern(gp_id)

        gp_length = gp.get_length()
        gp_lines = gp.get_lines()
        selected_line_ts = gp.get_selected_line()

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
            painter.drawPolygon(QPolygon([
                    QPoint(0, cursor_max_y),
                    QPoint(cursor_config['width'], 0),
                    QPoint(0, -cursor_max_y)]))

        end = time.time()
        elapsed = end - start
        #print('Grid pattern view updated in {:.2f} ms'.format(elapsed * 1000))

    def mousePressEvent(self, event):
        if not event.buttons() == Qt.LeftButton:
            return

        # Get grid pattern info
        grid_mgr = self._ui_model.get_grid_manager()
        gp_id = grid_mgr.get_selected_grid_pattern_id()
        if gp_id == None:
            return
        gp = grid_mgr.get_grid_pattern(gp_id)

        gp_length = gp.get_length()
        gp_lines = gp.get_lines()

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
        gp.select_line(nearest_ts)
        self._updater.signal_update('signal_grid_pattern_line_selection')

    def keyPressEvent(self, event):
        # Get grid pattern info
        grid_mgr = self._ui_model.get_grid_manager()
        gp_id = grid_mgr.get_selected_grid_pattern_id()
        if gp_id == None:
            return
        gp = grid_mgr.get_grid_pattern(gp_id)

        if event.modifiers() == Qt.NoModifier:
            if event.key() == Qt.Key_Up:
                gp.select_prev_line()
                self._updater.signal_update('signal_grid_pattern_line_selection')

            elif event.key() == Qt.Key_Down:
                gp.select_next_line()
                self._updater.signal_update('signal_grid_pattern_line_selection')

            elif event.key() == Qt.Key_Insert:
                selected_line_ts = gp.get_selected_line()
                if selected_line_ts != None:
                    part_count = grid_mgr.get_grid_pattern_subdiv_part_count()
                    warp_value = grid_mgr.get_grid_pattern_subdiv_warp()
                    line_style = grid_mgr.get_grid_pattern_subdiv_line_style()

                    gp.subdivide_interval(
                            selected_line_ts, part_count, warp_value, line_style)
                    self._updater.signal_update('signal_grid_pattern_modified')

            elif event.key() == Qt.Key_Delete:
                selected_line_ts = gp.get_selected_line()
                lines = gp.get_lines()
                lines_dict = dict(lines)
                line_style = lines_dict.get(selected_line_ts, 0)
                if line_style != 0:
                    gp.select_next_line()
                    if gp.get_selected_line() == selected_line_ts:
                        gp.select_prev_line()

                    gp.remove_line(selected_line_ts)
                    self._updater.signal_update(
                        'signal_grid_pattern_line_selection',
                        'signal_grid_pattern_modified')

        elif event.modifiers() == Qt.ControlModifier:
            if event.key() == Qt.Key_Minus:
                grid_mgr.set_zoom(grid_mgr.get_zoom() - 1)
                self._updater.signal_update('signal_grid_zoom')
            elif event.key() == Qt.Key_Plus:
                grid_mgr.set_zoom(grid_mgr.get_zoom() + 1)
                self._updater.signal_update('signal_grid_zoom')
            elif event.key() == Qt.Key_0:
                grid_mgr.set_zoom(0)
                self._updater.signal_update('signal_grid_zoom')


class GeneralEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._name = QLineEdit()

        self._length = VarPrecSpinBox(step_decimals=0, max_decimals=5)
        self._length.setMinimum(1)
        self._length.setMaximum(32)

        self._spacing_style = LineStyle(is_major_enabled=True)
        self._spacing_value = VarPrecSpinBox(step_decimals=1, max_decimals=2)
        self._spacing_value.setMinimum(0.01)
        self._spacing_value.setMaximum(1.0)

        self._offset = VarPrecSpinBox(step_decimals=1, max_decimals=5)
        self._offset.setMinimum(0)
        self._offset.setMaximum(32)

        nl = QHBoxLayout()
        nl.setContentsMargins(0, 0, 0, 0)
        nl.setSpacing(2)
        nl.addWidget(QLabel('Name:'), 0)
        nl.addWidget(self._name, 1)

        ll = QHBoxLayout()
        ll.setContentsMargins(0, 0, 0, 0)
        ll.setSpacing(2)
        ll.addWidget(QLabel('Grid length:'), 0)
        ll.addWidget(self._length, 1)

        sl = QHBoxLayout()
        sl.setContentsMargins(0, 0, 0, 0)
        sl.setSpacing(2)
        sl.addWidget(QLabel('Show lines'), 0)
        sl.addWidget(self._spacing_style, 3)
        sl.addWidget(QLabel('when spacing is ≥:'), 0)
        sl.addWidget(self._spacing_value, 1)

        ol = QHBoxLayout()
        ol.setContentsMargins(0, 0, 0, 0)
        ol.setSpacing(2)
        ol.addWidget(QLabel('Grid offset:'), 0)
        ol.addWidget(self._offset, 1)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addLayout(nl)
        v.addLayout(ll)
        v.addLayout(sl)
        v.addLayout(ol)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_grid_pattern_selection', self._update_all)
        self.register_action('signal_grid_pattern_modified', self._update_all)
        self.register_action('signal_style_changed', self._update_style)

        self._update_config()

        self._name.textEdited.connect(self._change_name)
        self._length.valueChanged.connect(self._change_length)
        self._spacing_style.currentIndexChanged.connect(self._select_spacing_style)
        self._spacing_value.valueChanged.connect(self._change_spacing)

        self._offset.valueChanged.connect(self._change_offset)

        self._update_all()

    def _update_style(self):
        self._update_config()
        self.update()

    def _update_config(self):
        style_mgr = self._ui_model.get_style_manager()
        config = DEFAULT_CONFIG.copy()
        custom_config = get_config_with_custom_style(style_mgr)
        config.update(custom_config)

        layout = self.layout()
        spacing = style_mgr.get_scaled_size_param('small_padding')
        for i in range(layout.count()):
            sub_layout = layout.itemAt(i).layout()
            if sub_layout:
                sub_layout.setSpacing(spacing)

        self._spacing_style.set_config(config, style_mgr)

    def _get_selected_grid_pattern(self):
        grid_mgr = self._ui_model.get_grid_manager()
        gp_id = grid_mgr.get_selected_grid_pattern_id()
        if gp_id == None:
            return None

        gp = grid_mgr.get_grid_pattern(gp_id)
        return gp

    def _update_all(self):
        gp = self._get_selected_grid_pattern()
        self.setEnabled(gp != None)
        if gp == None:
            return

        gp_name = gp.get_name()
        if gp_name != str(self._name.text()):
            old_block = self._name.blockSignals(True)
            self._name.setText(gp_name)
            self._name.blockSignals(old_block)

        gp_length = gp.get_length()
        gp_length_f = float(gp_length)

        if gp_length_f != self._length.value():
            old_block = self._length.blockSignals(True)
            self._length.setValue(gp_length_f)
            self._length.blockSignals(old_block)

        spacing_value = gp.get_line_style_spacing(
                self._spacing_style.get_current_line_style())

        if spacing_value != self._spacing_value.value():
            old_block = self._spacing_value.blockSignals(True)
            self._spacing_value.setValue(spacing_value)
            self._spacing_value.blockSignals(old_block)

        gp_offset = gp.get_offset()
        gp_offset_f = float(gp_offset)

        if gp_offset_f != self._offset.value():
            old_block = self._offset.blockSignals(True)
            self._offset.setValue(gp_offset_f)
            self._offset.blockSignals(old_block)

    def _change_name(self, text):
        text = str(text)

        gp = self._get_selected_grid_pattern()
        if gp == None:
            return

        gp.set_name(text)
        self._updater.signal_update(
            'signal_grid_pattern_modified', 'signal_grid_pattern_list')

    def _change_length(self, new_length):
        new_length_ts = tstamp.Tstamp(new_length)

        gp = self._get_selected_grid_pattern()
        if gp == None:
            return

        gp.set_length(new_length_ts)
        self._updater.signal_update('signal_grid_pattern_modified')

    def _select_spacing_style(self, new_style):
        self._spacing_style.select_line_style(new_style)
        self._update_all()

    def _change_spacing(self, new_spacing):
        gp = self._get_selected_grid_pattern()
        if gp == None:
            return

        line_style = self._spacing_style.get_current_line_style()
        gp.set_line_style_spacing(line_style, new_spacing)
        self._updater.signal_update('signal_grid_pattern_modified')

    def _change_offset(self, new_offset):
        new_offset_ts = tstamp.Tstamp(new_offset)

        gp = self._get_selected_grid_pattern()
        if gp == None:
            return

        gp.set_offset(new_offset_ts)
        self._updater.signal_update('signal_grid_pattern_modified')


class SubdivEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._subdiv_count = QSpinBox()
        self._subdiv_count.setMinimum(2)
        self._subdiv_count.setMaximum(32)
        self._subdiv_line_style = LineStyle()
        self._subdiv_warp = NumberSlider(3, 0.001, 0.999)
        self._subdiv_apply = QPushButton('Subdivide (Insert)')

        self._subdiv_line_style.select_line_style(1)

        self._controls_layout = QGridLayout()
        self._controls_layout.setContentsMargins(0, 0, 0, 0)
        self._controls_layout.setSpacing(2)
        self._controls_layout.setColumnStretch(0, 0)
        self._controls_layout.setColumnStretch(1, 1)
        self._controls_layout.addWidget(QLabel('Parts:'), 0, 0)
        self._controls_layout.addWidget(self._subdiv_count, 0, 1)
        self._controls_layout.addWidget(QLabel('Style:'), 1, 0)
        self._controls_layout.addWidget(self._subdiv_line_style, 1, 1)
        self._controls_layout.addWidget(QLabel('Warp:'), 2, 0)
        self._controls_layout.addWidget(self._subdiv_warp, 2, 1)

        self._header = HeaderLine('Create subdivision')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header, 0, Qt.AlignTop)
        v.addLayout(self._controls_layout, 0)
        v.addWidget(self._subdiv_apply, 0)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_grid_pattern_selection', self._update_all)
        self.register_action('signal_grid_pattern_line_selection', self._update_all)
        self.register_action('signal_grid_pattern_modified', self._update_all)
        self.register_action('signal_grid_pattern_subdiv', self._update_all)
        self.register_action('signal_style_changed', self._update_style)

        self._update_config()

        self._subdiv_count.valueChanged.connect(self._change_count)
        self._subdiv_line_style.currentIndexChanged.connect(self._change_line_style)
        self._subdiv_warp.numberChanged.connect(self._change_warp)
        self._subdiv_apply.clicked.connect(self._apply_subdivision)

        self._update_all()

    def _update_style(self):
        self._header.update_style(self._ui_model.get_style_manager())
        self._update_config()
        self.update()

    def _update_config(self):
        style_mgr = self._ui_model.get_style_manager()
        config = DEFAULT_CONFIG.copy()
        custom_config = get_config_with_custom_style(style_mgr)
        config.update(custom_config)

        spacing = style_mgr.get_scaled_size_param('small_padding')
        self._controls_layout.setSpacing(spacing)
        self.layout().setSpacing(spacing)

        self._subdiv_line_style.set_config(config, style_mgr)

    def _get_selected_line_ts(self):
        grid_mgr = self._ui_model.get_grid_manager()
        gp_id = grid_mgr.get_selected_grid_pattern_id()
        if gp_id == None:
            return None

        # Get line selection info
        gp = grid_mgr.get_grid_pattern(gp_id)
        gp_lines = gp.get_lines()
        lines_dict = dict(gp_lines)
        selected_line_ts = gp.get_selected_line()

        return selected_line_ts if selected_line_ts in lines_dict else None

    def _update_all(self):
        selected_line_ts = self._get_selected_line_ts()
        self.setEnabled(selected_line_ts != None)

        grid_mgr = self._ui_model.get_grid_manager()

        old_block = self._subdiv_count.blockSignals(True)
        self._subdiv_count.setValue(grid_mgr.get_grid_pattern_subdiv_part_count())
        self._subdiv_count.blockSignals(old_block)

        self._subdiv_line_style.select_line_style(
                grid_mgr.get_grid_pattern_subdiv_line_style())
        self._subdiv_warp.set_number(grid_mgr.get_grid_pattern_subdiv_warp())

    def _change_count(self, new_count):
        grid_mgr = self._ui_model.get_grid_manager()
        grid_mgr.set_grid_pattern_subdiv_part_count(new_count)
        self._updater.signal_update('signal_grid_pattern_subdiv')

    def _change_line_style(self, dummy):
        new_style = self._subdiv_line_style.get_current_line_style()
        grid_mgr = self._ui_model.get_grid_manager()
        grid_mgr.set_grid_pattern_subdiv_line_style(new_style)
        self._updater.signal_update('signal_grid_pattern_subdiv')

    def _change_warp(self, new_warp):
        grid_mgr = self._ui_model.get_grid_manager()
        grid_mgr.set_grid_pattern_subdiv_warp(new_warp)
        self._updater.signal_update('signal_grid_pattern_subdiv')

    def _apply_subdivision(self):
        selected_line_ts = self._get_selected_line_ts()
        assert selected_line_ts != None

        grid_mgr = self._ui_model.get_grid_manager()
        gp_id = grid_mgr.get_selected_grid_pattern_id()
        gp = grid_mgr.get_grid_pattern(gp_id)

        part_count = grid_mgr.get_grid_pattern_subdiv_part_count()
        warp_value = grid_mgr.get_grid_pattern_subdiv_warp()
        line_style = grid_mgr.get_grid_pattern_subdiv_line_style()

        gp.subdivide_interval(selected_line_ts, part_count, warp_value, line_style)
        self._updater.signal_update('signal_grid_pattern_modified')


class LineEditor(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._line_style = LineStyle()
        self._remove_button = QPushButton('Remove line (Delete)')

        self._controls_layout = QHBoxLayout()
        self._controls_layout.setContentsMargins(0, 0, 0, 0)
        self._controls_layout.setSpacing(2)
        self._controls_layout.addWidget(QLabel('Style:'), 0)
        self._controls_layout.addWidget(self._line_style, 1)

        self._header = HeaderLine('Current line')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header, 0, Qt.AlignTop)
        v.addLayout(self._controls_layout, 0)
        v.addWidget(self._remove_button, 0, Qt.AlignTop)
        v.addWidget(QWidget(), 1)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_grid_pattern_selection', self._update_all)
        self.register_action('signal_grid_pattern_line_selection', self._update_all)
        self.register_action('signal_grid_pattern_modified', self._update_all)
        self.register_action('signal_style_changed', self._update_config)

        self._update_config()

        self._line_style.currentIndexChanged.connect(self._change_line_style)
        self._remove_button.clicked.connect(self._remove_selected_line)

        self._update_all()

    def _update_config(self):
        style_mgr = self._ui_model.get_style_manager()
        self._header.update_style(style_mgr)
        config = DEFAULT_CONFIG.copy()
        custom_config = get_config_with_custom_style(style_mgr)
        config.update(custom_config)

        spacing = style_mgr.get_scaled_size_param('small_padding')
        self._controls_layout.setSpacing(spacing)
        self.layout().setSpacing(spacing)

        self._line_style.set_config(config, style_mgr)

    def _get_grid_pattern(self):
        grid_mgr = self._ui_model.get_grid_manager()
        gp_id = grid_mgr.get_selected_grid_pattern_id()
        if gp_id == None:
            return None

        return grid_mgr.get_grid_pattern(gp_id)

    def _update_all(self):
        gp = self._get_grid_pattern()
        if gp == None:
            self.setEnabled(False)
            return

        selected_line_ts = gp.get_selected_line()

        # Get line selection info
        gp_lines = gp.get_lines()
        lines_dict = dict(gp_lines)
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

        gp = self._get_grid_pattern()
        assert gp != None
        selected_line_ts = gp.get_selected_line()

        gp.change_line_style(selected_line_ts, line_style)
        self._updater.signal_update('signal_grid_pattern_modified')

    def _remove_selected_line(self):
        gp = self._get_grid_pattern()
        assert gp != None
        selected_line_ts = gp.get_selected_line()

        gp.remove_line(selected_line_ts)
        self._updater.signal_update('signal_grid_pattern_modified')


class LineStyleDelegate(QItemDelegate):

    def __init__(self, is_major_enabled, style_mgr):
        super().__init__()
        self._config = None

        self._first_style = 0 if is_major_enabled else 1

        self._pixmaps = {}
        self._list_pixmap = None

        self._pixmap_width = 0
        self._pixmap_height = 0
        self._pixmap_horiz_margin = 0

        self._style_mgr = style_mgr

    def set_config(self, config):
        self._config = config

        fm = QFontMetrics(self._config['font'], QWidget())
        em_px = int(math.ceil(fm.tightBoundingRect('m').width()))
        self._line_length = self._config['col_width'] * em_px

        # Pixmap style
        h_margin = self._style_mgr.get_scaled_size_param('large_padding')
        v_margin = self._style_mgr.get_scaled_size_param('large_padding')
        p_width = self._line_length + 2 * h_margin + 1
        p_height = 1 + 2 * v_margin

        self._pixmap_horiz_margin = h_margin
        self._pixmap_width = p_width
        self._pixmap_height = p_height

        # Create line pixmaps
        for i in range(STYLE_COUNT):
            pixmap = self._create_line_pixmap(i, p_width, p_height, h_margin, v_margin)
            self._pixmaps[i] = pixmap

        # Create list pixmap (work-around for incorrect drawing of selected entries)
        list_width = self._pixmap_width
        list_height = (STYLE_COUNT - self._first_style) * self._pixmap_height
        self._list_pixmap = QPixmap(list_width, list_height)

        lpainter = QPainter(self._list_pixmap)
        for i in range(self._first_style, STYLE_COUNT):
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
        pixmap_index = index.data(Qt.UserRole)
        assert pixmap_index in self._pixmaps

        # Background
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(option.rect)

        # Line pixmap
        cursor_width = self._config['grid']['edit_cursor']['width']
        pixmap = self._pixmaps[pixmap_index]
        pixmap_pos = option.rect.translated(cursor_width, 0).topLeft()
        painter.drawPixmap(QRect(pixmap_pos, pixmap.size()), pixmap)
        super().paint(painter, option, index)

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

            painter.drawPolygon(QPolygon([
                    QPoint(0, cursor_max_y),
                    QPoint(cursor_config['width'], 0),
                    QPoint(0, -cursor_max_y)]))

            painter.restore()

    def sizeHint(self, option, index):
        pixmap_index = index.data(Qt.UserRole)
        assert pixmap_index in self._pixmaps

        pixmap = self._pixmaps[pixmap_index]
        return pixmap.size()


class LineStyle(KqtComboBox):

    def __init__(self, is_major_enabled=False):
        super().__init__()

        self._first_style = 0 if is_major_enabled else 1
        self._is_major_displayed = True
        self._ls_delegate = None

        #self.set_config(DEFAULT_CONFIG)

        for i in range(self._first_style, STYLE_COUNT):
            self.addItem(str(i), i)

    def set_config(self, config, style_mgr):
        self._config = config

        is_major_enabled = (self._first_style == 0)
        self._ls_delegate = LineStyleDelegate(is_major_enabled, style_mgr)
        self._ls_delegate.set_config(self._config)

        self.setItemDelegate(self._ls_delegate)

    def get_current_line_style(self):
        return self.itemData(self.currentIndex())

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


